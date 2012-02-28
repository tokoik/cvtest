#include <cmath>

#include "matrix.h"

/*
** �P�ʍs���ݒ肷��
**   m: �P�ʍs���ݒ肷��z��ϐ�
*/
void loadIdentity(GLfloat *m)
{
  m[ 0] = m[ 5] = m[10] = m[15] = 1.0f;
  m[ 1] = m[ 2] = m[ 3] = m[ 4] =
  m[ 6] = m[ 7] = m[ 8] = m[ 9] =
  m[11] = m[12] = m[13] = m[14] = 0.0f;
}

/*
** ���𓊉e�ϊ��s������߂�
**   m: ���߂钼�𓊉e�ϊ��s��
**   left, right: �r���[�{�����[���̍��E�[
**   bottom, top: �r���[�{�����[���̏㉺�[
**   zNear, zFar: �O���ʂ���ь���ʂ܂ł̋���
*/
void ortho(GLfloat *m, float left, float right, float bottom, float top, float zNear, float zFar)
{
  m[ 0] =  2.0f / (right - left);
  m[ 5] =  2.0f / (top - bottom);
  m[10] = -2.0f / (zFar - zNear);
  m[12] = -(right + left) / (right - left);
  m[13] = -(top + bottom) / (top - bottom);
  m[14] = -(zFar + zNear) / (zFar - zNear);
  m[15] =  1.0f;
  m[ 1] = m[ 2] = m[ 3] = m[ 4] = m[ 6] = m[ 7] = m[ 8] = m[ 9] = m[11] = 0.0f;
}

/*
** �������e�ϊ��s������߂�
**   m: ���߂铧�����e�ϊ��s��
**   left, right: �O���ʂ̍��E�[
**   bottom, top: �O���ʂ̏㉺�[
**   zNear, zFar: �O���ʂ���ь���ʂ܂ł̋���
*/
void frustum(GLfloat *m, float left, float right, float bottom, float top, float zNear, float zFar)
{
  m[ 0] =  2.0f * zNear / (right - left);
  m[ 5] =  2.0f * zNear / (top - bottom);
  m[ 8] =  (right + left) / (right - left);
  m[ 9] =  (top + bottom) / (top - bottom);
  m[10] = -(zFar + zNear) / (zFar - zNear);
  m[11] = -1.0f;
  m[14] = -2.0f * zFar * zNear / (zFar - zNear);
  m[ 1] = m[ 2] = m[ 3] = m[ 4] = m[ 6] = m[ 7] = m[12] = m[13] = m[15] = 0.0f;
}

/*
** ��p���w�肵�ē������e�ϊ��s������߂�
**   m: ���߂铧�����e�ϊ��s��
**   fovy: ��p�i���W�A���j, aspect: �E�B���h�E�̏c����
**   zNear: �O���ʂ܂ł̋���, zFar: ����ʂ܂ł̋���
*/
void perspective(GLfloat *m, float fovy, float aspect, float zNear, float zFar)
{
  m[ 5] = 1.0f / tan(fovy * 0.5f);
  m[ 0] = m[ 5] / aspect;
  m[10] = -(zFar + zNear) / (zFar - zNear);
  m[11] = -1.0f;
  m[14] = -2.0f * zFar * zNear / (zFar - zNear);
  m[ 1] = m[ 2] = m[ 3] = m[ 4] = m[ 6] = m[ 7] = m[ 8] = m[ 9] = m[12] = m[13] = m[15] = 0.0f;
}

/*
** ����ϊ��s������߂�
**   m: ���߂鎋��ϊ��s��
**   ex, ey, ez: ���_�̈ʒu
**   tx, ty, tz: �ڕW�_�̈ʒu
**   ux, uy, uz: ������̃x�N�g��
*/
void lookat(GLfloat *m, float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz)
{
  float l;
  
  /* z �� = e - t */
  tx = ex - tx;
  ty = ey - ty;
  tz = ez - tz;
  l = sqrt(tx * tx + ty * ty + tz * tz);
  m[ 2] = tx / l;
  m[ 6] = ty / l;
  m[10] = tz / l;
  
  /* x �� = u x z �� */
  tx = uy * m[10] - uz * m[ 6];
  ty = uz * m[ 2] - ux * m[10];
  tz = ux * m[ 6] - uy * m[ 2];
  l = sqrt(tx * tx + ty * ty + tz * tz);
  m[ 0] = tx / l;
  m[ 4] = ty / l;
  m[ 8] = tz / l;
  
  /* y �� = z �� x x �� */
  m[ 1] = m[ 6] * m[ 8] - m[10] * m[ 4];
  m[ 5] = m[10] * m[ 0] - m[ 2] * m[ 8];
  m[ 9] = m[ 2] * m[ 4] - m[ 6] * m[ 0];
  
  /* ���s�ړ� */
  m[12] = -(ex * m[ 0] + ey * m[ 4] + ez * m[ 8]);
  m[13] = -(ex * m[ 1] + ey * m[ 5] + ez * m[ 9]);
  m[14] = -(ex * m[ 2] + ey * m[ 6] + ez * m[10]);
  
  /* �c�� */
  m[ 3] = m[ 7] = m[11] = 0.0f;
  m[15] = 1.0f;
}

