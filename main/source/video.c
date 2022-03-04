#include "video.h"

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <stdarg.h>
#include <string.h>

extern const u8 msx_font[];
int video_line_y = 0;
int video_line_x = 0;

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

void video_mode_singlebuffer()
{
    video_clear();
    video_whichbuf = 0;
    video_clear();
    SetMode(MODE_0 | BG0_ON | OBJ_ON);
}