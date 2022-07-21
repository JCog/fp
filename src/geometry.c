#include "gu.h"
#include <math.h>

Vec3f *vec3fPy(Vec3f *r, f32 p, f32 y) {
    Vec3f v = {
        -sin(y) * cos(p),
        -sin(p),
        -cos(y) * cos(p),
    };
    *r = v;
    return r;
}

void vec3fPyangles(Vec3f *a, f32 *p, f32 *y) {
    if (p) {
        *p = atan2f(-a->y, sqrtf(a->x * a->x + a->z * a->z));
    }
    if (y) {
        *y = atan2f(-a->x, -a->z);
    }
}

Vec3f *vec3fAdd(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x + b->x,
        a->y + b->y,
        a->z + b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3fSub(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x - b->x,
        a->y - b->y,
        a->z - b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3fMul(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x * b->x,
        a->y * b->y,
        a->z * b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3fScale(Vec3f *r, Vec3f *a, f32 s) {
    Vec3f v = {
        a->x * s,
        a->y * s,
        a->z * s,
    };
    *r = v;
    return r;
}

Vec3f *vec3fCross(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x,
    };
    *r = v;
    return r;
}

f32 vec3fDot(Vec3f *a, Vec3f *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

f32 vec3fMag(Vec3f *a) {
    return sqrtf(vec3fDot(a, a));
}

f32 vec3fCos(Vec3f *a, Vec3f *b) {
    return vec3fDot(a, b) / vec3fMag(a) / vec3fMag(b);
}

f32 vec3fAngle(Vec3f *a, Vec3f *b) {
    return acosf(vec3fCos(a, b));
}

Vec3f *vec3fNorm(Vec3f *r, Vec3f *a) {
    return vec3fScale(r, a, 1.f / vec3fMag(a));
}

Vec3f *vec3fProj(Vec3f *r, Vec3f *a, Vec3f *b) {
    vec3fNorm(r, b);
    vec3fScale(r, r, vec3fDot(a, r));
    return r;
}

Vec3f *vec3fRej(Vec3f *r, Vec3f *a, Vec3f *b) {
    vec3fProj(r, a, b);
    vec3fSub(r, a, r);
    return r;
}

f32 angleDif(f32 a, f32 b) {
    f32 d = fmodf(a - b, M_PI * 2.f);
    if (d < -M_PI) {
        d += M_PI * 2.f;
    }
    if (d > M_PI) {
        d -= M_PI * 2.f;
    }
    return d;
}

Vec3f *vec3fXfmw(Vec3f *r, Vec3f *a, f32 w, MtxF *b) {
    Vec3f v = {
        a->x * b->xx + a->y * b->yx + a->z * b->zx + w * b->wx,
        a->x * b->xy + a->y * b->yy + a->z * b->zy + w * b->wy,
        a->x * b->xz + a->y * b->yz + a->z * b->zz + w * b->wz,
    };
    *r = v;
    return r;
}
