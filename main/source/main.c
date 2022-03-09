#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bios.h"
#include "disk.h"
#include "fatfs/ff.h"
#include "video.h"
#include "mgba.h"

// [char * 0x170 fullpath] [u32 code] [u32 unk] [char * 2 unk] [char * 0xC title name] [u16 unk] [u32 unk] [u32 unk] [u16 unk] [u16 unk] [u32 unk] [u32 crc32]

typedef struct gba_cartheader_t
{
    u32 entry;
    char nintendo_logo[156];
    char title_name[12];
    u32 gamecode;
    u16 maker_code;
    u8 fixed;
    u8 unit_code;
    u8 dacs;
    u8 reserved[7];
    u8 sw_version;
    u8 checksum;
} gba_cartheader_t;

typedef struct gba_titleinfo_t
{
    u32 gamecode; // 0xAC
    u32 unk1; // pkmn ruby is 0x8000?
    u16 maker_code; // 0xB0
    char title_name[0xC]; // 0xA0
    u16 unk4; // padding?
    u8 cartheader_crc; // 0xBD
    u8 save_type; // bram type, not ED
    u8 rtc_enabled;
    u8 is_emulated;
} gba_titleinfo_t;

typedef struct gba_registry_t
{
    char fullpath[0x144];
    gba_titleinfo_t titleinfo;
    u32 unk6;
    u16 unk7;
    u16 unk8;
    u32 unk9;
    u32 crc;
} gba_registry_t;

// bram-db.dat to bi_set_save_type types
const u8 bram_to_ed[8] = {0,BI_SAV_EEP,BI_SAV_SRM,BI_SAV_FLA64,BI_SAV_FLA128,0,0,0,0};

const u32 a_bram_saveLengths[5] = {0, 0x2000, 0x8000, 0x10000, 0x20000};

// bram-db.dat doesn't include RTC stuff :/
const char* bram_rtc_list = "U3IPU3IEU32PU32EU3IJBLVJBLJJBPEJAXVJAXPJBPEEBPEPBPESAXVPAXVEAXVSAXVIAXPPAXPEAXPSAXPIBR4JBKAJU33JU3IP\x00\x00\x00\x00";

int sio_waiting = 0;

void serial_init() {
    REG_RCNT = 0;
    REG_SIOCNT = SIO_32BIT | SIO_SO_HIGH | SIO_IRQ;
}

u32 xfer32(u32 tosend) 
{
    //Wait for exchange to start
    REG_SIODATA32 = tosend;
    REG_SIOCNT &= ~(SIO_START | SIO_32BIT);
    REG_SIOCNT |= SIO_START | SIO_32BIT;
    while(REG_SIOCNT & SIO_START){} // wait for send

    //Master sends data, we read
    //while((REG_RCNT & 0x1) == 0){}
    u32 ret = REG_SIODATA32; //Get data

    u32 timeout = 1000;
    while (!(REG_RCNT & 1))
    {
        if (!--timeout) break;
    }
    
    return ret;
}

void irq_sio()
{
    sio_waiting = 1;
}

void sio_chainload()
{
    while (xfer32(0xF00F1234) != 0xF00F5678)
    {
        VBlankIntrWait();
    }

    u32 len = xfer32(0xF00F0000) & 0xFFFFFFF0;
    u32* out = (u32*)0x08000000;

    while (len)
    {
        *out = xfer32((u32)out);
        out++;
        len -= 4;
    }

    void (*entry)(void) __attribute__((noreturn)) = (void*)0x08000000;
    entry();
}

void sio_handle()
{
    u32 cmd = xfer32(0);
    if (cmd == 0xF00F8765) {
        sio_chainload();
        return;
    }

    sio_waiting = 0;
    char v = xfer32(0);
    if (v && v != 0xFF)
        iprintf("%c", v);
}

uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
  static uint32_t table[0x100];
  if(!*table)
    for(size_t i = 0; i < 0x100; ++i)
      table[i] = crc32_for_byte(i);
  for(size_t i = 0; i < n_bytes; ++i)
    *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}

gba_registry_t loading_rominfo __attribute((aligned(16)));

