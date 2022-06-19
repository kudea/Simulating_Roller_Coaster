/*
CSCI 420 Computer Graphics, USC
Assignment 2: Roller Coaster
C++ starter code

Student username: <9566797656 Hsuan Yeh>
*/

// Note: You should combine this file 
// with the solution of homework 1.

// Note for Windows/MS Visual Studio:
// You should set argv[1] to track.txt.
// To do this, on the "Solution Explorer",
// right click your project, choose "Properties",
// go to "Configuration Properties", click "Debug",
// then type your track file name for the "Command Arguments".
// You can also repeat this process for the "Release" configuration.


#include "basicPipelineProgram.h"
#include "texturePipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
  #pragma comment(lib, "glew32d.lib")
#else
  #pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };// rotate from x,y,z axis
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
//char windowTitle[512] = "CSCI 420 homework 2 milestone";
char windowTitle[512] = "CSCI 420 homework 2";


int k=0;
double tmax = 0.0;
double tmin = 0.0;
double aa,ab,ac, ba,bb,bc = 0.0; //find point position

OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;
TexturePipelineProgram *texturePipelineProgram;

// represents one control point along the spline
struct Point
{
double x;
double y;
double z;
};
//for Catmull-Rom Spline Matrix
const double s = 0.5;
const double basis[16] = {    -s, 2.0f-s,      s-2.0f,    s,
                          2.0f*s, s-3.0f, 3.0f-2.0f*s,   -s,
                              -s,   0.0f,           s, 0.0f,
                            0.0f,   1.0f,        0.0f, 0.0f};

glm::mat4 B; //basic matrix
glm::mat4x3 C; //control matrix
glm::vec4 catmullRom(GLfloat u) {
    glm::vec4 U(pow(u,3), pow(u,2), u, 1.0);
    return U;
}

glm::vec4 dcatmullRom(GLfloat u) {
    glm::vec4 dU(3*pow(u,2), 2*u, 1.0, 0.0);
    return dU;
}


vector<glm::vec3> splinePoints;
vector<glm::vec3> subsplinePoints;
vector<glm::vec4> splineColors;
vector<glm::vec4> lc, rc;
vector<glm::vec3> subtangents;
vector<glm::vec3> tangents, normals;
vector<glm::vec3> binormals;
vector<glm::vec3> lrail, rrail, midr; // rail triangle vertices
vector<glm::vec3> lrail_n, rrail_n, midr_n; // rail tiangle vertex normals
//texture
vector<glm::vec3> pospx, posnx, posg, poss, pospz, posnz;
vector<glm::vec2> uvpx, uvnx, uvg, uvs, uvpz, uvnz;
glm::mat3x4 CT;

int middistance=0;
int camera=0;
int tmpc=0;
glm::vec4 col = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
float alpha1 = 0.03f, alpha2 = 0.01f; //Rail shape


// VBOs and VAOs
GLuint vboSpline, vaoSpline;
GLuint vbolrail, vbolrail_n, vborrail, vborrail_n, vbomidr, vbomidr_n;
GLuint vaolrail, vaorrail, vaomidr;
GLuint vbopx, vbonx, vbopy, vbony, vbopz, vbonz;
GLuint vaopx, vaonx, vaopy, vaony, vaopz, vaonz;
GLuint texHandlepx,texHandlenx,texHandlepy,texHandleny,texHandlepz,texHandlenz;

bool shot = false;
bool all = false;
bool stop = false;
bool rota = false;
bool autoup = false;
bool restore = false;
bool start = false;
int cu=0;
int cd=0;
int cr=0;
int screenshot=0;
bool animation = false;
bool takeshot = false;
int c=0;

// spline struct
// contains how many control points the spline has, and an array of control points
struct Spline
{
    int numControlPoints;
    Point * points;
};

// the spline array
Spline * splines;
// total number of splines
int numSplines;

void setTextureUnit(GLint unit) {
      glActiveTexture(unit);
      // select texture unit affected by subsequent texture calls // get a handle to the “textureImage” shader variable

      GLint h_textureImage = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "textureImage");
      // deem the shader variable “textureImage” to read from texture unit “unit”
      glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}


// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
    unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

    ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

    if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
      cout << "File " << filename << " saved successfully." << endl;
    else cout << "Failed to save file " << filename << '.' << endl;

    delete [] screenshotData;
}

void displayFunc(){
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.LoadIdentity();
      
    //matrix.LookAt(0, 0, 0, 0, 0, 0, 0, 1, 0); origin
    
      
    if(all==false){
        matrix.LookAt((splinePoints[camera]+ binormals[camera] * 0.5f).x,
                      (splinePoints[camera]+ binormals[camera] * 0.5f).y,
                      (splinePoints[camera]+ binormals[camera] * 0.5f).z,
                      (splinePoints[camera]+ tangents[camera]).x,
                      (splinePoints[camera]+ tangents[camera]).y,
                      (splinePoints[camera]+ tangents[camera]).z,
                      binormals[camera].x,
                      binormals[camera].y,
                      binormals[camera].z);
    }
     
      
      
      
      
    //ch8
    //transform
    matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
    matrix.Rotate(landRotate[0], 1.0, 0.0, 0.0);//x
    matrix.Rotate(landRotate[1], 0.0, 1.0, 0.0);//y
    matrix.Rotate(landRotate[2], 0.0, 0.0, 1.0);//z
    matrix.Scale(landScale[0], landScale[1], landScale[2]);
      
      
      
    float m[16];
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.GetMatrix(m);

    float p[16];
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.GetMatrix(p);
    //
    // bind shader
    pipelineProgram->Bind();

    // set variable
    pipelineProgram->SetModelViewMatrix(m);
    pipelineProgram->SetProjectionMatrix(p);
    
    texturePipelineProgram->Bind();
      
    texturePipelineProgram->SetModelViewMatrix(m);
    texturePipelineProgram->SetProjectionMatrix(p);
     
    /*milestone
    glBindVertexArray(vaoSpline);
    glDrawArrays(GL_LINES, 0, splinePoints.size());
    */
    pipelineProgram->Bind();
      
    //setup the phong constant
    float La[4] = {0.1f, 0.1f, 0.1f};
    float ka[4] = {1.0f, 1.0f, 1.0f};
    
    float Ld[4] = {0.0f, 0.0f, 0.0f};
    float kd[4] = {1.0f, 1.0f, 1.0f};
    
    float Ls[4] = {0.5f, 0.5f, 0.5f};
    float ks[4] = {1.0f, 1.0f, 1.0f};
    
    
    GLint h_La = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "La");
    glUniform4fv(h_La, 1, La);
    GLint h_ka = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ka");
    glUniform4fv(h_ka, 1, ka);
    GLint h_Ld = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ld");
    glUniform4fv(h_Ld, 1, Ld);
    GLint h_kd = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "kd");
    glUniform4fv(h_kd, 1, kd);
    GLint h_Ls = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ls");
    glUniform4fv(h_Ls, 1, Ls);
    GLint h_ks = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ks");
    glUniform4fv(h_ks, 1, ks);

    //set it to 1.0f
    GLint h_alpha = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "alpha");
    glUniform1f(h_alpha, 1.0f);
      
      
    float view[16];
    //openGLMatrix->GetMatrix(view); // read the view matrix
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.LoadIdentity();
    matrix.GetMatrix(view);
      
      
    //float lightDirection[3] = { 0, 1, 0 }; // the “Sun” at noon
    glm::vec3 lightDirection = glm::vec3( 0.0f, 1.0f, 0.0f ); // the “Sun” at noon
    float viewLightDirection[3]; // light direction in the view space
    // the following line is pseudo-code:
    // viewLightDirection is vec3
    // viewLightDirection = (view * float4(lightDirection, 0.0)).xyz;
    viewLightDirection[0] = (view[0] * lightDirection.x + view[0] * lightDirection.y + view[0] * lightDirection.z);
    viewLightDirection[1] = (view[1] * lightDirection.x + view[1] * lightDirection.y + view[1] * lightDirection.z);
    viewLightDirection[2] = (view[2] * lightDirection.x + view[2] * lightDirection.y + view[2] * lightDirection.z);
      
    // get a handle to the program
    GLuint program = pipelineProgram->GetProgramHandle();
    // get a handle to the viewLightDirection shader variable
    GLint h_viewLightDirection = glGetUniformLocation(program, "viewLightDirection");
    // upload viewLightDirection to the GPU
    glUniform3fv(h_viewLightDirection, 1, viewLightDirection);
    // continue with model transformations
    program = pipelineProgram->GetProgramHandle();
    GLint h_normalMatrix = glGetUniformLocation(program, "normalMatrix");
    float n[16];
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.GetNormalMatrix(n); // get normal matrix
    // upload n to the GPU
    GLboolean isRowMajor = GL_FALSE;
    glUniformMatrix4fv(h_normalMatrix, 1, isRowMajor, n);
      
      
    glBindVertexArray(vaolrail);
    glDrawArrays(GL_TRIANGLES, 0, lrail.size());
    glBindVertexArray(vaomidr);
    glDrawArrays(GL_TRIANGLES, 0, midr.size());
    glBindVertexArray(vaorrail);
    glDrawArrays(GL_TRIANGLES, 0, rrail.size());

      
    texturePipelineProgram->Bind();
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandlepx);
    glBindVertexArray(vaopx);
    glDrawArrays(GL_TRIANGLES, 0, pospx.size());
      
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandlenx);
    glBindVertexArray(vaonx);
    glDrawArrays(GL_TRIANGLES, 0, posnx.size());
      
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandlepz);
    glBindVertexArray(vaopz);
    glDrawArrays(GL_TRIANGLES, 0, pospz.size());
      
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandlenz);
    glBindVertexArray(vaonz);
    glDrawArrays(GL_TRIANGLES, 0, posnz.size());
      
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandlepy);
    glBindVertexArray(vaopy);
    glDrawArrays(GL_TRIANGLES, 0, posg.size());
      
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandleny);
    glBindVertexArray(vaony);
    glDrawArrays(GL_TRIANGLES, 0, poss.size());
      
      
    glBindVertexArray(0);// unbind vao
    glutSwapBuffers();
}



