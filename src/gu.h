#ifndef GU_H
#define GU_H
#include <stdint.h>
#include <n64.h>
#include "pm64.h"

// clang-format off
#define M_PI 3.14159265358979323846
#define guDefMtxF(xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww) \
    {                                                                             \
        .f = { xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww }   \
    }

typedef f32 MtxF_t[4][4];
typedef union {
    MtxF_t mf;
    f32 f[16];
    struct {
        f32 xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww;
    };
} MtxF;
// clang-format on

void guMtxIdent(Mtx *m);
void guMtxIdentF(MtxF *mf);
void guPerspectiveF(MtxF *mf, u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale);
void guMtxCatF(const MtxF *m, const MtxF *n, MtxF *r);
void guRotateF(MtxF *mf, f32 a, f32 x, f32 y, f32 z);
void guRotateRPYF(MtxF *mf, f32 r, f32 p, f32 h);
void guScaleF(MtxF *mf, f32 x, f32 y, f32 z);
void guTranslateF(MtxF *mf, f32 x, f32 y, f32 z);
void guMtxF2L(const MtxF *mf, Mtx *m);

#endif
