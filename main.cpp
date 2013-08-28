#include <iostream>
#include <cstring>

#include <opencv2/highgui/highgui.hpp>

#ifdef _WIN32
#  include <Windows.h>
#  define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#  ifdef _DEBUG
#    define CV_EXT_STR "d.lib"
#  else
#    define CV_EXT_STR ".lib"
#  endif
#  pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_highgui" CV_VERSION_STR CV_EXT_STR)
#else
#  include <pthread.h>
#  include <unistd.h>
#endif

#include "gg.h"
using namespace gg;

// キャプチャする画像サイズ
#define WIDTH 640
#define HEIGHT 480

// キャプチャするフレームレート
#define FPS 30

// テクスチャサイズ
#define TEXWIDTH 1024
#define TEXHEIGHT 512

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

// キャプチャ用スレッド
class CaptureWorker
{
  // キャプチャ
  CvCapture *capture;

  // テクスチャ
  GLenum format;
  GLsizei width, height;
  GLubyte *texture;

  // 実行状態
  bool status;

  // スレッドとミューテックス
#ifdef _WIN32
  HANDLE thread;
  HANDLE mutex;
#else
  pthread_t thread;
  pthread_mutex_t mutex;
#endif

public:

  // コンストラクタ
  CaptureWorker(int index, int width, int height, int fps)
  {
    // カメラを初期化する
    capture = cvCreateCameraCapture(index);
    if (capture == 0)
    {
      std::cerr << "cannot capture image" << std::endl;
      exit(1);
    }
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
    cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, static_cast<double>(fps));

    // テクスチャ用のメモリを確保する
    texture = new GLubyte[width * height * 4];

    // パラメータに初期値を与えておく
    this->width = width;
    this->height = height;
    this->format = GL_BGRA;

    // スレッドとミューテックスを生成する
#ifdef _WIN32
    mutex = CreateMutex(NULL, TRUE, NULL);
    thread = CreateThread(NULL, 0, start, (LPVOID)this, 0, NULL);
#else
    pthread_mutex_init(&mutex, 0);
    pthread_create(&thread, 0, start, this);
#endif

    // スレッドが実行状態であることを記録する
    status = true;
  }

  // デストラクタ
  ~CaptureWorker()
  {
    // スレッドを停止する
    stop();

#ifdef _WIN32
    // スレッドの停止を待つ
    WaitForSingleObject(thread, 0); 
    CloseHandle(thread);

    // ミューテックスを破棄する
    CloseHandle(mutex);
#else
    // スレッドの停止を待つ
    pthread_join(thread, 0);

    // ミューテックスを破棄する
    pthread_mutex_destroy(&mutex);
#endif

    // image の release
    cvReleaseCapture(&capture);

    // メモリの解放
    delete[] texture;
  }

  // mutex のロック
  void lock(void)
  {
#ifdef _WIN32
    WaitForSingleObject(mutex, 0); 
#else
    pthread_mutex_lock(&mutex);
#endif
  }

  // mutex のリリース
  void unlock(void)
  {
#ifdef _WIN32
    ReleaseMutex(mutex);
#else
    pthread_mutex_unlock(&mutex);
#endif
  }

  // スレッドの開始
#ifdef _WIN32
  static DWORD WINAPI start(LPVOID arg)
#else
  static void *start(void *arg)
#endif
  {
    return reinterpret_cast<CaptureWorker *>(arg)->getTexture();
  }

  // スレッドの停止
  void stop(void)
  {
    lock();
    status = false;
    unlock();
  }

  // スレッドの停止判定
  bool check(void)
  {
    bool ret;

    lock();
    ret = status;
    unlock();

    return ret;
  }

  // テクスチャ作成
#ifdef _WIN32
  DWORD WINAPI getTexture(void)
#else
  void *getTexture(void)
