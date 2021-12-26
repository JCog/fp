#include <math.h>
#include "pm64.h"
#include "gu.h"

vec3f_t *vec3f_py(vec3f_t *r, float p, float y) {
    vec3f_t v = {
        -sin(y) * cos(p),
        -sin(p),
        -cos(y) * cos(p),
    };
    *r = v;
    return r;
}

void vec3f_pyangles(vec3f_t *a, float *p, float *y) {
    if (p) {
        *p = atan2f(-a->y, sqrtf(a->x * a->x + a->z * a->z));
    }
    if (y) {
        *y = atan2f(-a->x, -a->z);
    }
}

vec3f_t *vec3f_add(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_t v = {
        a->x + b->x,
        a->y + b->y,
        a->z + b->z,
    };
    *r = v;
    return r;
}

vec3f_t *vec3f_sub(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_t v = {
        a->x - b->x,
        a->y - b->y,
        a->z - b->z,
    };
    *r = v;
    return r;
}

vec3f_t *vec3f_mul(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_t v = {
        a->x * b->x,
        a->y * b->y,
        a->z * b->z,
    };
    *r = v;
    return r;
}

vec3f_t *vec3f_scale(vec3f_t *r, vec3f_t *a, float s) {
    vec3f_t v = {
        a->x * s,
        a->y * s,
        a->z * s,
    };
    *r = v;
    return r;
}

vec3f_t *vec3f_cross(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_t v = {
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x,
    };
    *r = v;
    return r;
}

float vec3f_dot(vec3f_t *a, vec3f_t *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vec3f_mag(vec3f_t *a) {
    return sqrtf(vec3f_dot(a, a));
}

float vec3f_cos(vec3f_t *a, vec3f_t *b) {
    return vec3f_dot(a, b) / vec3f_mag(a) / vec3f_mag(b);
}

float vec3f_angle(vec3f_t *a, vec3f_t *b) {
    return acosf(vec3f_cos(a, b));
}

vec3f_t *vec3f_norm(vec3f_t *r, vec3f_t *a) {
    return vec3f_scale(r, a, 1.f / vec3f_mag(a));
}

vec3f_t *vec3f_proj(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_norm(r, b);
    vec3f_scale(r, r, vec3f_dot(a, r));
    return r;
}

vec3f_t *vec3f_rej(vec3f_t *r, vec3f_t *a, vec3f_t *b) {
    vec3f_proj(r, a, b);
    vec3f_sub(r, a, r);
    return r;
}

float angle_dif(float a, float b) {
    float d = fmodf(a - b, M_PI * 2.f);
    if (d < -M_PI) {
        d += M_PI * 2.f;
    }
    if (d > M_PI) {
        d -= M_PI * 2.f;
    }
    return d;
}

vec3f_t *vec3f_xfmw(vec3f_t *r, vec3f_t *a, float w, MtxF *b) {
    vec3f_t v = {
        a->x * b->xx + a->y * b->yx + a->z * b->zx + w * b->wx,
        a->x * b->xy + a->y * b->yy + a->z * b->zy + w * b->wy,
        a->x * b->xz + a->y * b->yz + a->z * b->zz + w * b->wz,
    };
    *r = v;
    return r;
}
