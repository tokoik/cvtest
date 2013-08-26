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

// �摜�T�C�Y
#define WIDTH 640
#define HEIGHT 480

// �e�N�X�`���T�C�Y
#define TEXWIDTH 1024
#define TEXHEIGHT 512

// �L���v�`���p
static CvCapture *capture = 0;

// ���̕�����
#define SLICES 64
#define STACKS 32

// �A�j���[�V�����̎���
#define CYCLE 10000

// ���_�z��I�u�W�F�N�g
static GLuint vaname;

// �e�N�X�`���I�u�W�F�N�g
static GLuint texname;

// �v���O�����I�u�W�F�N�g
static GLuint progname;

// �e�N�X�`�����j�b�g
static GLint dmapLoc;

// �e�N�X�`���T�C�Y
static GLint sizeLoc;

/*
 * �e�N�X�`���쐬
 */
static void getTexture(void)
{
  if (cvGrabFrame(capture))
  {
    // �L���v�`���f������摜�̐؂�o��
    IplImage *image = cvRetrieveFrame(capture);
   
    // �؂�o�����摜�̎�ނ̔���
    GLenum format;
    if (image->nChannels == 3)
      format = GL_BGR;
    else if (image->nChannels == 4)
      format = GL_BGRA;
    else
      format = GL_LUMINANCE;

    // �e�N�X�`���������ւ̓]��
    glBindTexture(GL_TEXTURE_2D, texname);
    for (int y = 0; y < image->height; ++y)
    {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, image->width, 1, format,
                      GL_UNSIGNED_BYTE, image->imageData + image->widthStep * y);
    }
  }
}

/*
 * �v���O�����I�����̏���
 */
static void releaseCapture(void)
{
  // image �� release
  cvReleaseCapture(&capture);
}

/*
 * OpenCV �̏�����
 */
static void cvInit(void)
{
  // �J�����̏�����
  capture = cvCreateCameraCapture(CV_CAP_ANY);
  if (capture == 0)
  {
    std::cerr << "cannot capture image" << std::endl;
    exit(1);
  }
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30.0);
  
  // �v���O�����I������ capture �� release ����
  atexit(releaseCapture);
}

/*
 * ���̒��_����
 */
static GLfloat (*pv)[3] = 0;
static GLfloat (*nv)[3] = 0;
static GLfloat (*tv)[2] = 0;
static GLuint (*face)[3] = 0;
static GLsizei faces = 0;

/*
 * ���̃f�[�^�p�̃������̉�@
 */
static void releaseMemory(void)
{
  delete[] pv;
  delete[] nv;
  delete[] tv;
  delete[] face;
}

/*
 * ���̃f�[�^���쐬����
 */
static void makeSphere(float radius, int slices, int stacks)
{
  // �e�N�X�`�����W�̃X�P�[��
  static const GLfloat sScale = (GLfloat)WIDTH / (GLfloat)(TEXWIDTH);
  static const GLfloat tScale = (GLfloat)HEIGHT / (GLfloat)(TEXHEIGHT);
  
  // �������̉��
  releaseMemory();
  
  // ���_�p�������̊m��
  int vertices = (slices + 1) * (stacks + 1);
  pv = new GLfloat[vertices][3];
  nv = new GLfloat[vertices][3];
  tv = new GLfloat[vertices][2];
  
  // �C���f�b�N�X�p�������̊m��
  faces = slices * stacks * 2;
  face = new GLuint[faces][3];
  
  // ���_�̈ʒu�ƃe�N�X�`�����W�����߂�
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
      
      // ���_�̍��W�l
      pv[k][0] = x * radius;
      pv[k][1] = y * radius;
      pv[k][2] = z * radius;
      
      // ���_�̖@���x�N�g��
      nv[k][0] = x;
      nv[k][1] = y;
      nv[k][2] = z;
      
      // ���_�̃e�N�X�`�����W�l
      tv[k][0] = s * sScale;
      tv[k][1] = t * tScale;
      
      ++k;
    }
  }
  
  // �ʂ̎w�W�����߂�
  for (int k = 0, j = 0; j < stacks; ++j)
  {
    for (int i = 0; i < slices; ++i)
    {
      int count = (slices + 1) * j + i;
      
      /* �㔼�� */
      face[k][0] = count;
      face[k][1] = count + 1;
      face[k][2] = count + slices + 2;
      ++k;
      
      /* ������ */
      face[k][0] = count;
      face[k][1] = count + slices + 2;
      face[k][2] = count + slices + 1;
      ++k;
    }
  }
  
  // ���_�z��I�u�W�F�N�g�̐ݒ�
  glGenVertexArrays(1, &vaname);
  glBindVertexArray(vaname);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, pv);
  glNormalPointer(GL_FLOAT, 0, nv);
  glTexCoordPointer(2, GL_FLOAT, 0, tv);
  
  // �v���O�����I�����Ƀ��������������
  atexit(releaseMemory);
}

