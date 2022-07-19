#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "common.h"
#include "gu.h"

vec3f_t *vec3f_py(vec3f_t *r, f32 p, f32 y);
void vec3f_pyangles(vec3f_t *a, f32 *p, f32 *y);
vec3f_t *vec3f_add(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_sub(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_mul(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_scale(vec3f_t *r, vec3f_t *a, f32 s);
vec3f_t *vec3f_cross(vec3f_t *r, vec3f_t *a, vec3f_t *b);
f32 vec3f_dot(vec3f_t *a, vec3f_t *b);
f32 vec3f_mag(vec3f_t *a);
f32 vec3f_cos(vec3f_t *a, vec3f_t *b);
f32 vec3f_angle(vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_norm(vec3f_t *r, vec3f_t *a);
vec3f_t *vec3f_proj(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_rej(vec3f_t *r, vec3f_t *a, vec3f_t *b);
f32 angle_dif(f32 a, f32 b);
vec3f_t *vec3f_xfmw(vec3f_t *r, vec3f_t *a, f32 w, MtxF *b);

#endif
