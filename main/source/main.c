#if 0
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#endif

#include <tonc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utf8.h"

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
int menu_scroll = 0;
int menu_numents = 0;
u8 fs_initted = 0;
u8 menu_boot_sel_is_dir = 0;

#define MENU_NUM_LINES (12)

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

    if (!menu_scroll)
        video_printf("\n#{ci:1}%s\n", menu_curdir);
    else
        video_printf("\n#{ci:2;X:12}^^^\n");

    res = f_opendir(&dir, ".");
    int iter = 0;
    int pos_y = 20;
    int scroll_amt = menu_scroll;
    int num_drawn = 0;
    int stuff_below = 0;
    menu_numents = 0;
    if(res == FR_OK)
    {
        while (1)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fname[0] == '.') continue;
            u8 is_dir = fno.fattrib & AM_DIR;

            char* ext = utf8rchr(fno.fname, '.');
            if (ext && ext != fno.fname && !is_dir)
            {
                if (strcmp(ext, ".gba"))
                {
                    //continue;
                }
            }

            //mgba_printf("%s", fno.fname);

            menu_numents++;

            if (scroll_amt) {
                scroll_amt--;
                iter++;
                continue;
            }

            if (num_drawn >= MENU_NUM_LINES)
            {
                iter++;
                stuff_below++;
                continue;
            }

            num_drawn++;

            video_printf("#{ci:2;Y:%u;X:2}%c#{ci:1;X:12}%s%c", pos_y, menu_cur_sel == iter ? '>' : ' ', fno.fname, is_dir ? '/' : ' ');

            pos_y += 10;

            if (menu_cur_sel == iter)
            {
                //strcpy(loading_rominfo.fullpath, menu_curdir);
                strcpy(loading_rominfo.fullpath, fno.fname);
                menu_boot_sel_is_dir = is_dir;
            }

            iter++;
        }
    }
    if (stuff_below)
    {
        video_printf("#{ci:2;Y:%u;X:12}vvv",pos_y);

    }
    //video_printf("\nend %x\n", res);
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

void save_backup_to_sd()
{
    FIL file = {0};
    
    UINT btx = 0;
    int res;

    char fname[64];
    char tmp[0x80];

    // Read registery
    res = f_open(&file, "/GBASYS/sys/cfw_registery.dat", FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        res = f_read(&file, &loading_rominfo, sizeof(loading_rominfo), &btx);
        if(res != FR_OK) {
            video_printf("Error while reading, %x\n", res);
        }

        f_close(&file);

        if (btx != sizeof(loading_rominfo))
        {
            return;
        }
    }
    else
    {
        return;
    }

    // Check if the CRC is valid I guess
    u32 compare_crc = 0;
    crc32(&loading_rominfo, sizeof(loading_rominfo) - sizeof(u32), &compare_crc);

    if (loading_rominfo.crc != compare_crc)
        return;

    if (loading_rominfo.titleinfo.rtc_enabled)
        bi_rtc_on();

    bi_set_save_type(BI_SAV_SRM);

    // Get just the filename
    char* loading_fname = utf8rchr(loading_rominfo.fullpath, '/');
    if (!loading_fname) {
        loading_fname = loading_rominfo.fullpath;
    }
    else
    {
        loading_fname++;
    }

    // Strip the extension
    utf8ncpy(fname, loading_fname, 64);
    char* ext = utf8rchr(fname, '.');
    if (ext && ext != fname)
    {
        *ext = 0;
    }

    // Load the save file
    u32 offs = 0;
    u32 to_write = a_bram_saveLengths[loading_rominfo.titleinfo.save_type]; // TODO check oob
    snprintf(tmp, sizeof(tmp), "/GBASYS/save/%s%s", fname, bram_save_ext(loading_rominfo.titleinfo.save_type));

    video_printf("Write %s...\n", tmp);

    res = f_open(&file, tmp, FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        while (offs < to_write)
        {
            bi_sram_read(tmp, offs, sizeof(tmp));

            res = f_write(&file, tmp, sizeof(tmp), &btx);
            if(res != FR_OK) {
                video_printf("Error while writing save, %x\n", res);
                break;
            }

            offs += btx;
        }
        //video_printf("%x %x %x\n", offs, to_write, a_bram_saveLengths[loading_rominfo.titleinfo.save_type]);

        f_close(&file);
    }
    else
    {
        video_printf("Error while writing save, %x\n", res);
    }

    // ruin the CRC and write back
    loading_rominfo.titleinfo.gamecode = 0;

    res = f_open(&file, "/GBASYS/sys/cfw_registery.dat", FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        res = f_write(&file, &loading_rominfo, sizeof(loading_rominfo), &btx);
        if(res != FR_OK) {
            video_printf("Error while writing, %x\n", res);
        }

        f_close(&file);
    }
    else
    {
        video_printf("Error while writing reg, %x\n", res);
    }
}

