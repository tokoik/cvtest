#version 120

// 光源
const vec3 light = vec3(3.0, 4.0, 5.0);                     // 方向ベクトル

// フラグメントシェーダに送る
varying vec3 l;   // 光線ベクトル
varying vec3 v;   // 頂点位置

void main(void)
{
  vec3 n = normalize(gl_NormalMatrix * gl_Normal);          // 法線ベクトル
  vec3 t = normalize(vec3(n.z, 0.0, -n.x));                 // 接線ベクトル
  vec3 b = cross(n, t);                                     // 従接線ベクトル
  mat3 m = transpose(mat3(t, b, n));                        // 接空間基底行列
  
  l = normalize(m * light);                                 // 接空間の光線ベクトル
  v = normalize(m * (gl_ModelViewMatrix * gl_Vertex).xyz);  // 接空間の視線ベクトル
  
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();
}