FATFS fatfs __attribute((aligned(16))) = {0};
char menu_curdir[64];
int menu_cur_sel = 0;
u8 fs_initted = 0;
u8 menu_boot_sel_is_dir = 0;

const char* bram_save_ext(u8 type)
{
    switch (type)
    {
        case DB_SAV_NONE:
            return "";
        case DB_SAV_EEP:
            return ".eep";
        case DB_SAV_SRM:
            return ".srm";
        case DB_SAV_FLA64:
        case DB_SAV_FLA128:
            return ".fla";
        default:
            return ".sav";
    }
}

void menu_draw()
{
    int res;
    FILINFO fno = {0};
    DIR dir = {};

    video_clear();

    video_printf("\n%s\n", menu_curdir);

    res = f_opendir(&dir, ".");
    int iter = 0;
    if(res == FR_OK)
    {
        while (1)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fname[0] == '.') continue;
            u8 is_dir = fno.fattrib & AM_DIR;

            video_color = 4;
            video_printf(" %c ", menu_cur_sel == iter ? '>' : ' ');
            video_color = 2;
            video_printf("%s%c\n", fno.fname, is_dir ? '/' : ' ');

            if (menu_cur_sel == iter)
            {
                strcpy(loading_rominfo.fullpath, menu_curdir);
                strcat(loading_rominfo.fullpath, fno.fname);
                menu_boot_sel_is_dir = is_dir;
            }

            iter++;
        }
    }
    f_closedir(&dir);

    video_swap();
    video_dirty = 0;
}

void fs_init()
{
    int res = f_mount(&fatfs, "sdmc:", 1);
    fs_initted = (res == FR_OK);

    strcpy(menu_curdir, "/");
}

