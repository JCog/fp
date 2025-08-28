#ifndef TYPES_H
#define TYPES_H
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
} Vec2b; // size = 0x02

typedef struct {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
} Vec2bu; // size = 0x02

typedef struct {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 z;
} Vec2XZf; // size = 0x08

typedef struct Vec2s {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
} Vec2s; // size = 0x04

typedef struct {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
} Vec3f; // size = 0x0C

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
} Vec3s; // size = 0x06

typedef f32 Matrix4f[4][4]; // size = 0x40

typedef struct Matrix4s {
    /* 0x00 */ s16 whole[4][4];
    /* 0x20 */ s16 frac[4][4];
} Matrix4s; // size = 0x40

#endif // TYPES_H
