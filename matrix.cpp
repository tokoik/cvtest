#include <cmath>

#include "matrix.h"

/*
** 単位行列を設定する
**   m: 単位行列を設定する配列変数
*/
void loadIdentity(GLfloat *m)
{
  m[ 0] = m[ 5] = m[10] = m[15] = 1.0f;
  m[ 1] = m[ 2] = m[ 3] = m[ 4] =
  m[ 6] = m[ 7] = m[ 8] = m[ 9] =
  m[11] = m[12] = m[13] = m[14] = 0.0f;
}

/*
** 直交投影変換行列を求める
**   m: 求める直交投影変換行列
**   left, right: ビューボリュームの左右端
**   bottom, top: ビューボリュームの上下端
**   zNear, zFar: 前方面および後方面までの距離
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
** 透視投影変換行列を求める
**   m: 求める透視投影変換行列
**   left, right: 前方面の左右端
**   bottom, top: 前方面の上下端
**   zNear, zFar: 前方面および後方面までの距離
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
** 画角を指定して透視投影変換行列を求める
**   m: 求める透視投影変換行列
**   fovy: 画角（ラジアン）, aspect: ウィンドウの縦横比
**   zNear: 前方面までの距離, zFar: 後方面までの距離
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
** 視野変換行列を求める
**   m: 求める視野変換行列
**   ex, ey, ez: 視点の位置
**   tx, ty, tz: 目標点の位置
**   ux, uy, uz: 上方向のベクトル
*/
void lookat(GLfloat *m, float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz)
{
  float l;
  
  /* z 軸 = e - t */
  tx = ex - tx;
  ty = ey - ty;
  tz = ez - tz;
  l = sqrt(tx * tx + ty * ty + tz * tz);
  m[ 2] = tx / l;
  m[ 6] = ty / l;
  m[10] = tz / l;
  
  /* x 軸 = u x z 軸 */
  tx = uy * m[10] - uz * m[ 6];
  ty = uz * m[ 2] - ux * m[10];
  tz = ux * m[ 6] - uy * m[ 2];
  l = sqrt(tx * tx + ty * ty + tz * tz);
  m[ 0] = tx / l;
  m[ 4] = ty / l;
  m[ 8] = tz / l;
  
  /* y 軸 = z 軸 x x 軸 */
  m[ 1] = m[ 6] * m[ 8] - m[10] * m[ 4];
  m[ 5] = m[10] * m[ 0] - m[ 2] * m[ 8];
  m[ 9] = m[ 2] * m[ 4] - m[ 6] * m[ 0];
  
  /* 平行移動 */
  m[12] = -(ex * m[ 0] + ey * m[ 4] + ez * m[ 8]);
  m[13] = -(ex * m[ 1] + ey * m[ 5] + ez * m[ 9]);
  m[14] = -(ex * m[ 2] + ey * m[ 6] + ez * m[10]);
  
  /* 残り */
  m[ 3] = m[ 7] = m[11] = 0.0f;
  m[15] = 1.0f;
}

/*
** 平行移動変換行列を求める
**   m: 求める平行移動変換行列
**   x, y, z: 平行移動量
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
** 拡大縮小変換行列を求める
**   m: 求める平行移動変換行列
**   x, y, z: 拡大縮小率
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
** 任意軸周りの回転変換行列を求める
**   r: 求める回転変換行列
**   x, y, z: 中心軸
**   a: 回転角（ラジアン）
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
** 法線変換行列を求める
**   g: 求める法線ベクトルの変換行列
**   m: 元の変換行列
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
** 逆行列を求める
**   m1: 元の変換行列
**   m2: m1 の逆行列
**   逆行列が求められなければ戻り値は false
*/
bool inverse(GLfloat *m2, const GLfloat *m1)
{
  int i, j, k;
  GLfloat lu[20], *plu[4], det;
  
  // j 行の要素の値の絶対値の最大値を plu[j][4] に求める
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
  
  // ピボットを考慮した LU 分解
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
  
  // 逆行列を求める
  for (k = 0; k < 4; ++k) {
    // m2 に単位行列を設定する
    for (i = 0; i < 4; ++i) {
      m2[i * 4 + k] = (plu[i] == lu + k * 5) ? 1.0f : 0.0f;
    }
    // lu から逆行列を求める
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
** 4×4 行列どうしの積を求める
**   m ← m1 * m2
*/
void multiply(GLfloat *m, const GLfloat *m1, const GLfloat *m2)
{
  for (int i = 0; i < 16; ++i) {
    int j = i & 3, k = i & ~3;
    
    // 配列変数に行列が転置された状態で格納されていることを考慮している
    m[i] = m1[0 + j] * m2[k + 0] + m1[4 + j] * m2[k + 1] + m1[8 + j] * m2[k + 2] + m1[12 + j] * m2[k + 3];
  }
}

/*
** ベクトルに行列をかける
**   v2 ← m * v1
*/
void transform(GLfloat *v2, const GLfloat *m, const GLfloat *v1)
{
  // 配列変数に行列が転置された状態で格納されていることを考慮している
  v2[0] = m[ 0] * v1[0] + m[ 1] * v1[1] + m[ 2] * v1[2] + m[ 3] * v1[3];
  v2[1] = m[ 4] * v1[0] + m[ 5] * v1[1] + m[ 6] * v1[2] + m[ 7] * v1[3];
  v2[2] = m[ 8] * v1[0] + m[ 9] * v1[1] + m[10] * v1[2] + m[11] * v1[3];
  v2[3] = m[12] * v1[0] + m[13] * v1[1] + m[14] * v1[2] + m[15] * v1[3];
}

/*
** ベクトル v1 を正規化したものを求める
**   v2 ← v1 を正規化
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