void idleFunc()
{
      // do some stuff...

      // for example, here, you can save the screenshots to disk (to make the animation)

      // make the screen update
      
      //make it scale up and down to look funny, and slowly rotate it with y-axis to show the outcome
      if(stop == false){
          if(camera<splinePoints.size()){
              camera+=1;
          }else{
              camera=0;
          }
      }
      
    
    
      if(start){//bounce
          if(autoup && landScale[0] <= 4.0f){
              landScale[0] += 0.05f;
              landScale[1] += 0.05f;
              cu++;
              if(cu==24){
                  autoup = false;
                  restore = true;
                  cu=0;
              }
          }
          
          if(restore && landScale[0] >= 1.0f){
              landScale[0] -= 0.05f;
              landScale[1] -= 0.05f;
              cd++;
              if(cd==24){
                  autoup = true;
                  restore = false;
                  cd=0;
              }
          }
      }
      
      if(rota){
          landRotate[1] += 0.3f;
          cr++;
          if(cr==1200){
              rota = false;
              cr=0;
          }
      }
    if(animation || takeshot){
        
        screenshot++;
    }
    
      
    if(c<1000 && screenshot%2==1 && takeshot){
        char filenum[4];
        sprintf(filenum, "%03d", c);
        string filename(string(filenum) + ".jpg");
        saveScreenshot(filename.c_str());
        c++;
    }
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
      glViewport(0, 0, w, h);

      matrix.SetMatrixMode(OpenGLMatrix::Projection);
      matrix.LoadIdentity();
      matrix.Perspective(65.0f, (float)w / (float)h, 0.01f, 1000.0f);
      //Don't over-stretch the z-buffer. It has only finite precision.
      //A good call to setup perspective is:matrix->Perspective(fovy, aspect, 0.01, 1000.0);
}

void mouseMotionDragFunc(int x, int y)
{
      // mouse has moved and one of the mouse buttons is pressed (dragging)

      // the change in mouse position since the last invocation of this function
      int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
      //angle
      
      switch (controlState)
      {
      // translate the landscape
          case TRANSLATE:
              if (leftMouseButton)
              {
                  // control x,y translation via the left mouse button
                  landTranslate[0] += mousePosDelta[0] * 0.01f;
                  landTranslate[1] -= mousePosDelta[1] * 0.01f;
              }
              if (middleMouseButton)
              {
                  // control z translation via the middle mouse button
                  landTranslate[2] += mousePosDelta[1] * 1.0f;
              }
              break;

              // rotate the landscape
          case ROTATE:
              if (leftMouseButton)
              {
                  // control x,y rotation via the left mouse button
                  landRotate[0] += mousePosDelta[1];
                  landRotate[1] += mousePosDelta[0];
              }
              if (middleMouseButton)
              {
                  // control z rotation via the middle mouse button
                  landRotate[2] += mousePosDelta[1];
              }
              break;

              // scale the landscape
          case SCALE:
              if (leftMouseButton)
              {
                  // control x,y scaling via the left mouse button
                  landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
                  landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
              }
              if (middleMouseButton)
              {
                  // control z scaling via the middle mouse button
                  landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
              }
              break;
      }

      // store the new mouse position
      mousePos[0] = x;
      mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
      // mouse has moved
      // store the new mouse position
      mousePos[0] = x;
      mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
    // a mouse button has has been pressed or depressed

    // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
    switch (button)
    {
      case GLUT_LEFT_BUTTON:
        leftMouseButton = (state == GLUT_DOWN);
      break;

      case GLUT_MIDDLE_BUTTON:
        middleMouseButton = (state == GLUT_DOWN);
      break;

      case GLUT_RIGHT_BUTTON:
        rightMouseButton = (state == GLUT_DOWN);
      break;
    }

    // keep track of whether CTRL and SHIFT keys are pressed
    switch (glutGetModifiers())
    {
      //I cannot use CTRL on macbook M1 clip
            //so I change to alt
      case GLUT_ACTIVE_ALT:
        controlState = TRANSLATE;
      break;

      case GLUT_ACTIVE_SHIFT:
        controlState = SCALE;
      break;

      // if CTRL and SHIFT are not pressed, we are in rotate mode
      default:
        controlState = ROTATE;
      break;
    }

    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  GLint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");//basic.vertexShader.glsl
  switch (key)
  {
      case 27: // ESC key
          exit(0); // exit the program
          break;

      case ' ':
          cout << "You pressed the spacebar." << endl;
          break;

      case 'x':
          // take a screenshot
          saveScreenshot("screenshot.jpg");
          break;
          
      case '5'://autoup
          autoup = true;
          start = true;
          cout << "Auto on." << endl;
      break;
      
      case '6'://stop autoup
          start = false;
          landScale[0] = 1.0f;
          landScale[1] = 1.0f;
          cout << "Auto off." << endl;
      break;
          
      case '7'://rotate
          rota = true;
          cout << "Rotate on." << endl;
      break;

      case 's'://Stop
          stop = true;
          cout << "Stop." << endl;
      break;
          
      case 'g'://Go
          stop = false;
          cout << "Go." << endl;
      break;
          
      case 'q'://see total view
          all = true;
          cout << "See total view." << endl;
      break;
          
      case 'w'://keep on rail
          all = false;
          landRotate[0] = 0.0f;
          landRotate[1] = 0.0f;
          landRotate[2] = 0.0f;
          landTranslate[0] = 0.0f;
          landTranslate[1] = 0.0f;
          landTranslate[2] = 0.0f;
          landScale[0] =  1.0f;
          landScale[1] =  1.0f;
          landScale[2] =  1.0f;
          cout << "Keep on rail." << endl;
      break;
          
      case 't'://Animation & Take screenshot
          takeshot = true;
          cout << "Animation & Take screenshots." << endl;
      break;
          
      case 'r'://Initialize all the land transform
          landRotate[0] = 0.0f;
          landRotate[1] = 0.0f;
          landRotate[2] = 0.0f;
          landTranslate[0] = 0.0f;
          landTranslate[1] = 0.0f;
          landTranslate[2] = 0.0f;
          landScale[0] =  1.0f;
          landScale[1] =  1.0f;
          landScale[2] =  1.0f;
          cout << "Initialize all the land transforms." << endl;
      break;
      
  }
}

