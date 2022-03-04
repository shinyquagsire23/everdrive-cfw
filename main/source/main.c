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

// bram-db.dat to bi_set_save_type types
const u8 bram_to_ed[8] = {0,BI_SAV_EEP,BI_SAV_SRM,BI_SAV_FLA64,BI_SAV_FLA128,0,0,0,0};

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

FATFS fatfs __attribute((aligned(16))) = {0};
char menu_curdir[64];
char menu_boot_sel[64];
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
                strcpy(menu_boot_sel, menu_curdir);
                strcat(menu_boot_sel, fno.fname);
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

    strcpy(menu_curdir, "sdmc:/");
}

void menu_launch_selected()
{
    FIL file = {0};
    
    FILINFO fno = {0};
    UINT btx = 0;
    int res;

    //const char* to_load = "sdmc:/GBASYS/GBAOS.gba";
    const char* to_load = menu_boot_sel;//"sdmc:/Ruby.gba";
    video_printf("%s\n", to_load);
    //const char* to_load = "sdmc:/gba-switch-bios_mb.gba";

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
                video_printf("Error while reading, %x\n", res);
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
    res = f_open(&file, "sdmc:/GBASYS/sys/bram-db.dat", FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        uint32_t entry[2];
        entry[1] = 0;

        uint32_t num_entries = f_size(&file)/5;

        for (int i = 0; i < num_entries; i++)
        {
            res = f_read(&file, &entry, 5, &btx);
            if(res != FR_OK) {
                video_printf("Error while reading, %x\n", res);
                break;
            }
            if (*(u32*)0x080000AC == entry[0]) {
                mgba_printf(MGBA_LOG_ERROR, "%x %x\n", entry[0], entry[1]);

                u8 sav_type = entry[1] & DB_SAV_MASK;
                u8 ed_sav_type = bram_to_ed[sav_type];
                bi_set_save_type(ed_sav_type);

                if (entry[1] & DB_IS_32MIB) {
                    //TODO
                    bi_set_rom_mask(0x02000000);
                }
            }
        }
        f_close(&file);
    }

    // Check if the RTC needs enabling
    u32* rtc_list = (u32*)bram_rtc_list;
    while (1)
    {
        if (!*rtc_list) break;
        if (*rtc_list == *(u32*)0x080000AC) {
            bi_rtc_on();
        }
        rtc_list++;
    }
}

int main(void) 
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    mgba_open();

    GBA_WAITCNT = 0;//0x4317;

    bi_init();
    bi_set_rom_bank(0);


    u8 idk = 0;//EDIO_startup();

    video_init();

    //consoleDemoInit();

    video_printf("Hello world!\n");
    mgba_printf(MGBA_LOG_ERROR, "Hello mGBA!\n");

    fs_init();
    video_dirty = 1;

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
            f_chdir(menu_boot_sel);
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


