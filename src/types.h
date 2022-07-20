#ifndef _TYPES_H_
#define _TYPES_H_
#include <stdint.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef _Bool bool;

typedef struct {
    /* 0x00 */ s8 x;
    /* 0x01 */ s8 y;
} vec2b_t; // size = 0x02

typedef struct {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
} vec2bu_t; // size = 0x02

typedef struct {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 z;
} vec2xzf_t; // size = 0x08

typedef struct {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
} vec3f_t; // size = 0x0C

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
} vec3s_t; // size = 0x06

#endif //_TYPES_H_
