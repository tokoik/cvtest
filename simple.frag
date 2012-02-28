#version 120

// 光源
const vec4 lamb   = vec4(0.2, 0.2, 0.2, 1.0);   // 環境光強度
const vec4 ldiff  = vec4(1.0, 1.0, 1.0, 0.0);   // 光源強度の拡散反射成分
const vec4 lspec  = vec4(1.0, 1.0, 1.0, 0.0);   // 光源強度の鏡面反射成分

// 材質
const vec4 kamb   = vec4(0.6, 0.6, 0.6, 1.0);   // 環境光の反射係数
const vec4 kdiff  = vec4(0.6, 0.6, 0.6, 1.0);   // 拡散反射係数
const vec4 kspec  = vec4(0.4, 0.4, 0.4, 1.0);   // 鏡面反射係数
const float kshi  = 40.0;                       // 輝き係数

// 法線マップの高さのスケール
const float nz = 0.1;

// RGB -> Gray
const vec4 togray = vec4(0.29891, 0.58661, 0.11448, 0.0);

// バーテックスシェーダからもらう
varying vec3 l;   // 光線ベクトル
varying vec3 v;   // 頂点位置

// テクスチャ
uniform sampler2D dmap;  // diffuse color map
uniform vec2 size;       // texture size

void main(void)
{
  vec4 c0 = texture2D(dmap, gl_TexCoord[0].xy);
  vec4 cx = texture2D(dmap, gl_TexCoord[0].xy + vec2(1.0 / size.x, 0.0));
  vec4 cy = texture2D(dmap, gl_TexCoord[0].xy + vec2(0.0, 1.0 / size.y));
  
  vec3 n = normalize(vec3(dot(cx - c0, togray), dot(cy - c0, togray), nz));
  vec3 h = normalize(normalize(l) - normalize(v));
  
  vec4 iamb = kamb * lamb;
  vec4 idiff = max(dot(n, l), 0) * kdiff * ldiff;
  vec4 ispec = pow(max(dot(n, h), 0), kshi) * kspec * lspec;
  
  gl_FragColor = c0 * (iamb + idiff) + ispec;
}