void menu_launch_selected()
{
    FIL file = {0};
    
    FILINFO fno = {0};
    UINT btx = 0;
    int res;

    char fname[64];

    // Get just the filename
    char* loading_fname = strrchr(loading_rominfo.fullpath, '/');
    if (!loading_fname) {
        loading_fname = loading_rominfo.fullpath;
    }
    else
    {
        loading_fname++;
    }

    // Strip the extension
    strncpy(fname, loading_fname, 64);
    char* ext = strrchr(fname, '.');
    if (ext && ext != fname)
    {
        *ext = 0;
    }

    // Pre-set these
    loading_rominfo.titleinfo.rtc_enabled = 0;
    loading_rominfo.titleinfo.save_type = 0;

    //const char* to_load = "/GBASYS/GBAOS.gba";
    const char* to_load = loading_rominfo.fullpath;//"/Ruby.gba";
    video_printf("%s %s\n", to_load, fname);
    //const char* to_load = "/gba-switch-bios_mb.gba";

    res = f_open(&file, to_load, FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        size_t total = f_size(&file);
        size_t read = 0;
        size_t chunk_size = f_size(&file);
        if (chunk_size > 0x800000)
            chunk_size = 0x800000;

        video_printf("Reading %s...\nsz 0x%08zx\n", to_load, total);

        while (total)
        {
            res = f_read(&file, (void*)(0x08000000 + read), chunk_size, &btx);
            if(res != FR_OK) {
                video_printf("Error while reading ROM, %x\n", res);
                while (1);
                break;
            }
            //video_printf("%08zx\n", total);
            //(void*)(0x0A000000 + read)
            //*(u32*)(0x08000000 + read) = test;
            if (btx > total)
                break;

            total -= btx;
            read += btx;
            //iprintf("%x\n", read);

            if (read >= 0x01000000) {
                bi_set_rom_bank(1);
                read -= 0x01000000;
            }
        }

        bi_set_rom_bank(0);

        f_close(&file);
    }

    // TODO: Binary search for speed
    res = f_open(&file, "/GBASYS/sys/bram-db.dat", FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        uint32_t entry[2];
        entry[1] = 0;

        uint32_t num_entries = f_size(&file)/5;

        for (int i = 0; i < num_entries; i++)
        {
            res = f_read(&file, &entry, 5, &btx);
            if(res != FR_OK) {
                video_printf("Error while reading bram-db, %x\n", res);
                break;
            }
            if (*(u32*)0x080000AC == entry[0]) {
                mgba_printf(MGBA_LOG_ERROR, "%x %x\n", entry[0], entry[1]);

                u8 sav_type = entry[1] & DB_SAV_MASK;

                loading_rominfo.titleinfo.save_type = sav_type;

                if (entry[1] & DB_IS_32MIB) {
                    //TODO
                    bi_set_rom_mask(0x02000000);
                }
            }
        }
        f_close(&file);
    }

    gba_cartheader_t* pCartHeader = (gba_cartheader_t*)0x08000000;

    // Check if the RTC needs enabling
    u32* rtc_list = (u32*)bram_rtc_list;
    while (1)
    {
        if (!*rtc_list) break;
        if (*rtc_list == pCartHeader->gamecode) {
            loading_rominfo.titleinfo.rtc_enabled = 1;
            break;
        }
        rtc_list++;
    }

    // Reuse memory here
    char tmp[0x100]; // TODO sizes
    snprintf(tmp, sizeof(tmp), "/GBASYS/sys/romcfg/%s.dat", fname);

    // GBASYS/sys/romcfg/*.dat, [u16 RTC en override] [u16 save override]
    res = f_open(&file, tmp, FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        u16 entry[2];
        entry[0] = 0;
        entry[1] = 0;

        res = f_read(&file, &entry, sizeof(u16)*2, &btx);
        if(res != FR_OK) {
            video_printf("Error while reading romcfg, %x\n", res);
        }

        // RTC override
        if (entry[0])
        {
            loading_rominfo.titleinfo.rtc_enabled = 1;
        }

        // Save override
        if (entry[1])
        {
            u8 sav_type = entry[1] & DB_SAV_MASK;

            loading_rominfo.titleinfo.save_type = sav_type;
        }

        f_close(&file);
    }

    

    loading_rominfo.titleinfo.gamecode = pCartHeader->gamecode;
    loading_rominfo.titleinfo.maker_code = pCartHeader->maker_code;
    loading_rominfo.titleinfo.unk1 = 0x8000;
    strncpy(loading_rominfo.titleinfo.title_name, pCartHeader->title_name, 0xC);
    loading_rominfo.titleinfo.unk4 = 0;
    loading_rominfo.titleinfo.cartheader_crc = pCartHeader->checksum;
    loading_rominfo.titleinfo.is_emulated = 0;

    loading_rominfo.unk6 = 0x10000;
    loading_rominfo.unk7 = 0x1;
    loading_rominfo.unk8 = 0x1;
    loading_rominfo.unk9 = 0;
    loading_rominfo.crc = 0;

    crc32(&loading_rominfo, sizeof(loading_rominfo) - sizeof(u32), &loading_rominfo.crc);

    if (loading_rominfo.titleinfo.rtc_enabled)
        bi_rtc_on();

    bi_set_save_type(bram_to_ed[loading_rominfo.titleinfo.save_type]);

    // TODO: GBASYS/sys/registery.dat [char * 0x170 fullpath] [u32 code] [u32 unk] [char * 2 unk] [char * 0xC title name] [u16 unk] [u32 unk] [u32 unk] [u16 unk] [u16 unk] [u32 unk] [u32 crc32]

    res = f_open(&file, "/GBASYS/sys/test_registery.dat", FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        video_printf("Write...\n");

        res = f_write(&file, &loading_rominfo, sizeof(loading_rominfo), &btx);
        if(res != FR_OK) {
            video_printf("Error while writing, %x\n", res);
        }

        f_close(&file);
    }

    // Load the save file
    u32 offs = 0;
    u32 to_read = a_bram_saveLengths[loading_rominfo.titleinfo.save_type]; // TODO check oob
    snprintf(tmp, sizeof(tmp), "/GBASYS/save/%s%s", fname, bram_save_ext(loading_rominfo.titleinfo.save_type));

    video_printf("Load %s...\n", tmp);
    // GBASYS/sys/romcfg/*.dat, [u16 RTC en override] [u16 save override]
    res = f_open(&file, tmp, FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        to_read = min(to_read, f_size(&file));

        while (offs < to_read)
        {
            res = f_read(&file, tmp, sizeof(tmp), &btx);
            if(res != FR_OK) {
                video_printf("Error while reading romcfg, %x\n", res);
                break;
            }

            if (loading_rominfo.titleinfo.save_type == DB_SAV_EEP)
            {
                bi_eep_write(tmp, offs, sizeof(tmp));
            }
            else if (loading_rominfo.titleinfo.save_type == DB_SAV_SRM)
            {
                bi_sram_write(tmp, offs, sizeof(tmp));
            }
            else if (loading_rominfo.titleinfo.save_type == DB_SAV_FLA64 || loading_rominfo.titleinfo.save_type == DB_SAV_FLA128)
            {
                if (offs >= 0x10000)
                {
                    bi_flash_set_bank(1);
                    bi_flash_write(tmp, offs - 0x10000, sizeof(tmp));
                }
                else
                {
                    bi_flash_set_bank(0);
                    bi_flash_write(tmp, offs, sizeof(tmp));
                }
                
            }

            offs += btx;
        }
        //video_printf("%x %x %x\n", offs, to_read, a_bram_saveLengths[loading_rominfo.titleinfo.save_type]);

        f_close(&file);
    }
}

