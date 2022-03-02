/* 
 * File:   sys.h
 * Author: krik
 *
 * Created on December 15, 2015, 1:12 PM
 */

#ifndef SYS_H
#define	SYS_H

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

#define RGB16(r,g,b)  ((r)+(g<<5)+(b<<10)) 
#define GBA_DISPSTAT *((volatile u16*)0x4000004)
#define GBA_VCTR *((volatile u16*)0x4000006)
#define GBA_TIMER0_VAL *((vu16 *)0x4000100)
#define GBA_TIMER0_CFG *((vu16 *)0x4000102)
#define GBA_IO_RCNT *((vu16 *)0x4000134)
#define GBA_IO_SIOCNT *((vu16 *)0x4000128)
#define GBA_IO_SIODAT *((vu16 *)0x400012A)

#define JOY_DELAY 32

#define JOY_B   0x0002
#define JOY_A   0x0001
#define JOY_SEL 0x0004
#define JOY_STA 0x0008
#define JOY_U   0x0040
#define JOY_D   0x0080
#define JOY_L   0x0020
#define JOY_R   0x0010

#define SYS_BR_ROWS 15

#define DMA_SRC *((vu32 *)0x40000D4)
#define DMA_DST *((vu32 *)0x40000D8)
#define DMA_LEN *((vu16 *)0x40000DC)
#define DMA_CTR *((vu16 *)0x40000DE)


#define PAL_BG_1 256

#define SCREEN_W 30
#define SCREEN_H 20 
#define PLAN_W 32
#define PLAN_H 20
#define PLAN_SIZE (PLAN_W * PLAN_H * 2)

extern u16 joy;

#endif	/* SYS_H */

