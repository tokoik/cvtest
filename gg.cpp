#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include "gg.h"

/*
** �Q�[���O���t�B�b�N�X���_�̓s���ɂ��ƂÂ�������
*/
void ggInit(void)
{
  // Swap Interval �̐ݒ�
#if defined(WIN32)
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr <<  "Error: " << glewGetErrorString(err) << std::endl;
    exit(1);
  }
  if (!wglSwapIntervalEXT || wglSwapIntervalEXT(1) == FALSE)
    std::cerr << "Warning: Could not set swap interval." << std::endl;
#elif defined(X11)
#  if defined(GLX_EXT_swap_control) && !defined(__APPLE__)
  Display *dpy = glXGetCurrentDisplay();
  GLXDrawable drawable = glXGetCurrentDrawable();
  if (drawable && glXSwapIntervalEXT(dpy, drawable, 1))
    std::cerr << "Warning: Could not set swap interval" << std::endl;
#  else
  if (glXSwapIntervalSGI(1))
    std::cerr << "Warning: Could not set swap interval" << std::endl;
#  endif
#elif defined(__APPLE__)
  int swap_interval = 1;
  CGLContextObj cgl_context = CGLGetCurrentContext();
  if (CGLSetParameter(cgl_context, kCGLCPSwapInterval, &swap_interval))
    std::cerr << "Warning: Could not set swap interval" << std::endl;
#endif
  
  // �o�[�W�����`�F�b�N
  if (atof(reinterpret_cast<const char *>(glGetString(GL_VERSION))) < 2.1) {
    std::cerr << "Error: This program requires OpenGL 2.1 or lator." << std::endl;
    exit(1);
  }
}

/*
** �V�F�[�_�[�̃\�[�X�v���O�������������ɓǂݍ���
*/
static bool readShaderSource(GLuint shader, const char *name)
{
  bool ret = true; // �I���X�e�[�^�X�itrue �Ȃ�G���[�j
  
  // �\�[�X�t�@�C�����J��
  std::ifstream file(name, std::ios::binary);

  if (file.fail()) {
    // �J���Ȃ�����
    std::cerr << "Error: Can't open source file: " << name << std::endl;
  }
  else {
    // �t�@�C���̖����Ɉړ������݈ʒu�i���t�@�C���T�C�Y�j�𓾂�
    file.seekg(0L, std::ios::end);
    GLsizei length = file.tellg();
    
    // �t�@�C���T�C�Y�̃��������m��
    char *buffer = new GLchar[length];
    
    // �t�@�C����擪����ǂݍ���
    file.seekg(0L, std::ios::beg);
    file.read(buffer, length);

    if (file.bad()) {
      // ���܂��ǂݍ��߂Ȃ�����
      std::cerr << "Error: Could not read souce file: " << name << std::endl;
    }
    else {
      // �V�F�[�_�̃\�[�X�v���O�����̃V�F�[�_�I�u�W�F�N�g�ւ̓ǂݍ���
      const GLchar *source[] = { buffer };
      glShaderSource(shader, 1, source, &length);
      ret = false;
    }
    file.close();
    
    // �ǂݍ��݂Ɏg�������������J������
    delete[] buffer;
  }
  
  return ret;
}

/*
** �V�F�[�_�̏���\������
*/
static void printShaderInfoLog(GLuint shader)
{
  // �V�F�[�_�̃R���p�C�����̃��O�̒������擾����
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1) {
    // �V�F�[�_�̃R���p�C�����̃��O�̓��e���擾����
    GLchar *infoLog = new GLchar[bufSize];
    GLsizei length;
    glGetShaderInfoLog(shader, bufSize, &length, infoLog);
    std::cerr << "InfoLog:\n" << infoLog << '\n' << std::endl;
    delete[] infoLog;
  }
}

/*
** �v���O�����̏���\������
*/
static void printProgramInfoLog(GLuint program)
{
  // �V�F�[�_�̃����N���̃��O�̒������擾����
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1) {
    // �V�F�[�_�̃����N���̃��O�̓��e���擾����
    GLchar *infoLog = new GLchar[bufSize];
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, infoLog);
    std::cerr << "InfoLog:\n" << infoLog << '\n' << std::endl;
    delete[] infoLog;
  }
}

/*
** �V�F�[�_�[�\�[�X�t�@�C���̓ǂݍ���
*/
GLuint loadShader(const char *vert, const char *frag, const char *geom)
{
  GLint compiled, linked; // �R���p�C������, �����N����

  // �v���O�����I�u�W�F�N�g�̍쐬
  GLuint program = glCreateProgram();
  
  // �o�[�e�b�N�X�V�F�[�_
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  if (readShaderSource(vertShader, vert)) exit(1);
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(vertShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Error: Could not compile vertex shader source: " << vert << std::endl;
    exit(1);
  }
  glAttachShader(program, vertShader);
  glDeleteShader(vertShader);

  // �t���O�����g�V�F�[�_
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  if (readShaderSource(fragShader, frag)) exit(1);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(fragShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Error: Could not compile fragment shader source: " << frag << std::endl;
    exit(1);
  }
  glAttachShader(program, fragShader);
  glDeleteShader(fragShader);

  // �W�I���g���V�F�[�_�i�I�v�V�����j
  if (geom) {
    GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER_EXT);
    if (readShaderSource(geomShader, geom)) exit(1);
    glCompileShader(geomShader);
    glGetShaderiv(geomShader, GL_COMPILE_STATUS, &compiled);
    printShaderInfoLog(geomShader);
    if (compiled == GL_FALSE) {
      std::cerr << "Error: Could not compile geometry shader source: " << geom << std::endl;
      exit(1);
    }
    glAttachShader(program, geomShader);
    glDeleteShader(geomShader);
  }
  
  // �V�F�[�_�v���O�����̃����N
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  printProgramInfoLog(program);
  if (linked == GL_FALSE) {
    std::cerr << "Error: Could not link shader program" << std::endl;
    exit(1);
  }

  return program;
}

