#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "pm64.h"

vec3f_t *vec3f_py(vec3f_t *r, float p, float y);
void vec3f_pyangles(vec3f_t *a, float *p, float *y);
vec3f_t *vec3f_add(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_sub(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_mul(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_scale(vec3f_t *r, vec3f_t *a, float s);
vec3f_t *vec3f_cross(vec3f_t *r, vec3f_t *a, vec3f_t *b);
float vec3f_dot(vec3f_t *a, vec3f_t *b);
float vec3f_mag(vec3f_t *a);
float vec3f_cos(vec3f_t *a, vec3f_t *b);
float vec3f_angle(vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_norm(vec3f_t *r, vec3f_t *a);
vec3f_t *vec3f_proj(vec3f_t *r, vec3f_t *a, vec3f_t *b);
vec3f_t *vec3f_rej(vec3f_t *r, vec3f_t *a, vec3f_t *b);
float angle_dif(float a, float b);
vec3f_t *vec3f_xfmw(vec3f_t *r, vec3f_t *a, float w, MtxF *b);

#endif