int loadSplines(char * argv)
{
    char * cName = (char *) malloc(128 * sizeof(char));
    FILE * fileList;
    FILE * fileSpline;
    int iType, i = 0, j, iLength;

    // load the track file
    fileList = fopen(argv, "r");
    if (fileList == NULL)
    {
      printf ("can't open file\n");
      exit(1);
    }

    // stores the number of splines in a global variable
    fscanf(fileList, "%d", &numSplines);

    splines = (Spline*) malloc(numSplines * sizeof(Spline));

    // reads through the spline files
    for (j = 0; j < numSplines; j++)
    {
      i = 0;
      fscanf(fileList, "%s", cName);
      fileSpline = fopen(cName, "r");

      if (fileSpline == NULL)
      {
        printf ("can't open file\n");
        exit(1);
      }

      // gets length for spline file
      fscanf(fileSpline, "%d %d", &iLength, &iType);

      // allocate memory for all the points
      splines[j].points = (Point *)malloc(iLength * sizeof(Point));
      splines[j].numControlPoints = iLength;

      // saves the data to the struct
      while (fscanf(fileSpline, "%lf %lf %lf",
       &splines[j].points[i].x,
       &splines[j].points[i].y,
       &splines[j].points[i].z) != EOF)
      {
        i++;
      }
    }

    free(cName);

    return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle){
    
    // read the texture image
    ImageIO img;
    ImageIO::fileFormatType imgFormat;
    ImageIO::errorType err = img.load(imageFilename, &imgFormat);

    if (err != ImageIO::OK){
        
      printf("Loading texture from %s failed.\n", imageFilename);
      return -1;
    }

    // check that the number of bytes is a multiple of 4
    if (img.getWidth() * img.getBytesPerPixel() % 4){
        
      printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
      return -1;
    }

    // allocate space for an array of pixels
    int width = img.getWidth();
    int height = img.getHeight();
    unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

    // fill the pixelsRGBA array with the image pixels
    memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
    for (int h = 0; h < height; h++)
      for (int w = 0; w < width; w++)
      {
        // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
        pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
        pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
        pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
        pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

        // set the RGBA channels, based on the loaded image
        int numChannels = img.getBytesPerPixel();
        for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
          pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
      }

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // initialize the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

    // generate the mipmaps for this texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // set the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // added for skybox
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // added for skybox
    // query support for anisotropic texture filtering
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    printf("Max available anisotropic samples: %f\n", fLargest);
    // set anisotropic texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

    // query for any errors
    GLenum errCode = glGetError();
    if (errCode != 0)
    {
      printf("Texture initialization error. Error code: %d.\n", errCode);
      return -1;
    }

    // de-allocate the pixel array -- it is no longer needed
    delete [] pixelsRGBA;

    return 0;
}


void displaysplinemilestone(){
    //brute force
    //Catmull-Rom Spline Matrix
    //p(u) = U * basis * control_matrix
    B = glm::make_mat4x4(basis);
    glm::mat4 BT = transpose(B);
  
    for (int i=0; i < numSplines; i++){
        
        for (int j=0; j < splines[i].numControlPoints - 3; j++){
            
            double controlp[12] = {splines[i].points[j].x, splines[i].points[j].y, splines[i].points[j].z,
                                  splines[i].points[j+1].x, splines[i].points[j+1].y, splines[i].points[j+1].z,
                                  splines[i].points[j+2].x, splines[i].points[j+2].y, splines[i].points[j+2].z,
                                  splines[i].points[j+3].x, splines[i].points[j+3].y, splines[i].points[j+3].z};

            C = glm::make_mat4x3(controlp);
            glm::mat3x4 CT = transpose(C);
            for (float u = 0.000f; u <= 1.000f; u += 0.002f)
            {
                glm::vec4 U = catmullRom(u);
                glm::vec3 splinePoint = U * BT * CT;
                splinePoints.push_back(splinePoint);
                splinePoints.push_back(splinePoint);
                splineColors.push_back(col);
                splineColors.push_back(col);
                
            }
            
        }
    }
}

void displayspline(){
    //brute force
    //Catmull-Rom Spline Matrix
    //p(u) = U * basis * control_matrix
    B = glm::make_mat4x4(basis);
    glm::mat4 BT = transpose(B);
  
    for (int i=0; i < numSplines; i++){
        
        for (int j=0; j < splines[i].numControlPoints - 3; j++){
            
            double controlp[12] = {splines[i].points[j].x, splines[i].points[j].y, splines[i].points[j].z,
                                  splines[i].points[j+1].x, splines[i].points[j+1].y, splines[i].points[j+1].z,
                                  splines[i].points[j+2].x, splines[i].points[j+2].y, splines[i].points[j+2].z,
                                  splines[i].points[j+3].x, splines[i].points[j+3].y, splines[i].points[j+3].z};

            C = glm::make_mat4x3(controlp);
            glm::mat3x4 CT = transpose(C);
            
            for (float u = 0.000f; u <= 1.000f; u += 0.01f){

                glm::vec4 U = catmullRom(u);
                glm::vec3 splinePoint = U * BT * CT;
                splinePoints.push_back(splinePoint);
                
                glm::vec4 dU = dcatmullRom(u);
                glm::vec3 tangent = dU * BT * CT;
                tangents.push_back(glm::normalize(tangent));
                  
            }

        }
        
    }
}


void subdivide(float u0, float u1, float maxlinelength){
    
    //Catmull-Rom Spline Matrix
    //p(u) = U * basis * control_matrix
    B = glm::make_mat4x4(basis);
    glm::mat4 BT = transpose(B);
  
    for (int i=0; i < numSplines; i++){
        
        for (int j=0; j < splines[i].numControlPoints - 3; j++){
            
            double controlp[12] = {splines[i].points[j].x, splines[i].points[j].y, splines[i].points[j].z,
                                  splines[i].points[j+1].x, splines[i].points[j+1].y, splines[i].points[j+1].z,
                                  splines[i].points[j+2].x, splines[i].points[j+2].y, splines[i].points[j+2].z,
                                  splines[i].points[j+3].x, splines[i].points[j+3].y, splines[i].points[j+3].z};

            C = glm::make_mat4x3(controlp);
            glm::mat3x4 CT = transpose(C);
        }
    }
    
    GLfloat umid = (u0+u1)/2;
    
    glm::vec4 U0 = catmullRom(u0);
    glm::vec4 U1 = catmullRom(u1);
    
    glm::vec3 x0 = U0 * BT * CT;
    glm::vec3 x1 = U1 * BT * CT;
    
    glm::vec4 dU0 = dcatmullRom(u0);
    glm::vec4 dU1 = dcatmullRom(u1);
    
    glm::vec3 t1 = dU0 * BT * CT;
    glm::vec3 t2 = dU1 * BT * CT;
    
    if(glm::length(x1-x0) > maxlinelength){
        subdivide(u0, umid, maxlinelength);
        subdivide(umid, u1, maxlinelength);
    }else{
        splinePoints.push_back(x0);
        splinePoints.push_back(x1);
        subtangents.push_back(glm::normalize(t1));
        subtangents.push_back(glm::normalize(t2));
    }
    
    
}





