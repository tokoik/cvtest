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
** ゲームグラフィックス特論の都合にもとづく初期化
*/
extern void ggInit(void);

/*
** シェーダーソースファイルの読み込み
*/
extern GLuint loadShader(const char *vert, const char *frag, const char *geom);

/*
** テクスチャマッピング用の RAW 画像ファイルの読み込み
*/
extern void loadImage(const char *name, int width, int height, GLenum format);

/*
** 高さマップ用の RAW 画像ファイルの読み込んで法線マップを作成する
*/
extern void loadHeight(const char *name, int width, int height, float nz);

#endif