int main(void) 
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    mgba_open();

    memset(&loading_rominfo, 0, sizeof(loading_rominfo));

    //GBA_WAITCNT = 0;//0x4317;

    bi_init();
    bi_set_rom_bank(0);

    video_init();

    //consoleDemoInit();

    video_printf("Hello world!\n");
    mgba_printf(MGBA_LOG_ERROR, "Hello mGBA!\n");

    fs_init();
    video_dirty = 1;

#if 0
    u8 __attribute((aligned(16))) tmp[16];
    u16 addr = 0x8000;
    
    UINT btx = 0;
    FIL file = {0};
    int res = f_open(&file, "sdmc:/read_eeprom.bin", FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        video_printf("Write...\n");
        for (int i = 0; i < 0x200/16; i++)
        {
            bi_persist_read(addr, tmp, 16);

            res = f_write(&file, tmp, 16, &btx);
            if(res != FR_OK) {
                video_printf("Error while reading, %x\n", res);
                break;
            }
            
            addr += 16;
        }
        f_close(&file);
    }

    res = f_open(&file, "sdmc:/read_eeprom.bin", FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        f_close(&file);
    }

    f_unmount(&fatfs);

    while (1);
#endif

    while (1)
    {
        //if (video_dirty)
            menu_draw();
        scanKeys();
        u16 kd = keysDown();

        if (kd & KEY_DOWN)
        {
            menu_cur_sel++;
            video_dirty = 1;
        }
        if (kd & KEY_UP)
        {
            menu_cur_sel--;
            video_dirty = 1;
        }
        if (kd & KEY_B)
        {
            f_chdir("..");
            menu_cur_sel = 0;
            f_getcwd(menu_curdir+5, sizeof(menu_curdir)-5);
        }
        if (kd & KEY_A && menu_boot_sel_is_dir) {
            f_chdir(loading_rominfo.fullpath);
            menu_cur_sel = 0;
            f_getcwd(menu_curdir+5, sizeof(menu_curdir)-5);
        }
        else if (kd & KEY_A && !menu_boot_sel_is_dir)
        {
            video_mode_singlebuffer();

            menu_launch_selected();
            //video_swap();
            //
            video_printf("\nTest SD Read:\n%08x %08x\n", *(u32*)0x08000000, *(u32*)0x08000004, (void*)0x080000AC);
            //while (1);
            //if (fs_initted)
            bi_reboot(1);
            while (1);
        }
    }

#if 0
    test_read();
    
    bi_lock_regs();

    video_printf("\nTest SD Read:\n%08x %08x\n", *(u32*)0x08000000, *(u32*)0x08000004, (void*)0x080000AC);

    //bi_reboot(1);
#endif
    while (1);

    
    //*(u32*)0x09000000 = 0x1234ABCD;

#if 0
    iprintf("\nWaiting for next chainload...\n");
    serial_init();
    irqEnable(IRQ_SERIAL);
    irqSet(IRQ_SERIAL, irq_sio);
#endif

    while (1) 
    {
        //if (sio_waiting) {
            //sio_handle();
        //}
    }
}


