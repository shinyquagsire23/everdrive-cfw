/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * font.c - Debug Font.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: font.c 540 2005-07-08 19:35:10Z warren $
 */

#include "types.h"

const u8 msx_font[] __attribute((aligned(4))) = {
    0x00, 0x18, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x00,
    0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0x00,
    0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00, 0xC0, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x18, 0x00,
    0x40, 0xA0, 0x40, 0xA8, 0x90, 0x98, 0x60, 0x00, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x20, 0x40, 0x40, 0x40, 0x20, 0x10, 0x00, 0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00,
    0x20, 0xA8, 0x70, 0x20, 0x70, 0xA8, 0x20, 0x00, 0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,
    0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00, 0x20, 0x60, 0xA0, 0x20, 0x20, 0x20, 0xF8, 0x00,
    0x70, 0x88, 0x08, 0x10, 0x60, 0x80, 0xF8, 0x00, 0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70, 0x00,
    0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00, 0xF8, 0x80, 0xE0, 0x10, 0x08, 0x10, 0xE0, 0x00,
    0x30, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00, 0xF8, 0x88, 0x10, 0x20, 0x20, 0x20, 0x20, 0x00,
    0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00, 0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60, 0x00,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x20, 0x40,
    0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00,
    0xC0, 0x60, 0x30, 0x18, 0x30, 0x60, 0xC0, 0x00, 0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20, 0x00,
    0x70, 0x88, 0x08, 0x68, 0xA8, 0xA8, 0x70, 0x00, 0x20, 0x50, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,
    0xF0, 0x48, 0x48, 0x70, 0x48, 0x48, 0xF0, 0x00, 0x30, 0x48, 0x80, 0x80, 0x80, 0x48, 0x30, 0x00,
    0xE0, 0x50, 0x48, 0x48, 0x48, 0x50, 0xE0, 0x00, 0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00,
    0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0x00, 0x70, 0x88, 0x80, 0xB8, 0x88, 0x88, 0x70, 0x00,
    0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00, 0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00,
    0x38, 0x10, 0x10, 0x10, 0x90, 0x90, 0x60, 0x00, 0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00, 0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88, 0x00,
    0x88, 0xC8, 0xC8, 0xA8, 0x98, 0x98, 0x88, 0x00, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,
    0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00, 0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00,
    0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00, 0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70, 0x00,
    0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,
    0x88, 0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0x00, 0x88, 0x88, 0x88, 0xA8, 0xA8, 0xD8, 0x88, 0x00,
    0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00, 0x88, 0x88, 0x88, 0x70, 0x20, 0x20, 0x20, 0x00,
    0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00, 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x70, 0x00,
    0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70, 0x00,
    0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00,
    0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,
    0x80, 0x80, 0xB0, 0xC8, 0x88, 0xC8, 0xB0, 0x00, 0x00, 0x00, 0x70, 0x88, 0x80, 0x88, 0x70, 0x00,
    0x08, 0x08, 0x68, 0x98, 0x88, 0x98, 0x68, 0x00, 0x00, 0x00, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,
    0x10, 0x28, 0x20, 0xF8, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x68, 0x98, 0x98, 0x68, 0x08, 0x70,
    0x80, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x88, 0x00, 0x20, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00,
    0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x90, 0x60, 0x40, 0x40, 0x48, 0x50, 0x60, 0x50, 0x48, 0x00,
    0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00, 0x00, 0xD0, 0xA8, 0xA8, 0xA8, 0xA8, 0x00,
    0x00, 0x00, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00,
    0x00, 0x00, 0xB0, 0xC8, 0xC8, 0xB0, 0x80, 0x80, 0x00, 0x00, 0x68, 0x98, 0x98, 0x68, 0x08, 0x08,
    0x00, 0x00, 0xB0, 0xC8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x78, 0x80, 0xF0, 0x08, 0xF0, 0x00,
    0x40, 0x40, 0xF0, 0x40, 0x40, 0x48, 0x30, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x68, 0x00,
    0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00, 0x00, 0x00, 0x88, 0xA8, 0xA8, 0xA8, 0x50, 0x00,
    0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x88, 0x88, 0x98, 0x68, 0x08, 0x70,
    0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00, 0x18, 0x20, 0x20, 0x40, 0x20, 0x20, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x20, 0x20, 0x10, 0x20, 0x20, 0xC0, 0x00,
    0x40, 0xA8, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x00,
    0x20, 0x20, 0x78, 0x80, 0x80, 0x78, 0x20, 0x20, 0x18, 0x24, 0x20, 0xF8, 0x20, 0xE2, 0x5C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x50, 0x20, 0xF8, 0x20, 0xF8, 0x20, 0x00,
    0x20, 0x20, 0x20, 0x00, 0x20, 0x20, 0x20, 0x00, 0x38, 0x40, 0x30, 0x48, 0x48, 0x30, 0x08, 0x70,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00, 0xF8, 0x00, 0x00, 0x24, 0x48, 0x90, 0x48, 0x24, 0x00,
    0x00, 0x00, 0x00, 0xF8, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0xA0, 0x20, 0x40, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0x50, 0x50, 0x68, 0x80, 0x80,
    0x7C, 0xA8, 0xA8, 0x68, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x48, 0x24, 0x48, 0x90, 0x00,
    0x84, 0x88, 0x90, 0xA8, 0x58, 0xA8, 0x3C, 0x08, 0x84, 0x88, 0x90, 0xA8, 0x54, 0x84, 0x08, 0x1C,
    0xC0, 0x44, 0xC8, 0x54, 0xEC, 0x54, 0x9E, 0x04, 0x20, 0x00, 0x20, 0x40, 0x80, 0x88, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x50, 0x20, 0x50, 0x88, 0xF8, 0x88, 0x00,
    0x50, 0x00, 0x20, 0x50, 0x88, 0xF8, 0x88, 0x00, 0x20, 0x00, 0x20, 0x50, 0x88, 0xF8, 0x88, 0x00,
    0x3E, 0x50, 0x90, 0x9C, 0xF0, 0x90, 0x9E, 0x00, 0x70, 0x88, 0x80, 0x80, 0x88, 0x70, 0x20, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0xF8, 0x80, 0xF0, 0x80, 0xF8, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x50, 0x00, 0xC8, 0xA8, 0x98, 0x88, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x50, 0x00, 0x70, 0x88, 0x88, 0x70, 0x00,
    0x50, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x70, 0xA8, 0xA8, 0xA8, 0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x50, 0x00, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x48, 0x70, 0x48, 0x48, 0x70, 0xC0,
    0x20, 0x10, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00, 0x10, 0x20, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,
    0x20, 0x50, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00, 0x28, 0x50, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,
    0x48, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00, 0x20, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,
    0x00, 0x00, 0x6C, 0x12, 0x7E, 0x90, 0x6E, 0x00, 0x00, 0x70, 0x80, 0x80, 0x80, 0x70, 0x10, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,
    0x20, 0x10, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00, 0x50, 0x00, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,
    0x40, 0x20, 0x00, 0x60, 0x20, 0x20, 0x70, 0x00, 0x10, 0x20, 0x00, 0x60, 0x20, 0x20, 0x70, 0x00,
    0x20, 0x50, 0x00, 0x60, 0x20, 0x20, 0x70, 0x00, 0x50, 0x00, 0x00, 0x60, 0x20, 0x20, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xA0, 0x00, 0xA0, 0xD0, 0x90, 0x90, 0x00,
    0x40, 0x20, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x20, 0x40, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00,
    0x60, 0x90, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x50, 0xA0, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00,
    0x90, 0x00, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00, 0x20, 0x00, 0xF8, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x00, 0xA0, 0xA0, 0xA0, 0x50, 0x00,
    0x20, 0x40, 0x00, 0x90, 0x90, 0x90, 0x68, 0x00, 0x40, 0xA0, 0x00, 0xA0, 0xA0, 0xA0, 0x50, 0x00,
    0x90, 0x00, 0x00, 0x90, 0x90, 0x90, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x90, 0x90, 0xB0, 0x50, 0x10, 0xE0
};