/*
** ���s�ړ��ϊ��s������߂�
**   m: ���߂镽�s�ړ��ϊ��s��
**   x, y, z: ���s�ړ���
*/
void translate(GLfloat *m, float x, float y, float z)
{
  m[12] = x;
  m[13] = y;
  m[14] = z;
  m[ 0] = m[ 5] = m[10] = m[15] = 1.0f;
  m[ 1] = m[ 2] = m[ 3] =
  m[ 4] = m[ 6] = m[ 7] =
  m[ 8] = m[ 9] = m[11] = 0.0f;
}

/*
** �g��k���ϊ��s������߂�
**   m: ���߂镽�s�ړ��ϊ��s��
**   x, y, z: �g��k����
*/
void scale(GLfloat *m, float x, float y, float z)
{
  m[ 0] = x;
  m[ 5] = y;
  m[10] = z;
  m[ 1] = m[ 2] = m[ 3] = m[ 4] =
  m[ 6] = m[ 7] = m[ 8] = m[ 9] =
  m[11] = m[12] = m[13] = m[14] = 0.0f;
  m[15] = 1.0f;
}

/*
** �C�ӎ�����̉�]�ϊ��s������߂�
**   r: ���߂��]�ϊ��s��
**   x, y, z: ���S��
**   a: ��]�p�i���W�A���j
*/
void rotate(GLfloat *r, float x, float y, float z, float a)
{
  float d = sqrt(x * x + y * y + z * z);
  
  if (d > 0.0f) {
    float l  = x / d, m  = y / d, n  = z / d;
    float l2 = l * l, m2 = m * m, n2 = n * n;
    float lm = l * m, mn = m * n, nl = n * l;
    float c = cos(a), c1 = 1.0f - c;
    float s = sin(a);
    
    r[ 0] = (1.0f - l2) * c + l2;
    r[ 1] = lm * c1 + n * s;
    r[ 2] = nl * c1 - m * s;
    r[ 3] = 0.0f;
    
    r[ 4] = lm * c1 - n * s;
    r[ 5] = (1.0f - m2) * c + m2;
    r[ 6] = mn * c1 + l * s;
    r[ 7] = 0.0f;
    
    r[ 8] = nl * c1 + m * s;
    r[ 9] = mn * c1 - l * s;
    r[10] = (1.0f - n2) * c + n2;
    r[11] = 0.0f;
    
    r[12] = r[13] = r[14] = 0.0f;
    r[15] = 1.0f;
  }
}

/*
** �@���ϊ��s������߂�
**   g: ���߂�@���x�N�g���̕ϊ��s��
**   m: ���̕ϊ��s��
*/
void normal(GLfloat *g, const GLfloat *m)
{
  g[ 0] = m[ 5] * m[10] - m[ 6] * m[ 9];
  g[ 1] = m[ 6] * m[ 8] - m[ 4] * m[10];
  g[ 2] = m[ 4] * m[ 9] - m[ 5] * m[ 8];
  g[ 4] = m[ 9] * m[ 2] - m[10] * m[ 1];
  g[ 5] = m[10] * m[ 0] - m[ 8] * m[ 2];
  g[ 6] = m[ 8] * m[ 1] - m[ 9] * m[ 0];
  g[ 8] = m[ 1] * m[ 6] - m[ 2] * m[ 5];
  g[ 9] = m[ 2] * m[ 4] - m[ 0] * m[ 6];
  g[10] = m[ 0] * m[ 5] - m[ 1] * m[ 4];
  g[ 3] = g[ 7] = g[11] = g[12] = g[13] = g[14] = g[15] = 0.0f;
}

