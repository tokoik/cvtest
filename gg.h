#ifndef GG_H
#define GG_H

#if defined(WIN32)
#  pragma warning(disable:4996)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  pragma comment(lib, "glew32.lib")
#  include "glew.h"
#  include "wglew.h"
#  include "glut.h"
#elif defined(X11)
#  define GL_GLEXT_PROTOTYPES
#  define GLX_GLXEXT_PROTOTYPES
#  include <GL/glx.h>
#  include <GL/glut.h>
#elif defined(__APPLE__)
#  include <OpenGL/OpenGL.h>
#  include <GLUT/glut.h>
#  define glGenVertexArrays glGenVertexArraysAPPLE
#  define glBindVertexArray glBindVertexArrayAPPLE
#  define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#  define glIsVertexArray glIsVertexArrayAPPLE
#  define GL_RGB16F GL_RGB16F_ARB
#  define GL_RGB32F GL_RGB32F_ARB
#else
#  error "This platform is not supported."
#endif

/*
** �Q�[���O���t�B�b�N�X���_�̓s���ɂ��ƂÂ�������
*/
extern void ggInit(void);

/*
** �V�F�[�_�[�\�[�X�t�@�C���̓ǂݍ���
*/
extern GLuint loadShader(const char *vert, const char *frag, const char *geom);

/*
** �e�N�X�`���}�b�s���O�p�� RAW �摜�t�@�C���̓ǂݍ���
*/
extern void loadImage(const char *name, int width, int height, GLenum format);

/*
** �����}�b�v�p�� RAW �摜�t�@�C���̓ǂݍ���Ŗ@���}�b�v���쐬����
*/
extern void loadHeight(const char *name, int width, int height, float nz);

#endif