int t=0;
void spline(){

    glm::vec3 v = glm::vec3(0.0f, 1.0f, 0.0f);
    
    //  First spline point  : N0 = unit(T0 x V) and B0 = unit(T0 x N0)
    if (tangents[t] == v){
        v = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::vec3 normal = glm::normalize(glm::cross(tangents[t], v));
    normals.push_back(normal);
    glm::vec3 binormal = glm::normalize(glm::cross(tangents[t], normal));
    binormals.push_back(binormal);
    t++;
    
    //  Other spline points : N1 = unit(B0 x T1) and B1 = unit(T1 x N1)
    for (; t < splinePoints.size(); t++){// because of t++, now need to minus 1
        
        glm::vec3 normal = glm::normalize(glm::cross(binormals[t-1], tangents[t]));
        normals.push_back(normal);
        glm::vec3 binormal = glm::normalize(glm::cross(tangents[t], normals[t]));
        binormals.push_back(binormal);
    }

}

glm::vec3 rail(glm::vec3 p, glm::vec3 n, glm::vec3 b){
    // create the rectangle
    glm::vec3 v;
    n = n * alpha1;
    b = b * alpha2;
    v = p + n + b;
    return v;
}

glm::vec3 mrail(glm::vec3 p, glm::vec3 n, glm::vec3 b){
    // create the rectangle
    glm::vec3 v;
    n = n * 0.015f;
    b = b * 0.005f;
    v = p + n + b;
    return v;
}


void buildRails(){
    
      /* The formula is:
      v0 = p0 + (alpha * -n0) +(alpha * b0) ->v4
      v1 = p0 + (alpha * n0) +(alpha * b0)  ->v5
      v2 = p0 + (alpha * n0) -(alpha * b0)  ->v6
      v3 = p0 + (alpha * -n0) -(alpha * b0) ->v7
      */
    
      //build 3 rail to make a T-shaped rail
      glm::vec3 lrail_p0, lrail_p1, rrail_p0, rrail_p1;
      glm::vec3 lrail_v0, lrail_v1, lrail_v2, lrail_v3, lrail_v4, lrail_v5, lrail_v6, lrail_v7;
      glm::vec3 rrail_v0, rrail_v1, rrail_v2, rrail_v3, rrail_v4, rrail_v5, rrail_v6, rrail_v7;
    
      glm::vec3 lt1_p0, lt1_p1, rt1_p0, rt1_p1;
      glm::vec3 lt1_v0, lt1_v1, lt1_v2, lt1_v3, lt1_v4, lt1_v5, lt1_v6, lt1_v7;
      glm::vec3 rt1_v0, rt1_v1, rt1_v2, rt1_v3, rt1_v4, rt1_v5, rt1_v6, rt1_v7;
      
      glm::vec3 lt2_p0, lt2_p1, rt2_p0, rt2_p1;
      glm::vec3 lt2_v0, lt2_v1, lt2_v2, lt2_v3, lt2_v4, lt2_v5, lt2_v6, lt2_v7;
      glm::vec3 rt2_v0, rt2_v1, rt2_v2, rt2_v3, rt2_v4, rt2_v5, rt2_v6, rt2_v7;
      

      for (int i=0; i < splinePoints.size() - 1; i++){
          // Left rail vertices
          // Right rail vertices
          lrail_p0 = splinePoints[i] - 0.1f*normals[i];
          rrail_p0 = splinePoints[i] + 0.1f*normals[i];
          lrail_v0 = rail(lrail_p0, -1.0f*normals[i], binormals[i]);
          rrail_v0 = rail(rrail_p0, -1.0f*normals[i], binormals[i]);
          lrail_v1 = rail(lrail_p0, normals[i],       binormals[i]);
          rrail_v1 = rail(rrail_p0, normals[i],       binormals[i]);
          lrail_v2 = rail(lrail_p0, normals[i],       -1.0f*binormals[i]);
          rrail_v2 = rail(rrail_p0, normals[i],       -1.0f*binormals[i]);
          lrail_v3 = rail(lrail_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          rrail_v3 = rail(rrail_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          

          int j=i+1;
          lrail_p1 = splinePoints[j] - 0.1f*normals[j];
          rrail_p1 = splinePoints[j] + 0.1f*normals[j];
          lrail_v4 = rail(lrail_p1, -1.0f*normals[j], binormals[j]);
          rrail_v4 = rail(rrail_p1, -1.0f*normals[j], binormals[j]);
          lrail_v5 = rail(lrail_p1, normals[j],       binormals[j]);
          rrail_v5 = rail(rrail_p1, normals[j],       binormals[j]);
          lrail_v6 = rail(lrail_p1, normals[j],       -1.0f*binormals[j]);
          rrail_v6 = rail(rrail_p1, normals[j],       -1.0f*binormals[j]);
          lrail_v7 = rail(lrail_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          rrail_v7 = rail(rrail_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          
          //for T-shape
          
          lt1_p0 = lrail_p0  - 0.02f*binormals[i];
          rt1_p0 = rrail_p0  - 0.02f*binormals[i];
          lt1_v0 = mrail(lt1_p0, -1.0f*normals[i], binormals[i]);
          rt1_v0 = mrail(rt1_p0, -1.0f*normals[i], binormals[i]);
          lt1_v1 = mrail(lt1_p0, normals[i],       binormals[i]);
          rt1_v1 = mrail(rt1_p0, normals[i],       binormals[i]);
          lt1_v2 = mrail(lt1_p0, normals[i],       -1.0f*binormals[i]);
          rt1_v2 = mrail(rt1_p0, normals[i],       -1.0f*binormals[i]);
          lt1_v3 = mrail(lt1_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          rt1_v3 = mrail(rt1_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          
          lt1_p1 = lrail_p1  - 0.02f*binormals[j];
          rt1_p1 = rrail_p1  - 0.02f*binormals[j];
          lt1_v4 = mrail(lt1_p1, -1.0f*normals[j], binormals[j]);
          rt1_v4 = mrail(rt1_p1, -1.0f*normals[j], binormals[j]);
          lt1_v5 = mrail(lt1_p1, normals[j],       binormals[j]);
          rt1_v5 = mrail(rt1_p1, normals[j],       binormals[j]);
          lt1_v6 = mrail(lt1_p1, normals[j],       -1.0f*binormals[j]);
          rt1_v6 = mrail(rt1_p1, normals[j],       -1.0f*binormals[j]);
          lt1_v7 = mrail(lt1_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          rt1_v7 = mrail(rt1_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          
          lt2_p0 = lt1_p0 - 0.02f*binormals[i];
          rt2_p0 = rt1_p0 - 0.02f*binormals[i];
          lt2_v0 = rail(lt2_p0, -1.0f*normals[i], binormals[i]);
          rt2_v0 = rail(rt2_p0, -1.0f*normals[i], binormals[i]);
          lt2_v1 = rail(lt2_p0, normals[i],       binormals[i]);
          rt2_v1 = rail(rt2_p0, normals[i],       binormals[i]);
          lt2_v2 = rail(lt2_p0, normals[i],       -1.0f*binormals[i]);
          rt2_v2 = rail(rt2_p0, normals[i],       -1.0f*binormals[i]);
          lt2_v3 = rail(lt2_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          rt2_v3 = rail(rt2_p0, -1.0f*normals[i], -1.0f*binormals[i]);
          
          lt2_p1 = lt1_p1 - 0.02f*binormals[j];
          rt2_p1 = rt1_p1 - 0.02f*binormals[j];
          lt2_v4 = rail(lt2_p1, -1.0f*normals[j], binormals[j]);
          rt2_v4 = rail(rt2_p1, -1.0f*normals[j], binormals[j]);
          lt2_v5 = rail(lt2_p1, normals[j],       binormals[j]);
          rt2_v5 = rail(rt2_p1, normals[j],       binormals[j]);
          lt2_v6 = rail(lt2_p1, normals[j],       -1.0f*binormals[j]);
          rt2_v6 = rail(rt2_p1, normals[j],       -1.0f*binormals[j]);
          lt2_v7 = rail(lt2_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          rt2_v7 = rail(rt2_p1, -1.0f*normals[j], -1.0f*binormals[j]);
          
          
          //left Rail
          //left
          lrail.push_back(lrail_v2);
          lrail.push_back(lrail_v6);
          lrail.push_back(lrail_v3);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[i]);
          
          lrail.push_back(lrail_v3);
          lrail.push_back(lrail_v6);
          lrail.push_back(lrail_v7);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[j]);
          
          //right
          lrail.push_back(lrail_v0);
          lrail.push_back(lrail_v4);
          lrail.push_back(lrail_v5);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[j]);
          
          lrail.push_back(lrail_v0);
          lrail.push_back(lrail_v5);
          lrail.push_back(lrail_v1);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[i]);
          
          //top
          lrail.push_back(lrail_v1);
          lrail.push_back(lrail_v5);
          lrail.push_back(lrail_v2);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[i]);
          
          lrail.push_back(lrail_v2);
          lrail.push_back(lrail_v5);
          lrail.push_back(lrail_v6);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[j]);
          
          //bottom
          lrail.push_back(lrail_v3);
          lrail.push_back(lrail_v7);
          lrail.push_back(lrail_v4);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[j]);
          
          lrail.push_back(lrail_v3);
          lrail.push_back(lrail_v4);
          lrail.push_back(lrail_v0);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[i]);
          
          //t1
          //left
          lrail.push_back(lt1_v2);
          lrail.push_back(lt1_v6);
          lrail.push_back(lt1_v3);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[i]);
          
          lrail.push_back(lt1_v3);
          lrail.push_back(lt1_v6);
          lrail.push_back(lt1_v7);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[j]);
          
          //right
          lrail.push_back(lt1_v0);
          lrail.push_back(lt1_v4);
          lrail.push_back(lt1_v5);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[j]);
          
          lrail.push_back(lt1_v0);
          lrail.push_back(lt1_v5);
          lrail.push_back(lt1_v1);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[i]);
          
          //top
          lrail.push_back(lt1_v1);
          lrail.push_back(lt1_v5);
          lrail.push_back(lt1_v2);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[i]);
          
          lrail.push_back(lt1_v2);
          lrail.push_back(lt1_v5);
          lrail.push_back(lt1_v6);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[j]);
          
          //bottom
          lrail.push_back(lt1_v3);
          lrail.push_back(lt1_v7);
          lrail.push_back(lt1_v4);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[j]);
          
          lrail.push_back(lt1_v3);
          lrail.push_back(lt1_v4);
          lrail.push_back(lt1_v0);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[i]);
          
          //t2
          //left
          lrail.push_back(lt2_v2);
          lrail.push_back(lt2_v6);
          lrail.push_back(lt2_v3);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[i]);
          
          lrail.push_back(lt2_v3);
          lrail.push_back(lt2_v6);
          lrail.push_back(lt2_v7);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*binormals[i]);
          lrail_n.push_back(-1.0f*binormals[j]);
          lrail_n.push_back(-1.0f*binormals[j]);
          
          //right
          lrail.push_back(lt2_v0);
          lrail.push_back(lt2_v4);
          lrail.push_back(lt2_v5);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[j]);
          
          lrail.push_back(lt2_v0);
          lrail.push_back(lt2_v5);
          lrail.push_back(lt2_v1);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(binormals[i]);
          lrail_n.push_back(binormals[j]);
          lrail_n.push_back(binormals[i]);
          
          //top
          lrail.push_back(lt2_v1);
          lrail.push_back(lt2_v5);
          lrail.push_back(lt2_v2);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[i]);
          
          lrail.push_back(lt2_v2);
          lrail.push_back(lt2_v5);
          lrail.push_back(lt2_v6);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(normals[i]);
          lrail_n.push_back(normals[j]);
          lrail_n.push_back(normals[j]);
          
          //bottom
          lrail.push_back(lt2_v3);
          lrail.push_back(lt2_v7);
          lrail.push_back(lt2_v4);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[j]);
          
          lrail.push_back(lt2_v3);
          lrail.push_back(lt2_v4);
          lrail.push_back(lt2_v0);
          //lc.push_back(col);
          //lc.push_back(col);
          lrail_n.push_back(-1.0f*normals[i]);
          lrail_n.push_back(-1.0f*normals[j]);
          lrail_n.push_back(-1.0f*normals[i]);
          
          //mid distance
          middistance = 5;
          if(i%5 == 0){
              //left
              midr.push_back(lrail_v2);
              midr.push_back(rrail_v3);
              midr.push_back(rrail_v7);
              midr_n.push_back(-1.0f*binormals[i]);
              midr_n.push_back(-1.0f*binormals[i]);
              midr_n.push_back(-1.0f*binormals[j]);
              
              midr.push_back(lrail_v2);
              midr.push_back(rrail_v7);
              midr.push_back(lrail_v6);
              midr_n.push_back(-1.0f*binormals[i]);
              midr_n.push_back(-1.0f*binormals[j]);
              midr_n.push_back(-1.0f*binormals[j]);
              
              //right
              midr.push_back(lrail_v1);
              midr.push_back(lrail_v5);
              midr.push_back(rrail_v4);
              midr_n.push_back(binormals[i]);
              midr_n.push_back(binormals[j]);
              midr_n.push_back(binormals[j]);
              
              midr.push_back(lrail_v1);
              midr.push_back(rrail_v4);
              midr.push_back(rrail_v0);
              midr_n.push_back(binormals[i]);
              midr_n.push_back(binormals[j]);
              midr_n.push_back(binormals[i]);
              
              //front
              midr.push_back(lrail_v1);
              midr.push_back(rrail_v0);
              midr.push_back(rrail_v3);
              midr_n.push_back(-1.0f*tangents[i]);
              midr_n.push_back(-1.0f*tangents[i]);
              midr_n.push_back(-1.0f*tangents[i]);
              
              midr.push_back(lrail_v1);
              midr.push_back(rrail_v3);
              midr.push_back(lrail_v2);
              midr_n.push_back(-1.0f*tangents[i]);
              midr_n.push_back(-1.0f*tangents[i]);
              midr_n.push_back(-1.0f*tangents[i]);
              
              //back
              midr.push_back(lrail_v5);
              midr.push_back(lrail_v6);
              midr.push_back(rrail_v7);
              midr_n.push_back(tangents[j]);
              midr_n.push_back(tangents[j]);
              midr_n.push_back(tangents[j]);
              
              midr.push_back(lrail_v5);
              midr.push_back(rrail_v7);
              midr.push_back(rrail_v4);
              midr_n.push_back(tangents[j]);
              midr_n.push_back(tangents[j]);
              midr_n.push_back(tangents[j]);
              
              //top
              midr.push_back(rrail_v3);
              midr.push_back(rrail_v0);
              midr.push_back(rrail_v4);
              midr_n.push_back(normals[i]);
              midr_n.push_back(normals[i]);
              midr_n.push_back(normals[j]);
              
              midr.push_back(rrail_v3);
              midr.push_back(rrail_v4);
              midr.push_back(rrail_v7);
              midr_n.push_back(normals[i]);
              midr_n.push_back(normals[j]);
              midr_n.push_back(normals[j]);
              
              //bottom
              midr.push_back(lrail_v2);
              midr.push_back(lrail_v6);
              midr.push_back(lrail_v5);
              midr_n.push_back(-1.0f*normals[i]);
              midr_n.push_back(-1.0f*normals[j]);
              midr_n.push_back(-1.0f*normals[j]);
              
              midr.push_back(lrail_v2);
              midr.push_back(lrail_v5);
              midr.push_back(lrail_v1);
              midr_n.push_back(-1.0f*normals[i]);
              midr_n.push_back(-1.0f*normals[j]);
              midr_n.push_back(-1.0f*normals[i]);
          }
          
          
          //right Rail
          //left
          rrail.push_back(rrail_v2);
          rrail.push_back(rrail_v6);
          rrail.push_back(rrail_v3);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*binormals[i]);
          rrail_n.push_back(-1.0f*binormals[j]);
          rrail_n.push_back(-1.0f*binormals[i]);
          
          rrail.push_back(rrail_v3);
          rrail.push_back(rrail_v6);
          rrail.push_back(rrail_v7);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*binormals[i]);
          rrail_n.push_back(-1.0f*binormals[j]);
          rrail_n.push_back(-1.0f*binormals[j]);
          
          //right
          rrail.push_back(rrail_v0);
          rrail.push_back(rrail_v4);
          rrail.push_back(rrail_v5);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(binormals[i]);
          rrail_n.push_back(binormals[j]);
          rrail_n.push_back(binormals[j]);
          
          rrail.push_back(rrail_v0);
          rrail.push_back(rrail_v5);
          rrail.push_back(rrail_v1);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(binormals[i]);
          rrail_n.push_back(binormals[j]);
          rrail_n.push_back(binormals[i]);
          
          //top
          rrail.push_back(rrail_v1);
          rrail.push_back(rrail_v5);
          rrail.push_back(rrail_v2);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[i]);
          
          rrail.push_back(rrail_v2);
          rrail.push_back(rrail_v5);
          rrail.push_back(rrail_v6);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[j]);
          
          //bottom
          rrail.push_back(rrail_v3);
          rrail.push_back(rrail_v7);
          rrail.push_back(rrail_v4);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[j]);
          
          rrail.push_back(rrail_v3);
          rrail.push_back(rrail_v4);
          rrail.push_back(rrail_v0);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[i]);
          
          //t1
          //left
          rrail.push_back(0.01f*rt1_v2);
          rrail.push_back(0.01f*rt1_v6);
          rrail.push_back(0.01f*rt1_v3);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-0.02f*binormals[i]);
          rrail_n.push_back(-0.02f*binormals[j]);
          rrail_n.push_back(-0.02f*binormals[i]);
          
          rrail.push_back(0.01f*rt1_v3);
          rrail.push_back(0.01f*rt1_v6);
          rrail.push_back(0.01f*rt1_v7);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-0.02f*binormals[i]);
          rrail_n.push_back(-0.02f*binormals[j]);
          rrail_n.push_back(-0.02f*binormals[j]);
          
          //right
          rrail.push_back(0.01f*rt1_v0);
          rrail.push_back(0.01f*rt1_v4);
          rrail.push_back(0.01f*rt1_v5);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(0.02f*binormals[i]);
          rrail_n.push_back(0.02f*binormals[j]);
          rrail_n.push_back(0.02f*binormals[j]);
          
          rrail.push_back(0.01f*rt1_v0);
          rrail.push_back(0.01f*rt1_v5);
          rrail.push_back(0.01f*rt1_v1);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(0.02f*binormals[i]);
          rrail_n.push_back(0.02f*binormals[j]);
          rrail_n.push_back(0.02f*binormals[i]);
          
          //top
          rrail.push_back(rt1_v1);
          rrail.push_back(rt1_v5);
          rrail.push_back(rt1_v2);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[i]);
          
          rrail.push_back(rt1_v2);
          rrail.push_back(rt1_v5);
          rrail.push_back(rt1_v6);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[j]);
          
          //bottom
          rrail.push_back(rt1_v3);
          rrail.push_back(rt1_v7);
          rrail.push_back(rt1_v4);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[j]);
          
          rrail.push_back(rt1_v3);
          rrail.push_back(rt1_v4);
          rrail.push_back(rt1_v0);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[i]);
          
          //t2
          //left
          rrail.push_back(rt2_v2);
          rrail.push_back(rt2_v6);
          rrail.push_back(rt2_v3);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*binormals[i]);
          rrail_n.push_back(-1.0f*binormals[j]);
          rrail_n.push_back(-1.0f*binormals[i]);
          
          rrail.push_back(rt2_v3);
          rrail.push_back(rt2_v6);
          rrail.push_back(rt2_v7);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*binormals[i]);
          rrail_n.push_back(-1.0f*binormals[j]);
          rrail_n.push_back(-1.0f*binormals[j]);
          
          //right
          rrail.push_back(rt2_v0);
          rrail.push_back(rt2_v4);
          rrail.push_back(rt2_v5);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(binormals[i]);
          rrail_n.push_back(binormals[j]);
          rrail_n.push_back(binormals[j]);
          
          rrail.push_back(rt2_v0);
          rrail.push_back(rt2_v5);
          rrail.push_back(rt2_v1);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(binormals[i]);
          rrail_n.push_back(binormals[j]);
          rrail_n.push_back(binormals[i]);
          
          //top
          rrail.push_back(rt2_v1);
          rrail.push_back(rt2_v5);
          rrail.push_back(rt2_v2);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[i]);
          
          rrail.push_back(rt2_v2);
          rrail.push_back(rt2_v5);
          rrail.push_back(rt2_v6);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(normals[i]);
          rrail_n.push_back(normals[j]);
          rrail_n.push_back(normals[j]);
          
          //bottom
          rrail.push_back(rt2_v3);
          rrail.push_back(rt2_v7);
          rrail.push_back(rt2_v4);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[j]);
          
          rrail.push_back(rt2_v3);
          rrail.push_back(rt2_v4);
          rrail.push_back(rt2_v0);
          //rc.push_back(col);
          //rc.push_back(col);
          rrail_n.push_back(-1.0f*normals[i]);
          rrail_n.push_back(-1.0f*normals[j]);
          rrail_n.push_back(-1.0f*normals[i]);
      }
}


