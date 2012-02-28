#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include "gg.h"

/*
** ゲームグラフィックス特論の都合にもとづく初期化
*/
void ggInit(void)
{
  // Swap Interval の設定
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
  
  // バージョンチェック
  if (atof(reinterpret_cast<const char *>(glGetString(GL_VERSION))) < 2.1) {
    std::cerr << "Error: This program requires OpenGL 2.1 or lator." << std::endl;
    exit(1);
  }
}

/*
** シェーダーのソースプログラムをメモリに読み込む
*/
static bool readShaderSource(GLuint shader, const char *name)
{
  bool ret = true; // 終了ステータス（true ならエラー）
  
  // ソースファイルを開く
  std::ifstream file(name, std::ios::binary);

  if (file.fail()) {
    // 開けなかった
    std::cerr << "Error: Can't open source file: " << name << std::endl;
  }
  else {
    // ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
    file.seekg(0L, std::ios::end);
    GLsizei length = file.tellg();
    
    // ファイルサイズのメモリを確保
    char *buffer = new GLchar[length];
    
    // ファイルを先頭から読み込む
    file.seekg(0L, std::ios::beg);
    file.read(buffer, length);

    if (file.bad()) {
      // うまく読み込めなかった
      std::cerr << "Error: Could not read souce file: " << name << std::endl;
    }
    else {
      // シェーダのソースプログラムのシェーダオブジェクトへの読み込み
      const GLchar *source[] = { buffer };
      glShaderSource(shader, 1, source, &length);
      ret = false;
    }
    file.close();
    
    // 読み込みに使ったメモリを開放する
    delete[] buffer;
  }
  
  return ret;
}

/*
** シェーダの情報を表示する
*/
static void printShaderInfoLog(GLuint shader)
{
  // シェーダのコンパイル時のログの長さを取得する
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1) {
    // シェーダのコンパイル時のログの内容を取得する
    GLchar *infoLog = new GLchar[bufSize];
    GLsizei length;
    glGetShaderInfoLog(shader, bufSize, &length, infoLog);
    std::cerr << "InfoLog:\n" << infoLog << '\n' << std::endl;
    delete[] infoLog;
  }
}

/*
** プログラムの情報を表示する
*/
static void printProgramInfoLog(GLuint program)
{
  // シェーダのリンク時のログの長さを取得する
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1) {
    // シェーダのリンク時のログの内容を取得する
    GLchar *infoLog = new GLchar[bufSize];
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, infoLog);
    std::cerr << "InfoLog:\n" << infoLog << '\n' << std::endl;
    delete[] infoLog;
  }
}

/*
** シェーダーソースファイルの読み込み
*/
GLuint loadShader(const char *vert, const char *frag, const char *geom)
{
  GLint compiled, linked; // コンパイル結果, リンク結果

  // プログラムオブジェクトの作成
  GLuint program = glCreateProgram();
  
  // バーテックスシェーダ
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

  // フラグメントシェーダ
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

  // ジオメトリシェーダ（オプション）
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
  
  // シェーダプログラムのリンク
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
** テクスチャマッピング用の RAW 画像ファイルの読み込み
*/
void loadImage(const char *name, int width, int height, GLenum format)
{
  // テクスチャの読み込み先
  char *image = 0;
  
  // テクスチャファイルを開く
  std::ifstream file(name, std::ios::binary);

  if (file.fail()) {
    // 開けなかった
    std::cerr << "Waring: Can't open texture file: " << name << std::endl;
  }
  else {
    // ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
    file.seekg(0L, std::ios::end);
    GLsizei size = file.tellg();

    // テクスチャサイズ分のメモリを確保する
    GLsizei maxsize = width * height * ((format == GL_RGB) ? 3 : 4);
    image = new char[maxsize];

    // ファイルを先頭から読み込む
    file.seekg(0L, std::ios::beg);
    file.read(image, (size < maxsize) ? size : maxsize);

    if (file.bad()) {
      // うまく読み込めなかった
      std::cerr << "Warning: Could not read texture file: " << name << std::endl;
    }
    file.close();
  }
  
  // format が RGBA なら 4 バイト境界に設定
  glPixelStorei(GL_UNPACK_ALIGNMENT, (format == GL_RGBA) ? 4 : 1);
  
  // テクスチャを割り当てる
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
  
  // バイリニア（ミップマップなし），エッジでクランプ
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  // 読み込みに使ったメモリを開放する
  delete[] image;
}

/*
** 高さマップ用の RAW 画像ファイルの読み込んで法線マップを作成する
*/
void loadHeight(const char *name, int width, int height, float nz)
{
  // 高さマップの読み込み先
  GLubyte *hmap = 0;
  
  // 法線マップ
  GLfloat (*nmap)[4] = 0;
  
  // テクスチャファイルを開く
  std::ifstream file(name, std::ios::binary);
  
  if (file.fail()) {
    // 開けなかった
    std::cerr << "Waring: Can't open texture file: " << name << std::endl;
  }
  else {
    // ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
    file.seekg(0L, std::ios::end);
    GLsizei size = file.tellg();
    
    // メモリを確保する
    GLsizei maxsize = width * height;
    hmap = new GLubyte[maxsize];
    nmap = new GLfloat[maxsize][4];
    
    // ファイルを先頭から読み込む
    file.seekg(0L, std::ios::beg);
    file.read(reinterpret_cast<char *>(hmap), (size < maxsize) ? size : maxsize);
    
    if (file.bad()) {
      // うまく読み込めなかった
      std::cerr << "Warning: Could not read texture file: " << name << std::endl;
    }
    file.close();
    
    // 法線マップの作成
    for (int i = 0; i < maxsize; ++i) {
      int x = i % width, y = i - x;
      
      // 隣接する画素との値の差を法線ベクトルの成分に用いる
      float nx = static_cast<float>(hmap[y + (x + 1) % width] - hmap[i]);
      float ny = static_cast<float>(hmap[(y + width) % maxsize + x] - hmap[i]);
      
      // 法線ベクトルの長さを求めておく
      float nl = sqrt(nx * nx + ny * ny + nz * nz);
      
      nmap[i][0] = nx * 0.5f / nl + 0.5f;
      nmap[i][1] = ny * 0.5f / nl + 0.5f;
      nmap[i][2] = nz * 0.5f / nl + 0.5f;
      nmap[i][3] = hmap[i] * 0.0039215686f;
    }
  }
  
  // nmap が GLfloat なので 4 バイト境界に設定
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  
  // テクスチャを割り当てる
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nmap);
  
  // バイリニア（ミップマップなし），エッジでクランプ
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  
  // 読み込みに使ったメモリを開放する
  delete[] hmap;
  delete[] nmap;
}
