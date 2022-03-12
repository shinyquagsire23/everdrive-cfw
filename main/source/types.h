#ifndef _TYPES_H
#define _TYPES_H

#include <tonc_types.h>
#include <tonc_math.h>

#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1 << (x))

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

#define clamp(x, low, high) \
    ({ __typeof__(x) __x = (x); \
       __typeof__(low) __low = (low); \
       __typeof__(high) __high = (high); \
       __x > __high ? __high : (__x < __low ? __low : __x); })

#endif // _TYPES_H