void vbovaorail(){
      /*milestone
      glGenBuffers(1, &vboSpline); // get handle on VBO buffer
      glBindBuffer(GL_ARRAY_BUFFER, vboSpline);  // bind the VBO buffer
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * splinePoints.size()+sizeof(glm::vec4) * splineColors.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * splinePoints.size(), (glm::vec3*)(splinePoints.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * splinePoints.size(), sizeof(glm::vec4) * splineColors.size(), (glm::vec4 *) splineColors.data());
      
      glGenVertexArrays(1, &vaoSpline);
      glBindVertexArray(vaoSpline); // bind the VAO
      
      glBindBuffer(GL_ARRAY_BUFFER, vboSpline);
      
      GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc); // enable the "position" attribute
      const void* offset = (const void*) 0;
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset); // set the layout of the "position" attribute data

      loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
      glEnableVertexAttribArray(loc);
      const void* offset2 = (const void*)(sizeof(glm::vec3) * splinePoints.size());
      glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, offset2);
       */
      
      
    //vbo
    glGenBuffers(1, &vbolrail);
    glBindBuffer(GL_ARRAY_BUFFER, vbolrail);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*lrail.size(), lrail.data(), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lrail.size()+sizeof(glm::vec4) * lc.size(), lrail.data(), GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * lrail.size(), (glm::vec3*)(lrail.data()));
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lrail.size(), sizeof(glm::vec4) * lc.size(), (glm::vec4 *) lc.data());

    glGenBuffers(1, &vbolrail_n);
    glBindBuffer(GL_ARRAY_BUFFER, vbolrail_n);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lrail_n.size(), lrail_n.data(), GL_STATIC_DRAW);
        
        
    glGenBuffers(1, &vbomidr);
    glBindBuffer(GL_ARRAY_BUFFER, vbomidr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*midr.size(), midr.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vbomidr_n);
    glBindBuffer(GL_ARRAY_BUFFER, vbomidr_n);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * midr_n.size(), midr_n.data(), GL_STATIC_DRAW);
    

    glGenBuffers(1, &vborrail);
    glBindBuffer(GL_ARRAY_BUFFER, vborrail);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*rrail.size(), rrail.data(), GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * rrail.size()+sizeof(glm::vec4) * rc.size(), rrail.data(), GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * rrail.size(), (glm::vec3*)(rrail.data()));
    //glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * rrail.size(), sizeof(glm::vec4) * rc.size(), (glm::vec4 *) rc.data());

    glGenBuffers(1, &vborrail_n);
    glBindBuffer(GL_ARRAY_BUFFER, vborrail_n);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * rrail_n.size(), rrail_n.data(), GL_STATIC_DRAW);
      
      
      
    
    //vao
    //left rail
    glGenVertexArrays(1, &vaolrail);
    glBindVertexArray(vaolrail);
    glBindBuffer(GL_ARRAY_BUFFER, vbolrail);
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    const void* offset = (const void*) 0;
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
      
    glBindBuffer(GL_ARRAY_BUFFER, vbolrail_n);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "normal");
    //const void* offsetl = (const void*)(sizeof(glm::vec3) * lc.size());
    //loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    //glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, offsetl);
    
    //mid rail
    glGenVertexArrays(1, &vaomidr);
    glBindVertexArray(vaomidr);
    glBindBuffer(GL_ARRAY_BUFFER, vbomidr);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
      
    glBindBuffer(GL_ARRAY_BUFFER, vbomidr_n);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "normal");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

    
    //right rail
    glGenVertexArrays(1, &vaorrail);
    glBindVertexArray(vaorrail);
    glBindBuffer(GL_ARRAY_BUFFER, vborrail);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    
    glBindBuffer(GL_ARRAY_BUFFER, vborrail_n);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "normal");
    //const void* offsetr = (const void*)(sizeof(glm::vec3) * rc.size());
    //loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    //glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, offsetr);
      
}

