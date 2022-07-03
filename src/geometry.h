#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "pm64.h"
#include "gu.h"

Vec3f *vec3f_py(Vec3f *r, f32 p, f32 y);
void vec3f_pyangles(Vec3f *a, f32 *p, f32 *y);
Vec3f *vec3f_add(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3f_sub(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3f_mul(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3f_scale(Vec3f *r, Vec3f *a, f32 s);
Vec3f *vec3f_cross(Vec3f *r, Vec3f *a, Vec3f *b);
f32 vec3f_dot(Vec3f *a, Vec3f *b);
f32 vec3f_mag(Vec3f *a);
f32 vec3f_cos(Vec3f *a, Vec3f *b);
f32 vec3f_angle(Vec3f *a, Vec3f *b);
Vec3f *vec3f_norm(Vec3f *r, Vec3f *a);
Vec3f *vec3f_proj(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3f_rej(Vec3f *r, Vec3f *a, Vec3f *b);
f32 angle_dif(f32 a, f32 b);
Vec3f *vec3f_xfmw(Vec3f *r, Vec3f *a, f32 w, MtxF *b);

#endif
