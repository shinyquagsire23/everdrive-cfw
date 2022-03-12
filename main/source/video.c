#include "video.h"

#include <stdarg.h>
#include <string.h>
#include <tonc.h>

#include "myVerdana.h"
#include "fatfs/ff.h"
#include "mgba.h"

u8 video_color = 2;
u8 video_whichbuf = 0;
u8 video_dirty = 0;

extern const TFont myVerdanaFont;

#if 0
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>


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
#endif

static inline void draw_plot_raw(int x, int y, u8 c)
{

}

int draw_char(int x, int y, u8 color, char c)
{
    return 0;
}

char* count_chars(const char* str, int len)
{
    const char* str_iter = str;
    const char* str_iter_next = str_iter;
    int num_chars = 0;
    while (len && *str_iter)
    {
        if (((uint8_t)*str_iter) >= 0x80) {
            utf8_decode_char(str_iter, &str_iter_next);
        }
        else
        {
            str_iter_next++;
        }
        str_iter = str_iter_next;
        num_chars++;

        if (num_chars == len)
            break;
    }
    return str_iter;
}

int get_end_notflipped(const char* str, char** out)
{
    const char* str_iter = str;
    const char* str_iter_next = str_iter;
    int num_chars = 0;
    while (*str_iter)
    {
        if (((uint8_t)*str_iter) >= 0x80) {
            uint ch = utf8_decode_char(str_iter, &str_iter_next);
            if (ch >= 0x590 && ch < 0x700)
            {
                ;
            }
            else
            {
                break;
            }
        }
        else
        {
            // TODO a LUT for unicode flag stuff
            str_iter_next++;
            uint ch = *str_iter;
            if (ch == ' '
                || (ch >= '0' && ch <= '9'))
            {
                ;
            }
            else
            {
                break;
            }
        }
        str_iter = str_iter_next;
        num_chars++;
    }
    if (out)
        *out = str_iter;
    return num_chars;
}

void flip_numbers(const char* str)
{

}

void video_print(const char* str, int c)
{
    char tmp[128];
    const char* str_iter = str;
    const char* str_iter_next = str_iter;
    char* tmp_iter = tmp;
    memset(tmp, 0, sizeof(tmp));
    while (*str_iter)
    {
        if (((uint8_t)*str_iter) >= 0x80) {
            uint ch = utf8_decode_char(str_iter, &str_iter_next);

            // Flipped characters. We kinda fake it by just manually moving bytes to the rendered order.
            if (ch >= 0x590 && ch < 0x700)
            {
                char* last_flipped;
                int num = get_end_notflipped(str_iter, &last_flipped);

                int last_num_cnt = 0;
                char* last_num_end = NULL;
                char* last_num_out = NULL;
                for (int i = 0; i < num; i++)
                {
                    const char* next_end = count_chars(str_iter, num-i);
                    const char* next = count_chars(str_iter, num-i-1);

                    // If we start rendering a number, take note so that we can flip the flipping
                    uint ch = utf8_decode_char(next, NULL);

                    // TODO a LUT for unicode flag stuff
                    if ((ch >= '0' && ch <= '9') || (ch >= 0x660 && ch <= 0x669))
                    {
                        if (!last_num_end)
                        {
                            last_num_end = next_end;
                            last_num_out = tmp_iter;
                        }
                    }
                    else
                    {
                        if (last_num_end)
                        {
                            char* last_num = next;
                            //mgba_printf("%x\n", last_num_cnt);

                            for (int j = 0; j < last_num_cnt; j++)
                            {
                                const char* num_next_end = count_chars(last_num, j+2);
                                const char* num_next = count_chars(last_num, j+1);

                                memcpy(last_num_out, num_next, num_next_end - num_next);
                                last_num_out += (num_next_end - num_next);
                                //mgba_printf("%s\n", num_next);
                            }

                            last_num_cnt = 0;
                            last_num_end = NULL;
                            last_num_out = NULL;
                        }
                    }
                    
                    if (last_num_end)
                        last_num_cnt++;

                    //mgba_printf("%p %p / %p %p\n", next, next_end, str_iter, last_flipped);
                    memcpy(tmp_iter, next, next_end - next);
                    tmp_iter += (next_end - next);
                }

                str_iter_next = last_flipped;
            }
            else
            {
                memcpy(tmp_iter, str_iter, str_iter_next - str_iter);
                tmp_iter += (str_iter_next - str_iter);
                //mgba_printf("asdf %s %s\n", str_iter, tmp);
            }
        }
        else
        {
            *(tmp_iter++) = *(str_iter++);

            //mgba_printf("asdf %s %s\n", str_iter, tmp);
            str_iter_next = str_iter;
        }

        str_iter = str_iter_next;
    }
    //mgba_printf("asdf %s\n", tmp);
    tte_write(tmp);
    //tte_write(str);
}

void video_printf(const char *format, ...)
{
    char tmp[128];

    va_list args;
    va_start(args, format);

    vsnprintf(tmp, 128, format, args);
    video_print(tmp, video_color);

    va_end(args);
}

#define FONT_PSRAM (0x9C00000)
#define FONT_VW_PSRAM (FONT_PSRAM - 0x20000)

extern const unsigned char myVerdanaWidths[735];
const TFont myVerdanaFont_sd __attribute__((aligned(4))) =
{
(void*)FONT_PSRAM,
(void*)FONT_VW_PSRAM,
0,
0x20,
0xFFFF,
8,
12,
16,
16,
32,
1,
0

};

int video_loadfile(const char* to_load, intptr_t dest)
{
    FIL file = {0};
    UINT btx = 0;
    int res;

    mgba_printf("Loading %s to %p...\n", to_load, dest);

    bi_set_rom_bank(0);

    res = f_open(&file, to_load, FA_OPEN_EXISTING | FA_READ);
    if(res == FR_OK)
    {
        size_t total = f_size(&file);
        size_t chunk_size = f_size(&file);
        size_t read = 0;

        mgba_printf("Reading %s...\nsz 0x%08zx", to_load, total);
        //video_printf("Reading %s...\nsz 0x%08zx\n", to_load, total);

        while (total)
        {
            res = f_read(&file, (void*)((intptr_t)dest + read), chunk_size, &btx);
            if(res != FR_OK) {
                return 0;
            }

            if (btx > total) {
                break;
            }

            total -= btx;
            read += btx;
        }

        f_close(&file);
    }
    else
    {
        return 0;
    }
    return 1;
}

void video_init()
{
    

    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

    //tte_init_chr4c_default(0, BG_CBB(0) | BG_SBB(31));
    //tte_init_chr4c(0, BG_CBB(0) | BG_SBB(31), 0xF000, 0x0201, CLR_YELLOW<<16|CLR_WHITE, &myVerdanaFont, NULL);

    int has_fonts = video_loadfile("/GBASYS/font.img.bin", FONT_PSRAM) && video_loadfile("/GBASYS/font.vwflen.bin", FONT_VW_PSRAM);

    if (has_fonts)
        tte_init_chr4c(0, BG_CBB(0) | BG_SBB(31), 0xF000, 0x0201, CLR_YELLOW<<16|CLR_WHITE, &myVerdanaFont_sd, NULL);
    else
        tte_init_chr4c(0, BG_CBB(0) | BG_SBB(31), 0xF000, 0x0201, CLR_YELLOW<<16|CLR_WHITE, &myVerdanaFont, NULL);
    
    video_clear();
}

void video_clear()
{
    tte_erase_screen();
    tte_set_pos(0,0);
}

void video_swap()
{
    VBlankIntrWait();
}

void video_mode_singlebuffer()
{
    video_clear();
}