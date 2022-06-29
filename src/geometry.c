#include <math.h>
#include "pm64.h"
#include "gu.h"

Vec3f *vec3f_py(Vec3f *r, f32 p, f32 y) {
    Vec3f v = {
        -sin(y) * cos(p),
        -sin(p),
        -cos(y) * cos(p),
    };
    *r = v;
    return r;
}

void vec3f_pyangles(Vec3f *a, f32 *p, f32 *y) {
    if (p) {
        *p = atan2f(-a->y, sqrtf(a->x * a->x + a->z * a->z));
    }
    if (y) {
        *y = atan2f(-a->x, -a->z);
    }
}

Vec3f *vec3f_add(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x + b->x,
        a->y + b->y,
        a->z + b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3f_sub(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x - b->x,
        a->y - b->y,
        a->z - b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3f_mul(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->x * b->x,
        a->y * b->y,
        a->z * b->z,
    };
    *r = v;
    return r;
}

Vec3f *vec3f_scale(Vec3f *r, Vec3f *a, f32 s) {
    Vec3f v = {
        a->x * s,
        a->y * s,
        a->z * s,
    };
    *r = v;
    return r;
}

Vec3f *vec3f_cross(Vec3f *r, Vec3f *a, Vec3f *b) {
    Vec3f v = {
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x,
    };
    *r = v;
    return r;
}

f32 vec3f_dot(Vec3f *a, Vec3f *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

f32 vec3f_mag(Vec3f *a) {
    return sqrtf(vec3f_dot(a, a));
}

f32 vec3f_cos(Vec3f *a, Vec3f *b) {
    return vec3f_dot(a, b) / vec3f_mag(a) / vec3f_mag(b);
}

f32 vec3f_angle(Vec3f *a, Vec3f *b) {
    return acosf(vec3f_cos(a, b));
}

Vec3f *vec3f_norm(Vec3f *r, Vec3f *a) {
    return vec3f_scale(r, a, 1.f / vec3f_mag(a));
}

Vec3f *vec3f_proj(Vec3f *r, Vec3f *a, Vec3f *b) {
    vec3f_norm(r, b);
    vec3f_scale(r, r, vec3f_dot(a, r));
    return r;
}

Vec3f *vec3f_rej(Vec3f *r, Vec3f *a, Vec3f *b) {
    vec3f_proj(r, a, b);
    vec3f_sub(r, a, r);
    return r;
}

f32 angle_dif(f32 a, f32 b) {
    f32 d = fmodf(a - b, M_PI * 2.f);
    if (d < -M_PI) {
        d += M_PI * 2.f;
    }
    if (d > M_PI) {
        d -= M_PI * 2.f;
    }
    return d;
}

Vec3f *vec3f_xfmw(Vec3f *r, Vec3f *a, f32 w, MtxF *b) {
    Vec3f v = {
        a->x * b->xx + a->y * b->yx + a->z * b->zx + w * b->wx,
        a->x * b->xy + a->y * b->yy + a->z * b->zy + w * b->wy,
        a->x * b->xz + a->y * b->yz + a->z * b->zz + w * b->wz,
    };
    *r = v;
    return r;
}
