#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "bios.h"
#include "disk.h"
#include "fatfs/ff.h"

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

#define BG0_CHARBASE (0)
#define BG1_CHARBASE (1)
#define BG0_MAPBASE (20)

#define FONT_SCALE (1)

#define FB_OFFSET(x, y)     ( ((((x%8)/4) ) * 2) + ((y % 8)*4) + ((y/8) * 0x400) + ((x/8) * 0x20) + 0x20 )
#define FB_SHIFT(c, x, y)     (c << (x % 4)*4)

extern const u8 msx_font[];
int video_line_y = 0;
int video_line_x = 0;
FATFS fatfs __attribute((aligned(16))) = {0};
char menu_curdir[64];
char menu_boot_sel[64];
int menu_cur_sel = 0;
u8 video_color = 2;
u8 video_whichbuf = 0;
u8 video_dirty = 0;

static inline void draw_plot_raw(int x, int y, u8 c)
{
    void* addr = CHAR_BASE_ADR(video_whichbuf ? BG1_CHARBASE : BG0_CHARBASE) + FB_OFFSET(x, y);
    u16 read = *((u16*)addr);
    read &= ~FB_SHIFT(0xF, x, y);
    *((u16*)addr) = read | (FB_SHIFT(c, x, y));
}

int draw_char(int x, int y, u8 color, char c)
{
    u8 *font = (u8*)(msx_font + c * 8);

    if (x > 240 || x < -8) return -1;
    if (y > 160 || y < -8) return -1;
    
    for (int i = 0; i < (8 * FONT_SCALE); ++i)
    {
        for (int j = 0; j < 8 * FONT_SCALE; ++j)
        {
            u8 a = (*font & (128 >> (j / FONT_SCALE))) ? color : 0;
            if (a)
                draw_plot_raw(x + j, y + i, a);
        }
        if(i % FONT_SCALE == 0)
            ++font;
    }
    return x + (8 * FONT_SCALE);
}

void video_print(const char* str, int c)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\n')
        {
            video_line_y += 8;
            video_line_x = 0;
            continue;
        }
        else if (str[i] == '\r')
        {
            video_line_x = 0;
            continue;
        }
        draw_char(video_line_x, video_line_y, c, str[i]);
        video_line_x += 6;
    }
}

void video_printf(const char *format, ...)
{
    char tmp[50];

    va_list args;
    va_start(args, format);

    vsnprintf(tmp, 50, format, args);
    video_print(tmp, video_color);

    va_end(args);
}


void video_init()
{
    //REG_DISPCNT |= LCDC_OFF;

    video_line_x = 0;
    video_line_y = 0;
    
    BG_COLORS[0] = 0x0;
    BG_COLORS[1] = 0x0;
    BG_COLORS[2] = 0x7FFF;
    BG_COLORS[3] = RGB8(0xFF, 0, 0);
    BG_COLORS[4] = RGB8(0, 0xFF, 0);
    BG_COLORS[5] = RGB8(0, 0, 0xFF);
    BG_COLORS[6] = RGB8(0xFF, 0, 0xFF);
    BG_COLORS[7] = RGB8(0, 0xFF, 0xFF);
    BG_COLORS[7] = RGB8(0xFF, 0xFF, 0);

    OBJ_COLORS[0] = 0x0;
    OBJ_COLORS[1] = 0x0;
    OBJ_COLORS[2] = 0x7FFF;

    //memset((void*)0x06000000, 0, 0x18000);

    SetMode(MODE_0 | BG0_ON | OBJ_ON);
    BGCTRL[1] = BG_SIZE_0 | CHAR_BASE(BG1_CHARBASE) | SCREEN_BASE(BG0_MAPBASE);
    BGCTRL[0] = BG_SIZE_0 | CHAR_BASE(BG0_CHARBASE) | SCREEN_BASE(BG0_MAPBASE);
    //REG_BG0CNT = BG_SIZE_0 | CHAR_BASE(BG0_CHARBASE) | BG_MAP_BASE(BG0_MAPBASE);
    //REG_BG1CNT = BG_SIZE_0 | CHAR_BASE(BG1_CHARBASE) | BG_MAP_BASE(BG0_MAPBASE);

    //*(u16*)CHAR_BASE_ADR(BG0_CHARBASE) = 0x0102;
    for (int i = 0; i < 20-3; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            *(u16*)(MAP_BASE_ADR(BG0_MAPBASE)+(j*2)+((i+1)*2*32)) = (j+(i*32))+1;
        }
    }

    /*for (int j = 0; j < (160/8); j++)
    {
        for (int i = 0; i < 50; i++)
        {
            draw_char(i*6, i+(j*8), 2+(i%4), 'A'+i);
        }
    }*/
    //video_print("asdf", 2);
}

