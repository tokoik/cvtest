#version 120

// ����
const vec4 lamb   = vec4(0.2, 0.2, 0.2, 1.0);   // �������x
const vec4 ldiff  = vec4(1.0, 1.0, 1.0, 0.0);   // �������x�̊g�U���ː���
const vec4 lspec  = vec4(1.0, 1.0, 1.0, 0.0);   // �������x�̋��ʔ��ː���

// �ގ�
const vec4 kamb   = vec4(0.6, 0.6, 0.6, 1.0);   // �����̔��ˌW��
const vec4 kdiff  = vec4(0.6, 0.6, 0.6, 1.0);   // �g�U���ˌW��
const vec4 kspec  = vec4(0.4, 0.4, 0.4, 1.0);   // ���ʔ��ˌW��
const float kshi  = 40.0;                       // �P���W��

// �@���}�b�v�̍����̃X�P�[��
const float nz = 0.1;

// RGB -> Gray
const vec4 togray = vec4(0.29891, 0.58661, 0.11448, 0.0);

// �o�[�e�b�N�X�V�F�[�_������炤
varying vec3 l;   // �����x�N�g��
varying vec3 v;   // ���_�ʒu

// �e�N�X�`��
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
