#version 120

// ����
const vec3 light = vec3(3.0, 4.0, 5.0);                     // �����x�N�g��

// �t���O�����g�V�F�[�_�ɑ���
varying vec3 l;   // �����x�N�g��
varying vec3 v;   // ���_�ʒu

void main(void)
{
  vec3 n = normalize(gl_NormalMatrix * gl_Normal);          // �@���x�N�g��
  vec3 t = normalize(vec3(n.z, 0.0, -n.x));                 // �ڐ��x�N�g��
  vec3 b = cross(n, t);                                     // �]�ڐ��x�N�g��
  mat3 m = transpose(mat3(t, b, n));                        // �ڋ�Ԋ��s��
  
  l = normalize(m * light);                                 // �ڋ�Ԃ̌����x�N�g��
  v = normalize(m * (gl_ModelViewMatrix * gl_Vertex).xyz);  // �ڋ�Ԃ̎����x�N�g��
  
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();
}
