#include "video.h"

#include <stdarg.h>
#include <string.h>
#include <tonc.h>

#include "myVerdana.h"
#include "fatfs/ff.h"
#include "mgba.h"
#include "bios.h"
#include "utf8.h"

u8 video_color = 2;
u8 video_whichbuf = 0;
u8 video_dirty = 0;

extern const TFont myVerdanaFont;
extern uint utf8_decode_char(const char *ptr, char **endptr);

// Gets a pointer to str + len characters
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

// When we start rendering RTL strings, we need to find the last RTL character.
// This returns the number of characters in the RTL sprint, and a pointer to the last char
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
    tmp[127] = 0;
    tte_write(tmp);
    //tte_write(str);
}

void video_printf(const char *format, ...)
{
    char tmp[128];

    va_list args;
    va_start(args, format);

    vsnprintf(tmp, 128, format, args);

    tmp[127] = 0;
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

void video_use_inbuilt_fonts()
{
    tte_get_context()->font = &myVerdanaFont;
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