#endif
  {
    for (;;)
    {
      // 終了条件のテスト
      if (!check()) break;

      if (cvGrabFrame(capture))
      {
        // キャプチャ映像から画像を切り出す
        IplImage *image = cvRetrieveFrame(capture);

        if (image)
        {
          // 切り出した画像の種類の判別
          width = image->width;
          height = image->height;
          if (image->nChannels == 3)
            format = GL_BGR;
          else if (image->nChannels == 4)
            format = GL_BGRA;
          else
            format = GL_LUMINANCE;

          // テクスチャメモリへの転送
          GLsizei size = image->width * image->nChannels;
          lock();
          for (int y = 0; y < image->height; ++y)
            memcpy(texture + size * y, image->imageData + image->widthStep * y, size);
          unlock();
        }
        else
        {
          // １フレーム分待つ
#ifdef _WIN32
          Sleep(1000 / FPS);
#else
          usleep(1000000 / FPS);
#endif
        }
      }
    }

    return 0;
  }

  // テクスチャ転送
  void sendTexture(void)
  {
    lock();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, texture);
    unlock();
  }
};
static CaptureWorker *worker = 0;

//
// キャプチャ終了時の処理
//
static void endCapture(void)
{
  delete worker;
}

//
// OpenCV の初期化
//
static void cvInit(void)
{
  // スレッドを生成する
  worker = new CaptureWorker(CV_CAP_ANY, WIDTH, HEIGHT, FPS);

  // キャプチャ終了時の処理を予約する
  atexit(endCapture);
}

//
// 球のデータを作成する
//
static GLuint makeSphere(float radius, int slices, int stacks, GLfloat sScale, GLfloat tScale)
{
  // 頂点配列オブジェクトの設定
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // 頂点バッファオブジェクトの設定
  int vertices = (slices + 1) * (stacks + 1);
  int faces = slices * stacks * 2;
  GLuint vbo[2];
  glGenBuffers(2, vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 8 * vertices, 0, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint) * 3 * faces, 0, GL_STATIC_DRAW);
  GLfloat (*vertex)[8] = reinterpret_cast<GLfloat (*)[8]>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
  GLuint (*face)[3] = reinterpret_cast<GLuint (*)[3]>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

  // 頂点の位置とテクスチャ座標を求める
  for (int k = 0, j = 0; j <= stacks; ++j)
  {
    GLfloat t = static_cast<GLfloat>(j) / static_cast<GLfloat>(stacks);
    GLfloat ph = 3.141593f * t;
    GLfloat y = cos(ph);
    GLfloat r = sin(ph);

    for (int i = 0; i <= slices; ++i)
    {
      GLfloat s = static_cast<GLfloat>(i) / static_cast<GLfloat>(slices);
      GLfloat th = 2.0f * 3.141593f * s;
      GLfloat x = r * cosf(th);
      GLfloat z = r * sinf(th);

      // 頂点の座標値
      vertex[k][0] = x * radius;
      vertex[k][1] = y * radius;
      vertex[k][2] = z * radius;

      // 頂点の法線ベクトル
      vertex[k][3] = x;
      vertex[k][4] = y;
      vertex[k][5] = z;

      // 頂点のテクスチャ座標値
      vertex[k][6] = sScale - s * sScale;
      vertex[k][7] = t * tScale;

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

  glUnmapBuffer(GL_ARRAY_BUFFER);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof (GLfloat) * 8, reinterpret_cast<GLfloat *>(0));
  glNormalPointer(GL_FLOAT, sizeof (GLfloat) * 8, reinterpret_cast<GLfloat *>(0) + 3);
  glTexCoordPointer(2, GL_FLOAT, sizeof (GLfloat) * 8, reinterpret_cast<GLfloat *>(0) + 6);

  return vao;
}

//
// OpenGL の初期化
//
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
  vaname = makeSphere(1.0f, SLICES, STACKS, (GLfloat)WIDTH / (GLfloat)TEXWIDTH, (GLfloat)HEIGHT / (GLfloat)TEXHEIGHT);

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

//
// 画面表示
//
static void display(void)
{
  // 時刻の計測
  static int firstTime = 0;
  GLdouble t;
  if (firstTime == 0) { firstTime = glutGet(GLUT_ELAPSED_TIME); t = 0.0; }
  else t = (GLdouble)((glutGet(GLUT_ELAPSED_TIME) - firstTime) % CYCLE) / (GLdouble)CYCLE;

  // テクスチャ作成
  worker->sendTexture();

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
  glDrawElements(GL_TRIANGLES, SLICES * STACKS * 2 * 3, GL_UNSIGNED_INT, 0);
  glPopMatrix();

  // ダブルバッファリング
  glutSwapBuffers();
}

//
// ウィンドウのリサイズ
//
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

//
// アニメーション
//
static void idle(void)
{
  glutPostRedisplay();
}

//
// キー操作の処理
//
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

//
// メインプログラム
//
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
