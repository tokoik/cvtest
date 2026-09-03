#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include <opencv2/opencv.hpp>
#include "gg.h"
using namespace gg;

// キャプチャ画像サイズ
#define WIDTH 640
#define HEIGHT 480

// キャプチャフレームレート
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
  cv::VideoCapture capture;
  GLenum format;
  GLsizei width, height;
  GLubyte *texture;
  std::atomic<bool> status{ false };
  std::thread workerThread;
  std::mutex mtx;

public:
  CaptureWorker(int index, int width, int height, int fps)
    : width(width), height(height), format(GL_BGR), texture(new GLubyte[width * height * 4])
  {
    capture.open(index);
    if (!capture.isOpened())
    {
      std::cerr << "Warning: Cannot open camera capture device." << std::endl;
      // カメラが接続されていない場合でもクラッシュせずダミー画像を生成
      memset(texture, 128, width * height * 4);
    }
    else
    {
      capture.set(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
      capture.set(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
      capture.set(cv::CAP_PROP_FPS, static_cast<double>(fps));
    }

    status = true;
    workerThread = std::thread(&CaptureWorker::run, this);
  }

  ~CaptureWorker()
  {
    status = false;
    if (workerThread.joinable())
    {
      workerThread.join();
    }
    if (capture.isOpened())
    {
      capture.release();
    }
    delete[] texture;
  }

  void lock() { mtx.lock(); }
  void unlock() { mtx.unlock(); }

  void run()
  {
    cv::Mat frame;
    while (status)
    {
      if (capture.isOpened() && capture.read(frame) && !frame.empty())
      {
        lock();
        width = frame.cols;
        height = frame.rows;
        if (frame.channels() == 3)
          format = GL_BGR;
        else if (frame.channels() == 4)
          format = GL_BGRA;
        else
          format = GL_LUMINANCE;

        GLsizei size = width * frame.channels();
        for (int y = 0; y < height; ++y)
        {
          memcpy(texture + size * y, frame.ptr(y), size);
        }
        unlock();
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
      }
    }
  }

  void sendTexture()
  {
    lock();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, texture);
    unlock();
  }
};

static CaptureWorker *worker = nullptr;

static void endCapture(void)
{
  delete worker;
  worker = nullptr;
}

static void cvInit(void)
{
  worker = new CaptureWorker(0, WIDTH, HEIGHT, FPS);
  atexit(endCapture);
}

static GLuint makeSphere(float radius, int slices, int stacks, GLfloat sScale, GLfloat tScale)
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  int vertices = (slices + 1) * (stacks + 1);
  int faces = slices * stacks * 2;
  GLuint vbo[2];
  glGenBuffers(2, vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * vertices, 0, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * faces, 0, GL_STATIC_DRAW);
  GLfloat(*vertex)[8] = reinterpret_cast<GLfloat(*)[8]>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
  GLuint(*face)[3] = reinterpret_cast<GLuint(*)[3]>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

  for (int k = 0, j = 0; j <= stacks; ++j)
  {
    GLfloat t = static_cast<GLfloat>(j) / static_cast<GLfloat>(stacks);
    GLfloat ph = 3.141593f * t;
    GLfloat y = cosf(ph);
    GLfloat r = sinf(ph);

    for (int i = 0; i <= slices; ++i)
    {
      GLfloat s = static_cast<GLfloat>(i) / static_cast<GLfloat>(slices);
      GLfloat th = 2.0f * 3.141593f * s;
      GLfloat x = r * cosf(th);
      GLfloat z = r * sinf(th);

      vertex[k][0] = x * radius;
      vertex[k][1] = y * radius;
      vertex[k][2] = z * radius;

      vertex[k][3] = x;
      vertex[k][4] = y;
      vertex[k][5] = z;

      vertex[k][6] = sScale - s * sScale;
      vertex[k][7] = t * tScale;

      ++k;
    }
  }

  for (int k = 0, j = 0; j < stacks; ++j)
  {
    for (int i = 0; i < slices; ++i)
    {
      int count = (slices + 1) * j + i;

      face[k][0] = count;
      face[k][1] = count + 1;
      face[k][2] = count + slices + 2;
      ++k;

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
  glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 8, reinterpret_cast<GLfloat *>(0));
  glNormalPointer(GL_FLOAT, sizeof(GLfloat) * 8, reinterpret_cast<GLfloat *>(0) + 3);
  glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 8, reinterpret_cast<GLfloat *>(0) + 6);

  return vao;
}

static void glInit(void)
{
  ggInit();

  progname = ggLoadShader("simple.vert", "simple.frag");
  dmapLoc = glGetUniformLocation(progname, "dmap");
  sizeLoc = glGetUniformLocation(progname, "size");

  vaname = makeSphere(1.0f, SLICES, STACKS, (GLfloat)WIDTH / (GLfloat)TEXWIDTH, (GLfloat)HEIGHT / (GLfloat)TEXHEIGHT);

  glGenTextures(1, &texname);
  glBindTexture(GL_TEXTURE_2D, texname);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  

  glEnable(GL_DEPTH_TEST);
  glClearColor(1.0, 1.0, 1.0, 1.0);
}

static void display(void)
{
  static int firstTime = 0;
  GLdouble t;
  if (firstTime == 0) { firstTime = glutGet(GLUT_ELAPSED_TIME); t = 0.0; }
  else t = (GLdouble)((glutGet(GLUT_ELAPSED_TIME) - firstTime) % CYCLE) / (GLdouble)CYCLE;

  if (worker) worker->sendTexture();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glBindTexture(GL_TEXTURE_2D, texname);
  glBindVertexArray(vaname);
  glUseProgram(progname);
  glUniform1i(dmapLoc, 0);
  glUniform2f(sizeLoc, TEXWIDTH, TEXHEIGHT);
  glPushMatrix();
  glRotated(t * 360.0, 0.0, 1.0, 0.0);
  glDrawElements(GL_TRIANGLES, SLICES * STACKS * 2 * 3, GL_UNSIGNED_INT, 0);
  glPopMatrix();

  glutSwapBuffers();
}

static void resize(int w, int h)
{
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 1.0, 5.0);

  glMatrixMode(GL_MODELVIEW);
}

static void idle(void)
{
  glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 'q':
  case 'Q':
  case '':
    exit(0);
  default:
    break;
  }
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("cvtest");
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);

  cvInit();
  glInit();

  glutMainLoop();

  return 0;
}