void menu_launch_selected()
{
    FIL file = {0};
    
    FILINFO fno = {0};
    UINT btx = 0;
    int res;

    char fname[64];

    // Get just the filename
    char* loading_fname = utf8rchr(loading_rominfo.fullpath, '/');
    if (!loading_fname) {
        loading_fname = loading_rominfo.fullpath;
    }
    else
    {
        loading_fname++;
    }

    // Strip the extension
    utf8ncpy(fname, loading_fname, 64);
    char* ext = utf8rchr(fname, '.');
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
                mgba_printf("%x %x\n", entry[0], entry[1]);

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
    char tmp[0x80]; // TODO sizes
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

    res = f_open(&file, "/GBASYS/sys/cfw_registery.dat", FA_CREATE_ALWAYS | FA_WRITE);
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
                video_printf("Error while reading save, %x\n", res);
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
    else
    {
        memset(tmp, 0xFF, sizeof(tmp));

        while (offs < to_read)
        {
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
    }
}

int main(void) 
{
    irq_init(NULL);
    irq_enable(II_VBLANK);

    mgba_open();

    memset(&loading_rominfo, 0, sizeof(loading_rominfo));

    //GBA_WAITCNT = 0;//0x4317;

    bi_init();
    bi_set_rom_bank(0);

    //consoleDemoInit();

    mgba_printf("Hello mGBA!\n");

    fs_init();
    video_dirty = 1;

    video_init();
    video_printf("Hello world!\n");

    //video_mode_singlebuffer();
    save_backup_to_sd();

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
        if (video_dirty)
            menu_draw();
        else
            VBlankIntrWait();

        key_poll();
        u32 kd = key_hit(KEY_FULL);

        if (kd & KEY_DOWN)
        {
            if (menu_cur_sel < menu_numents-1)
                menu_cur_sel++;
            video_dirty = 1;

            if ((menu_cur_sel - menu_scroll) >= MENU_NUM_LINES)
            {
                menu_scroll++;
            }
        }
        if (kd & KEY_UP)
        {
            if (menu_cur_sel)
                menu_cur_sel--;
            video_dirty = 1;

            if ((menu_cur_sel - menu_scroll) < 0)
            {
                menu_scroll--;
            }
        }
        if (kd & KEY_B)
        {
            //f_getcwd(menu_curdir, sizeof(menu_curdir));
            char* last_dir = utf8rchr(menu_curdir, '/');
            if (last_dir)
            {
                memset(last_dir, 0, &menu_curdir[sizeof(menu_curdir)] - last_dir);
            }

            if (last_dir == menu_curdir)
            {
                *(menu_curdir) = '/';
            }

            menu_cur_sel = 0;
            menu_numents = 0;
            menu_scroll = 0;
            
            f_chdir(menu_curdir);

            if (last_dir && last_dir != menu_curdir) {
                utf8cat(menu_curdir, "/");
            }
            //mgba_printf("%s\n", menu_curdir);

            video_dirty = 1;
        }
        if (kd & KEY_A && menu_boot_sel_is_dir) {
            
            menu_cur_sel = 0;
            menu_numents = 0;
            menu_scroll = 0;
            //f_getcwd(menu_curdir, sizeof(menu_curdir));
            if (strcmp(menu_curdir, "/"))
                utf8cat(menu_curdir, "/");

            utf8cat(menu_curdir, loading_rominfo.fullpath);
            //mgba_printf("%s\n", menu_curdir);
            f_chdir(menu_curdir);

            video_dirty = 1;
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