/*
** �t�s������߂�
**   m1: ���̕ϊ��s��
**   m2: m1 �̋t�s��
**   �t�s�񂪋��߂��Ȃ���Ζ߂�l�� false
*/
bool inverse(GLfloat *m2, const GLfloat *m1)
{
  int i, j, k;
  GLfloat lu[20], *plu[4], det;
  
  // j �s�̗v�f�̒l�̐�Βl�̍ő�l�� plu[j][4] �ɋ��߂�
  for (j = 0; j < 4; ++j) {
    GLfloat max = fabs(*(plu[j] = lu + 5 * j) = *(m1++));
    for (i = 0; ++i < 4;) {
      GLfloat a = fabs(plu[j][i] = *(m1++));
      if (a > max) max = a;
    }
    if (max == 0.0) return false;
    plu[j][4] = 1.0f / max;
  }
  
  det = 1.0f;
  
  // �s�{�b�g���l������ LU ����
  for (j = 0; j < 4; ++j) {
    GLfloat max = fabs(plu[j][j] * plu[j][4]);
    i = j;
    for (k = j; ++k < 4;) {
      GLfloat a = fabs(plu[k][j] * plu[k][4]);
      if (a > max) {
        max = a;
        i = k;
      }
    }
    
    if (i > j) {
      GLfloat *t = plu[j];
      plu[j] = plu[i];
      plu[i] = t;
      det = -det;
    }
    if (plu[j][j] == 0.0) return false;
    det *= plu[j][j];
    
    for (k = j; ++k < 4;) {
      plu[k][j] /= plu[j][j];
      for (i = j; ++i < 4;) {
        plu[k][i] -= plu[j][i] * plu[k][j];
      }
    }
  }
  
  // �t�s������߂�
  for (k = 0; k < 4; ++k) {
    // m2 �ɒP�ʍs���ݒ肷��
    for (i = 0; i < 4; ++i) {
      m2[i * 4 + k] = (plu[i] == lu + k * 5) ? 1.0f : 0.0f;
    }
    // lu ����t�s������߂�
    for (i = 0; i < 4; ++i) {
      for (j = i; ++j < 4;) {
        m2[j * 4 + k] -= m2[i * 4 + k] * plu[j][i];
      }
    }
    for (i = 4; --i >= 0;){
      for (j = i; ++j < 4;) {
        m2[i * 4 + k] -= plu[i][j] * m2[j * 4 + k];
      }
      m2[i * 4 + k] /= plu[i][i];
    }
  }
  
  return true;
}

/*
** 4�~4 �s��ǂ����̐ς����߂�
**   m �� m1 * m2
*/
void multiply(GLfloat *m, const GLfloat *m1, const GLfloat *m2)
{
  for (int i = 0; i < 16; ++i) {
    int j = i & 3, k = i & ~3;
    
    // �z��ϐ��ɍs�񂪓]�u���ꂽ��ԂŊi�[����Ă��邱�Ƃ��l�����Ă���
    m[i] = m1[0 + j] * m2[k + 0] + m1[4 + j] * m2[k + 1] + m1[8 + j] * m2[k + 2] + m1[12 + j] * m2[k + 3];
  }
}

/*
** �x�N�g���ɍs���������
**   v2 �� m * v1
*/
void transform(GLfloat *v2, const GLfloat *m, const GLfloat *v1)
{
  // �z��ϐ��ɍs�񂪓]�u���ꂽ��ԂŊi�[����Ă��邱�Ƃ��l�����Ă���
  v2[0] = m[ 0] * v1[0] + m[ 1] * v1[1] + m[ 2] * v1[2] + m[ 3] * v1[3];
  v2[1] = m[ 4] * v1[0] + m[ 5] * v1[1] + m[ 6] * v1[2] + m[ 7] * v1[3];
  v2[2] = m[ 8] * v1[0] + m[ 9] * v1[1] + m[10] * v1[2] + m[11] * v1[3];
  v2[3] = m[12] * v1[0] + m[13] * v1[1] + m[14] * v1[2] + m[15] * v1[3];
}

/*
** �x�N�g�� v1 �𐳋K���������̂����߂�
**   v2 �� v1 �𐳋K��
*/
void normalize(GLfloat *v2, const GLfloat *v1)
{
  GLfloat l = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2];

  if (l > 0.0f) {
    l = sqrt(l);
    
    v2[0] = v1[0] / l;
    v2[1] = v1[1] / l;
    v2[2] = v1[2] / l;
    v2[3] = 0.0f;
  }
}