void textureImage(){
      float value=100.0f;
      //+x, -x, +y, ground, +z, -z
      /*                                      +y
      // positions                        -x  +z  +x  -z
                                              -y
      // texture coords(0,0 0,1 1,0 1,1) 2 triangle
                                               *****
                                               *   *
                                           *****************
                                           *   *   *   *   *
                                           *****************
                                               *   *
                                               *****
       clockwise
       +x
       +  +   +   0   0,
       +  -   +   0   1,
       +  -   -   1   1,
       +  +   +   0   0,
       +  -   -   1   1,
       +  +   -   1   0,
       
       -x
       -  +   -   0   0,
       -  -   -   0   1,
       -  -   +   1   1,
       -  +   -   0   0,
       -  -   +   1   1,
       -  +   +   1   0,
       
       +z
       -  +   +   0   0,
       -  -   +   0   1,
       +  -   +   1   1,
       -  +   +   0   0,
       +  -   +   1   1,
       +  +   +   1   0,
       
       -z
       +  +   -   0   0,
       +  -   -   0   1,
       -  -   -   1   1,
       +  +   -   0   0,
       -  -   -   1   1,
       -  +   -   1   0,
       
       ground
       +  -   +   1   1,
       +  -   -   1   0,
       -  -   -   0   0,
       +  -   +   1   1,
       -  -   -   0   0,
       -  -   +   0   1,
       
       sky
       +  +   -   1   1,
       +  +   +   1   0,
       -  +   +   0   0,
       +  +   -   1   1,
       -  +   +   0   0,
       -  +   -   0   1,
       
       
      */
      
      pospx.push_back(glm::vec3(value, value, value));
      pospx.push_back(glm::vec3(value, -value, value));
      pospx.push_back(glm::vec3(value, -value, -value));
      pospx.push_back(glm::vec3(value, value, value));
      pospx.push_back(glm::vec3(value, -value, -value));
      pospx.push_back(glm::vec3(value, value, -value));
      uvpx.push_back(glm::vec2(0,0));
      uvpx.push_back(glm::vec2(0,1));
      uvpx.push_back(glm::vec2(1,1));
      uvpx.push_back(glm::vec2(0,0));
      uvpx.push_back(glm::vec2(1,1));
      uvpx.push_back(glm::vec2(1,0));
      
      posnx.push_back(glm::vec3(-value, value, -value));
      posnx.push_back(glm::vec3(-value, -value, -value));
      posnx.push_back(glm::vec3(-value, -value, value));
      posnx.push_back(glm::vec3(-value, value, -value));
      posnx.push_back(glm::vec3(-value, -value, value));
      posnx.push_back(glm::vec3(-value, value, value));
      uvnx.push_back(glm::vec2(0,0));
      uvnx.push_back(glm::vec2(0,1));
      uvnx.push_back(glm::vec2(1,1));
      uvnx.push_back(glm::vec2(0,0));
      uvnx.push_back(glm::vec2(1,1));
      uvnx.push_back(glm::vec2(1,0));
      
      pospz.push_back(glm::vec3(-value, value, value));
      pospz.push_back(glm::vec3(-value, -value, value));
      pospz.push_back(glm::vec3(value, -value, value));
      pospz.push_back(glm::vec3(-value, value, value));
      pospz.push_back(glm::vec3(value, -value, value));
      pospz.push_back(glm::vec3(value, value, value));
      uvpz.push_back(glm::vec2(0,0));
      uvpz.push_back(glm::vec2(0,1));
      uvpz.push_back(glm::vec2(1,1));
      uvpz.push_back(glm::vec2(0,0));
      uvpz.push_back(glm::vec2(1,1));
      uvpz.push_back(glm::vec2(1,0));
      
      posnz.push_back(glm::vec3(value, value, -value));
      posnz.push_back(glm::vec3(value, -value, -value));
      posnz.push_back(glm::vec3(-value, -value, -value));
      posnz.push_back(glm::vec3(value, value, -value));
      posnz.push_back(glm::vec3(-value, -value, -value));
      posnz.push_back(glm::vec3(-value, value, -value));
      uvnz.push_back(glm::vec2(0,0));
      uvnz.push_back(glm::vec2(0,1));
      uvnz.push_back(glm::vec2(1,1));
      uvnz.push_back(glm::vec2(0,0));
      uvnz.push_back(glm::vec2(1,1));
      uvnz.push_back(glm::vec2(1,0));
      
      //ground
      posg.push_back(glm::vec3(value, -value, -value));
      posg.push_back(glm::vec3(value, -value, value));
      posg.push_back(glm::vec3(-value, -value, value));
      posg.push_back(glm::vec3(value, -value, -value));
      posg.push_back(glm::vec3(-value, -value, value));
      posg.push_back(glm::vec3(-value, -value, -value));
      uvg.push_back(glm::vec2(1,1));
      uvg.push_back(glm::vec2(1,0));
      uvg.push_back(glm::vec2(0,0));
      uvg.push_back(glm::vec2(1,1));
      uvg.push_back(glm::vec2(0,0));
      uvg.push_back(glm::vec2(0,1));
     
      //sky
      poss.push_back(glm::vec3(value, value, value));
      poss.push_back(glm::vec3(value, value, -value));
      poss.push_back(glm::vec3(-value, value, -value));
      poss.push_back(glm::vec3(value, value, value));
      poss.push_back(glm::vec3(-value, value, -value));
      poss.push_back(glm::vec3(-value, value, value));
      uvs.push_back(glm::vec2(1,1));
      uvs.push_back(glm::vec2(1,0));
      uvs.push_back(glm::vec2(0,0));
      uvs.push_back(glm::vec2(1,1));
      uvs.push_back(glm::vec2(0,0));
      uvs.push_back(glm::vec2(0,1));
  
}

