#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "common.h"
#include "gu.h"

Vec3f *vec3fPy(Vec3f *r, f32 p, f32 y);
void vec3fPyangles(Vec3f *a, f32 *p, f32 *y);
Vec3f *vec3fAdd(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3fSub(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3fMul(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3fScale(Vec3f *r, Vec3f *a, f32 s);
Vec3f *vec3fCross(Vec3f *r, Vec3f *a, Vec3f *b);
f32 vec3fDot(Vec3f *a, Vec3f *b);
f32 vec3fMag(Vec3f *a);
f32 vec3fCos(Vec3f *a, Vec3f *b);
f32 vec3fAngle(Vec3f *a, Vec3f *b);
Vec3f *vec3fNorm(Vec3f *r, Vec3f *a);
Vec3f *vec3fProj(Vec3f *r, Vec3f *a, Vec3f *b);
Vec3f *vec3fRej(Vec3f *r, Vec3f *a, Vec3f *b);
f32 angleDif(f32 a, f32 b);
Vec3f *vec3fXfmw(Vec3f *r, Vec3f *a, f32 w, MtxF *b);

#endif