/*
 * OpenGL �̏�����
 */
static void glInit(void)
{
  // �Q�[���O���t�B�b�N�X���_�̓s���ɂ��ƂÂ�������
  ggInit();
  
  // �V�F�[�_�v���O�����̓ǂݍ���
  progname = ggLoadShader("simple.vert", "simple.frag");
  
  // �e�N�X�`�����j�b�g�p�� uniform �ϐ�
  dmapLoc = glGetUniformLocation(progname, "dmap");
  
  // �e�N�X�`���T�C�Y�� uniform �ϐ�
  sizeLoc = glGetUniformLocation(progname, "size");
  
  // �}�`�f�[�^�̍쐬
  makeSphere(1.0f, SLICES, STACKS);
  
  // �e�N�X�`���������̊m��
  glGenTextures(1, &texname);
  glBindTexture(GL_TEXTURE_2D, texname);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  // �e�N�X�`���͂��̂܂ܕ\������i�|���S���F�𖳎�����j
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  
  
  // �B�ʏ���
  glEnable(GL_DEPTH_TEST);
  
  // �w�i�F
  glClearColor(1.0, 1.0, 1.0, 1.0);
}

/*
 * ��ʕ\��
 */
static void display(void)
{
  // �����̌v��
  static int firstTime = 0;
  GLdouble t;
  if (firstTime == 0) { firstTime = glutGet(GLUT_ELAPSED_TIME); t = 0.0; }
  else t = (GLdouble)((glutGet(GLUT_ELAPSED_TIME) - firstTime) % CYCLE) / (GLdouble)CYCLE;

  // �e�N�X�`���쐬
  getTexture();
  
  // ��ʃN���A
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // ���f���r���[�ϊ��s��̐ݒ�
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  
  // �}�`�`��
  glBindTexture(GL_TEXTURE_2D, texname);
  glBindVertexArray(vaname);
  glUseProgram(progname);
  glUniform1i(dmapLoc, 0);
  glUniform2f(sizeLoc, TEXWIDTH, TEXHEIGHT);
  glPushMatrix();
  glRotated(t * 360.0, 0.0, 1.0, 0.0);
  glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_INT, face);
  glPopMatrix();
  
  // �_�u���o�b�t�@�����O
  glutSwapBuffers();
}

/*
 * �E�B���h�E�̃��T�C�Y
 */
static void resize(int w, int h)
{
  // �E�B���h�E�S�̂�\���̈�ɂ���
  glViewport(0, 0, w, h);
  
  // ���e�ϊ��s���ݒ肷��
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 1.0, 5.0);
  
  // ���f���r���[�ϊ��s��ɐ؂�ւ���
  glMatrixMode(GL_MODELVIEW);
}

/*
 ** �A�j���[�V����
 */
static void idle(void)
{
  glutPostRedisplay();
}

/*
 * �L�[����̏���
 */
static void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 'q':
    case 'Q':
    case '\033':  // '\033' �� ESC �� ASCII �R�[�h
      exit(0);
    default:
      break;
  }
}

/*
 * ���C���v���O����
 */
int main(int argc, char *argv[])
{
  // OpenCV �̏�����
  cvInit();
  
  // GLUT �̏�����
  glutInit(&argc, argv);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow("cvtest");
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  
  // OpenGL �̏�����
  glInit();
  
  // ���C�����[�v
  glutMainLoop();
  
  return 0;
}