void texvbovao(){
      glGenBuffers(1, &vbopx);
      glBindBuffer(GL_ARRAY_BUFFER, vbopx);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pospx.size() + sizeof(glm::vec2) * uvpx.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * pospx.size(), (glm::vec3*)(pospx.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pospx.size(), sizeof(glm::vec2) * uvpx.size(), (glm::vec2*)(uvpx.data()));
      
      glGenBuffers(1, &vbonx);
      glBindBuffer(GL_ARRAY_BUFFER, vbonx);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posnx.size() + sizeof(glm::vec2) * uvnx.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * posnx.size(), (glm::vec3*)(posnx.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posnx.size(), sizeof(glm::vec2) * uvnx.size(), (glm::vec2*)(uvnx.data()));
      
      glGenBuffers(1, &vbopz);
      glBindBuffer(GL_ARRAY_BUFFER, vbopz);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pospz.size() + sizeof(glm::vec2) * uvpz.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * pospz.size(), (glm::vec3*)(pospz.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pospz.size(), sizeof(glm::vec2) * uvpz.size(), (glm::vec2*)(uvpz.data()));
      
      glGenBuffers(1, &vbonz);
      glBindBuffer(GL_ARRAY_BUFFER, vbonz);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posnz.size() + sizeof(glm::vec2) * uvnz.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * posnz.size(), (glm::vec3*)(posnz.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posnz.size(), sizeof(glm::vec2) * uvnz.size(), (glm::vec2*)(uvnz.data()));
      
      glGenBuffers(1, &vbopy);
      glBindBuffer(GL_ARRAY_BUFFER, vbopy);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posg.size() + sizeof(glm::vec2) * uvg.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * posg.size(), (glm::vec3*)(posg.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * posg.size(), sizeof(glm::vec2) * uvg.size(), (glm::vec2*)(uvg.data()));
      
      glGenBuffers(1, &vbony);
      glBindBuffer(GL_ARRAY_BUFFER, vbony);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * poss.size() + sizeof(glm::vec2) * uvs.size(), NULL, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * poss.size(), (glm::vec3*)(poss.data()));
      glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * poss.size(), sizeof(glm::vec2) * uvs.size(), (glm::vec2*)(uvs.data()));
      
      
      
      
      glGenVertexArrays(1, &vaopx);
      glBindVertexArray(vaopx);
      glBindBuffer(GL_ARRAY_BUFFER, vbopx);
      GLuint loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * pospx.size()));
      
      glGenVertexArrays(1, &vaonx);
      glBindVertexArray(vaonx);
      glBindBuffer(GL_ARRAY_BUFFER, vbonx);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * posnx.size()));
      
      glGenVertexArrays(1, &vaopz);
      glBindVertexArray(vaopz);
      glBindBuffer(GL_ARRAY_BUFFER, vbopz);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * pospz.size()));
      
      glGenVertexArrays(1, &vaonz);
      glBindVertexArray(vaonz);
      glBindBuffer(GL_ARRAY_BUFFER, vbonz);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * posnz.size()));
      
      glGenVertexArrays(1, &vaopy);
      glBindVertexArray(vaopy);
      glBindBuffer(GL_ARRAY_BUFFER, vbopy);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * posg.size()));
      
      glGenVertexArrays(1, &vaony);
      glBindVertexArray(vaony);
      glBindBuffer(GL_ARRAY_BUFFER, vbony);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "position");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
      loc = glGetAttribLocation(texturePipelineProgram->GetProgramHandle(), "texCoord");
      glEnableVertexAttribArray(loc);
      glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (sizeof(glm::vec3) * poss.size()));
}

