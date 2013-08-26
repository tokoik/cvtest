#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif
#pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_highgui" CV_VERSION_STR CV_EXT_STR)

#include "gg.h"
using namespace gg;

// 画像サイズ
#define WIDTH 640
#define HEIGHT 480

// テクスチャサイズ
#define TEXWIDTH 1024
#define TEXHEIGHT 512

// キャプチャ用
static CvCapture *capture = 0;

// 球の分割数
#define SLICES 64
#define STACKS 32

// アニメーションの周期
#define CYCLE 10000

// 頂点配列オブジェクト
static GLuint vaname;

// テクスチャオブジェクト
static GLuint texname;

// プログラムオブジェクト
static GLuint progname;

// テクスチャユニット
static GLint dmapLoc;

// テクスチャサイズ
static GLint sizeLoc;

/*
 * テクスチャ作成
 */
static void getTexture(void)
{
  if (cvGrabFrame(capture))
  {
    // キャプチャ映像から画像の切り出し
    IplImage *image = cvRetrieveFrame(capture);
   
    // 切り出した画像の種類の判別
    GLenum format;
    if (image->nChannels == 3)
      format = GL_BGR;
    else if (image->nChannels == 4)
      format = GL_BGRA;
    else
      format = GL_LUMINANCE;

    // テクスチャメモリへの転送
    glBindTexture(GL_TEXTURE_2D, texname);
    for (int y = 0; y < image->height; ++y)
    {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, image->width, 1, format,
                      GL_UNSIGNED_BYTE, image->imageData + image->widthStep * y);
    }
  }
}

/*
 * プログラム終了時の処理
 */
static void releaseCapture(void)
{
  // image の release
  cvReleaseCapture(&capture);
}

/*
 * OpenCV の初期化
 */
static void cvInit(void)
{
  // カメラの初期化
  capture = cvCreateCameraCapture(CV_CAP_ANY);
  if (capture == 0)
  {
    std::cerr << "cannot capture image" << std::endl;
    exit(1);
  }
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30.0);
  
  // プログラム終了時に capture を release する
  atexit(releaseCapture);
}

/*
 * 球の頂点属性
 */
static GLfloat (*pv)[3] = 0;
static GLfloat (*nv)[3] = 0;
static GLfloat (*tv)[2] = 0;
static GLuint (*face)[3] = 0;
static GLsizei faces = 0;

/*
 * 球のデータ用のメモリの解法
 */
static void releaseMemory(void)
{
  delete[] pv;
  delete[] nv;
  delete[] tv;
  delete[] face;
}

/*
 * 球のデータを作成する
 */
static void makeSphere(float radius, int slices, int stacks)
{
  // テクスチャ座標のスケール
  static const GLfloat sScale = (GLfloat)WIDTH / (GLfloat)(TEXWIDTH);
  static const GLfloat tScale = (GLfloat)HEIGHT / (GLfloat)(TEXHEIGHT);
  
  // メモリの解放
  releaseMemory();
  
  // 頂点用メモリの確保
  int vertices = (slices + 1) * (stacks + 1);
  pv = new GLfloat[vertices][3];
  nv = new GLfloat[vertices][3];
  tv = new GLfloat[vertices][2];
  
  // インデックス用メモリの確保
  faces = slices * stacks * 2;
  face = new GLuint[faces][3];
  
  // 頂点の位置とテクスチャ座標を求める
  for (int k = 0, j = 0; j <= stacks; ++j)
  {
    float t = (float)j / (float)stacks;
    float ph = 3.141593f * t;
    float y = cosf(ph);
    float r = sinf(ph);
    
    for (int i = 0; i <= slices; ++i)
    {
      float s = (float)i / (float)slices;
      float th = 2.0f * 3.141593f * s;
      float x = r * cosf(th);
      float z = r * sinf(th);
      
      // 頂点の座標値
      pv[k][0] = x * radius;
      pv[k][1] = y * radius;
      pv[k][2] = z * radius;
      
      // 頂点の法線ベクトル
      nv[k][0] = x;
      nv[k][1] = y;
      nv[k][2] = z;
      
      // 頂点のテクスチャ座標値
      tv[k][0] = s * sScale;
      tv[k][1] = t * tScale;
      
      ++k;
    }
  }
  
  // 面の指標を求める
  for (int k = 0, j = 0; j < stacks; ++j)
  {
    for (int i = 0; i < slices; ++i)
    {
      int count = (slices + 1) * j + i;
      
      /* 上半分 */
      face[k][0] = count;
      face[k][1] = count + 1;
      face[k][2] = count + slices + 2;
      ++k;
      
      /* 下半分 */
      face[k][0] = count;
      face[k][1] = count + slices + 2;
      face[k][2] = count + slices + 1;
      ++k;
    }
  }
  
  // 頂点配列オブジェクトの設定
  glGenVertexArrays(1, &vaname);
  glBindVertexArray(vaname);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, pv);
  glNormalPointer(GL_FLOAT, 0, nv);
  glTexCoordPointer(2, GL_FLOAT, 0, tv);
  
  // プログラム終了時にメモリを解放する
  atexit(releaseMemory);
}