void video_clear()
{
    memset(CHAR_BASE_ADR(video_whichbuf ? BG1_CHARBASE : BG0_CHARBASE), 0, 0x4000);
    video_line_x = 0;
    video_line_y = 0;
    video_color = 2;
}

void video_swap()
{
    VBlankIntrWait();
    if (video_whichbuf)
    {
        SetMode(MODE_0 | BG1_ON | OBJ_ON);
    }
    else
    {
        SetMode(MODE_0 | BG0_ON | OBJ_ON);
    }

    video_whichbuf = !video_whichbuf;
}

void menu_draw()
{
    int res;
    FILINFO fno = {0};
    DIR dir = {};

    video_clear();

    video_printf("\n%s\n", menu_curdir);
#if 1
    res = f_opendir(&dir, "sdmc:/");
    int iter = 0;
    if(res == FR_OK)
    {
        while (1)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fname[0] == '.') continue;
            video_color = 4;
            video_printf(" %c ", menu_cur_sel == iter ? '>' : ' ');
            video_color = 2;
            video_printf("%s\n", fno.fname);

            if (menu_cur_sel == iter)
            {
                strcpy(menu_boot_sel, menu_curdir);
                strcat(menu_boot_sel, fno.fname);
            }

            iter++;
        }
    }
    f_closedir(&dir);
#endif
    video_swap();
    video_dirty = 0;
}

void fs_init()
{
    f_mount(&fatfs, "sdmc:", 1);

    strcpy(menu_curdir, "sdmc:/");
}

void test_read()
{
    
    FIL file = {0};
    
    FILINFO fno = {0};
    UINT btx = 0;
    int res;

    
    //const char* to_load = "sdmc:/GBASYS/GBAOS.gba";
    const char* to_load = menu_boot_sel;//"sdmc:/Ruby.gba";
    video_printf("%s\n", to_load);
    //const char* to_load = "sdmc:/gba-switch-bios_mb.gba";
#if 1
    res = f_open(&file, to_load, FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        size_t total = f_size(&file);
        size_t read = 0;

        video_printf("Reading %s... sz 0x%08zx\n", to_load, total);

        while (total)
        {
            u32 test = 0;

            res = f_read(&file, (void*)(0x08000000 + read), f_size(&file), &btx);
            if(res != FR_OK) {
                video_printf("Error while reading, %x\n", res);
                break;
            }
            //(void*)(0x0A000000 + read)
            //*(u32*)(0x08000000 + read) = test;
            if (btx > total)
                break;

            total -= btx;
            read += btx;
            //iprintf("%x\n", read);
        }

        f_close(&file);
    }
#endif
}

int main(void) 
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    GBA_WAITCNT = 0;//0x4317;

    bi_init();
    bi_rtc_on();
    bi_set_rom_bank(0);
    bi_set_save_type(BI_SAV_FLA128);


    u8 idk = 0;//EDIO_startup();

    video_init();

    //consoleDemoInit();

    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");
    video_printf("Hello world!\n");

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
        if (kd & KEY_A)
        {
            video_clear();
            video_whichbuf = 0;
            video_clear();
            SetMode(MODE_0 | BG0_ON | OBJ_ON);

            test_read();
            //video_swap();
            //
            video_printf("\nTest SD Read:\n%08x %08x\n", *(u32*)0x08000000, *(u32*)0x08000004, (void*)0x080000AC);
            //while (1);
            bi_reboot(1);
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


