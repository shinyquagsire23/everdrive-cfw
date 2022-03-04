/*
 mgba.h
 Copyright (c) 2016 Jeffrey Pfau
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "types.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "mgba.h"

#define REG_DEBUG_ENABLE (vu16*) 0x4FFF780
#define REG_DEBUG_FLAGS (vu16*) 0x4FFF700
#define REG_DEBUG_STRING (char*) 0x4FFF600

u8 mgba_print_on = 0;

void mgba_printf(int level, const char* ptr, ...) {
   if (!mgba_print_on) return;

	va_list args;
	level &= 0x7;
	va_start(args, ptr);
	vsnprintf(REG_DEBUG_STRING, 0x100, ptr, args);
	va_end(args);
	*REG_DEBUG_FLAGS = level | 0x100;
}

int mgba_open(void) {
	*REG_DEBUG_ENABLE = 0xC0DE;
	int good = *REG_DEBUG_ENABLE == 0x1DEA;
   if (good)
   {
      mgba_print_on = 1;
   }
   return good;
}

void mgba_close(void) {
	*REG_DEBUG_ENABLE = 0;
   mgba_print_on = 0;
}