/*
 * OpenGL の初期化
 */
static void glInit(void)
{
  // ゲームグラフィックス特論の都合にもとづく初期化
  ggInit();
  
  // シェーダプログラムの読み込み
  progname = ggLoadShader("simple.vert", "simple.frag");
  
  // テクスチャユニット用の uniform 変数
  dmapLoc = glGetUniformLocation(progname, "dmap");
  
  // テクスチャサイズの uniform 変数
  sizeLoc = glGetUniformLocation(progname, "size");
  
  // 図形データの作成
  makeSphere(1.0f, SLICES, STACKS);
  
  // テクスチャメモリの確保
  glGenTextures(1, &texname);
  glBindTexture(GL_TEXTURE_2D, texname);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  // テクスチャはそのまま表示する（ポリゴン色を無視する）
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  
  
  // 隠面消去
  glEnable(GL_DEPTH_TEST);
  
  // 背景色
  glClearColor(1.0, 1.0, 1.0, 1.0);
}

/*
 * 画面表示
 */
static void display(void)
{
  // 時刻の計測
  static int firstTime = 0;
  GLdouble t;
  if (firstTime == 0) { firstTime = glutGet(GLUT_ELAPSED_TIME); t = 0.0; }
  else t = (GLdouble)((glutGet(GLUT_ELAPSED_TIME) - firstTime) % CYCLE) / (GLdouble)CYCLE;

  // テクスチャ作成
  getTexture();
  
  // 画面クリア
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // モデルビュー変換行列の設定
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  
  // 図形描画
  glBindTexture(GL_TEXTURE_2D, texname);
  glBindVertexArray(vaname);
  glUseProgram(progname);
  glUniform1i(dmapLoc, 0);
  glUniform2f(sizeLoc, TEXWIDTH, TEXHEIGHT);
  glPushMatrix();
  glRotated(t * 360.0, 0.0, 1.0, 0.0);
  glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_INT, face);
  glPopMatrix();
  
  // ダブルバッファリング
  glutSwapBuffers();
}

/*
 * ウィンドウのリサイズ
 */
static void resize(int w, int h)
{
  // ウィンドウ全体を表示領域にする
  glViewport(0, 0, w, h);
  
  // 投影変換行列を設定する
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 1.0, 5.0);
  
  // モデルビュー変換行列に切り替える
  glMatrixMode(GL_MODELVIEW);
}

/*
 ** アニメーション
 */
static void idle(void)
{
  glutPostRedisplay();
}

/*
 * キー操作の処理
 */
static void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 'q':
    case 'Q':
    case '\033':  // '\033' は ESC の ASCII コード
      exit(0);
    default:
      break;
  }
}

/*
 * メインプログラム
 */
int main(int argc, char *argv[])
{
  // OpenCV の初期化
  cvInit();
  
  // GLUT の初期化
  glutInit(&argc, argv);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("cvtest");
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  
  // OpenGL の初期化
  glInit();
  
  // メインループ
  glutMainLoop();
  
  return 0;
}
