#ifndef MATRIX_H
#define MATRIX_H

#include "gg.h"

extern void loadIdentity(GLfloat *m);
extern void ortho(GLfloat *m, float left, float right, float bottom, float top, float zNear, float zFar);
extern void frustum(GLfloat *m, float left, float right, float bottom, float top, float zNear, float zFar);
extern void perspective(GLfloat *m, float fovy, float aspect, float zNear, float zFar);
extern void lookat(GLfloat *m, float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz);
extern void translate(GLfloat *m, float x, float y, float z);
extern void scale(GLfloat *m, float x, float y, float z);
extern void rotate(GLfloat *r, float x, float y, float z, float a);
extern void normal(GLfloat *g, const float *m);
extern bool inverse(GLfloat *m2, const GLfloat *m1);
extern void multiply(GLfloat *m, const GLfloat *m1, const GLfloat *m2);
extern void transform(GLfloat *v2, const GLfloat *m, const GLfloat *v1);
extern void normalize(GLfloat *v2, const GLfloat *v1);

#endif