void textureHandle(){
      //skybox http://titan.csit.rmit.edu.au/~e20068/teaching/i3dg&a/2020/tute-tex.html
      //+x, -x, +y, -y, +z, and -z

      glGenTextures(1, &texHandlepx);

      int code = initTexture("skybox/right.jpg", texHandlepx);
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }

      glGenTextures(1, &texHandlenx);

      code = initTexture("skybox/left.jpg", texHandlenx);
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }

      glGenTextures(1, &texHandlepy);

      code = initTexture("skybox/top.jpg", texHandlepy);
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }

      glGenTextures(1, &texHandleny);

      code = initTexture("skybox/bottom.jpg", texHandleny);//ground
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }

      glGenTextures(1, &texHandlepz);

      code = initTexture("skybox/front.jpg", texHandlepz);
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }

      glGenTextures(1, &texHandlenz);

      code = initTexture("skybox/back.jpg", texHandlenz);
      if (code != 0)
      {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
      }
}

void initScene(int argc, char *argv[]){
    
    /*for(int i=0; i<splinePoints.size();i++){
        cout<< "{"
            << splinePoints[i].x << " " << splinePoints[i].y << " " << splinePoints[i].z
            << "}";
    }*/
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    //displaysplinemilestone();
    
    displayspline();
    //u0 = 0.0, u1 = 1.0, maxlinelength = 0.5
    subdivide(0.0, 1.0, 0.5);
    
    spline();
    buildRails();
    
    pipelineProgram = new BasicPipelineProgram;
    int ret = pipelineProgram->Init(shaderBasePath);
    if (ret != 0) abort();

    pipelineProgram->Bind();
      
    vbovaorail();
      
    textureImage();
      
    texturePipelineProgram = new TexturePipelineProgram;
    ret = texturePipelineProgram->Init(shaderBasePath);
    if (ret != 0) abort();

    texturePipelineProgram->Bind();
      
    textureHandle();
    texvbovao();
      
    glEnable(GL_DEPTH_TEST);

    std::cout << "GL error: " << glGetError() << std::endl;
      
}

int main(int argc, char *argv[]){
  
      if (argc<2)
      {
        printf ("usage: %s <trackfile>\n", argv[0]);
        exit(0);
      }

      // load the splines from the provided filename
      loadSplines(argv[1]);

      printf("Loaded %d spline(s).\n", numSplines);
      
      for(int i=0; i<numSplines; i++){
          printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);
      }
        
      
      cout << "Initializing GLUT..." << endl;
      glutInit(&argc,argv);

      cout << "Initializing OpenGL..." << endl;

      #ifdef __APPLE__
          glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
      #else
          glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
      #endif

      glutInitWindowSize(windowWidth, windowHeight);
      glutInitWindowPosition(0, 0);
      glutCreateWindow(windowTitle);

      cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
      cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
      cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

      #ifdef __APPLE__
          // This is needed on recent Mac OS X versions to correctly display the window.
          glutReshapeWindow(windowWidth - 1, windowHeight - 1);
      #endif

      // tells glut to use a particular display function to redraw
      glutDisplayFunc(displayFunc);   //glDrawArrays()
      // perform animation inside idleFunc
      glutIdleFunc(idleFunc);
      // callback for mouse drags
      glutMotionFunc(mouseMotionDragFunc);
      // callback for idle mouse movement
      glutPassiveMotionFunc(mouseMotionFunc);
      // callback for mouse button changes
      glutMouseFunc(mouseButtonFunc);
      // callback for resizing the window
      glutReshapeFunc(reshapeFunc);
      // callback for pressing the keys on the keyboard
      glutKeyboardFunc(keyboardFunc);

      // init glew
      #ifdef __APPLE__
          // nothing is needed on Apple
      #else
          // Windows, Linux
          GLint result = glewInit();
          if (result != GLEW_OK)
          {
              cout << "error: " << glewGetErrorString(result) << endl;
              exit(EXIT_FAILURE);
          }
      #endif
      
      // do initialization
      initScene(argc, argv);
      
      // sink forever into the glut loop
      glutMainLoop();
}
