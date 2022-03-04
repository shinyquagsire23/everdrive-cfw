#ifndef _VIDEO_H
#define _VIDEO_H

#include "types.h"

#define BG0_CHARBASE (0)
#define BG1_CHARBASE (1)
#define BG0_MAPBASE (20)

#define FONT_SCALE (1)

#define FB_OFFSET(x, y)     ( ((((x%8)/4) ) * 2) + ((y % 8)*4) + ((y/8) * 0x400) + ((x/8) * 0x20) + 0x20 )
#define FB_SHIFT(c, x, y)     (c << (x % 4)*4)

extern int video_line_y;
extern int video_line_x;
extern u8 video_color;
extern u8 video_whichbuf;
extern u8 video_dirty;

int draw_char(int x, int y, u8 color, char c);

void video_print(const char* str, int c);
void video_printf(const char *format, ...);

void video_init();
void video_clear();
void video_swap();
void video_mode_singlebuffer();

#endif // _VIDEO_H