/*
** �e�N�X�`���}�b�s���O�p�� RAW �摜�t�@�C���̓ǂݍ���
*/
void loadImage(const char *name, int width, int height, GLenum format)
{
  // �e�N�X�`���̓ǂݍ��ݐ�
  char *image = 0;
  
  // �e�N�X�`���t�@�C�����J��
  std::ifstream file(name, std::ios::binary);

  if (file.fail()) {
    // �J���Ȃ�����
    std::cerr << "Waring: Can't open texture file: " << name << std::endl;
  }
  else {
    // �t�@�C���̖����Ɉړ������݈ʒu�i���t�@�C���T�C�Y�j�𓾂�
    file.seekg(0L, std::ios::end);
    GLsizei size = file.tellg();

    // �e�N�X�`���T�C�Y���̃��������m�ۂ���
    GLsizei maxsize = width * height * ((format == GL_RGB) ? 3 : 4);
    image = new char[maxsize];

    // �t�@�C����擪����ǂݍ���
    file.seekg(0L, std::ios::beg);
    file.read(image, (size < maxsize) ? size : maxsize);

    if (file.bad()) {
      // ���܂��ǂݍ��߂Ȃ�����
      std::cerr << "Warning: Could not read texture file: " << name << std::endl;
    }
    file.close();
  }
  
  // format �� RGBA �Ȃ� 4 �o�C�g���E�ɐݒ�
  glPixelStorei(GL_UNPACK_ALIGNMENT, (format == GL_RGBA) ? 4 : 1);
  
  // �e�N�X�`�������蓖�Ă�
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
  
  // �o�C���j�A�i�~�b�v�}�b�v�Ȃ��j�C�G�b�W�ŃN�����v
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  // �ǂݍ��݂Ɏg�������������J������
  delete[] image;
}

/*
** �����}�b�v�p�� RAW �摜�t�@�C���̓ǂݍ���Ŗ@���}�b�v���쐬����
*/
void loadHeight(const char *name, int width, int height, float nz)
{
  // �����}�b�v�̓ǂݍ��ݐ�
  GLubyte *hmap = 0;
  
  // �@���}�b�v
  GLfloat (*nmap)[4] = 0;
  
  // �e�N�X�`���t�@�C�����J��
  std::ifstream file(name, std::ios::binary);
  
  if (file.fail()) {
    // �J���Ȃ�����
    std::cerr << "Waring: Can't open texture file: " << name << std::endl;
  }
  else {
    // �t�@�C���̖����Ɉړ������݈ʒu�i���t�@�C���T�C�Y�j�𓾂�
    file.seekg(0L, std::ios::end);
    GLsizei size = file.tellg();
    
    // ���������m�ۂ���
    GLsizei maxsize = width * height;
    hmap = new GLubyte[maxsize];
    nmap = new GLfloat[maxsize][4];
    
    // �t�@�C����擪����ǂݍ���
    file.seekg(0L, std::ios::beg);
    file.read(reinterpret_cast<char *>(hmap), (size < maxsize) ? size : maxsize);
    
    if (file.bad()) {
      // ���܂��ǂݍ��߂Ȃ�����
      std::cerr << "Warning: Could not read texture file: " << name << std::endl;
    }
    file.close();
    
    // �@���}�b�v�̍쐬
    for (int i = 0; i < maxsize; ++i) {
      int x = i % width, y = i - x;
      
      // �אڂ����f�Ƃ̒l�̍���@���x�N�g���̐����ɗp����
      float nx = static_cast<float>(hmap[y + (x + 1) % width] - hmap[i]);
      float ny = static_cast<float>(hmap[(y + width) % maxsize + x] - hmap[i]);
      
      // �@���x�N�g���̒��������߂Ă���
      float nl = sqrt(nx * nx + ny * ny + nz * nz);
      
      nmap[i][0] = nx * 0.5f / nl + 0.5f;
      nmap[i][1] = ny * 0.5f / nl + 0.5f;
      nmap[i][2] = nz * 0.5f / nl + 0.5f;
      nmap[i][3] = hmap[i] * 0.0039215686f;
    }
  }
  
  // nmap �� GLfloat �Ȃ̂� 4 �o�C�g���E�ɐݒ�
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  
  // �e�N�X�`�������蓖�Ă�
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nmap);
  
  // �o�C���j�A�i�~�b�v�}�b�v�Ȃ��j�C�G�b�W�ŃN�����v
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  
  // �ǂݍ��݂Ɏg�������������J������
  delete[] hmap;
  delete[] nmap;
}
