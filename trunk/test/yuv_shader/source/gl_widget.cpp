#include "gl_widget.h"

#include <QtGui>
#include <QtOpenGL>

#include <iostream>

Gl_widget::Gl_widget(QWidget* parent)
	:QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    , y_texture(0)
    , u_texture(0)
    , v_texture(0)
    , y_data(0)
    , u_data(0)
    , v_data(0)
    //, shader(QGLShader::Fragment, this)
    , program(this)
{
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
}

void Gl_widget::initializeGL()
{
    qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment,
     "uniform sampler2D y_texture;\n"
     "uniform sampler2D u_texture;\n"
     "uniform sampler2D v_texture;\n"
     "void main(void)\n"
     "{\n"
     "   float y, u, v, red, green, blue;\n"
     "   y = texture2D(y_texture, gl_TexCoord[0].st).r;\n"
     "   y =  1.1643 * (y - 0.0625);\n"
     "   u = texture2D(u_texture, gl_TexCoord[0].st).r - 0.5;\n"
     "   v = texture2D(v_texture, gl_TexCoord[0].st).r - 0.5;\n"
     "   red = y+1.5958*v;\n"
     "   green = y-0.39173*u-0.81290*v;\n"
     "   blue = y+2.017*u;\n"
     "   gl_FragColor = vec4(red, green, blue, 1.0);\n"
     "}");
     
    qDebug() << program.link();
    qDebug() << program.bind();
    
    y_data = new unsigned char[352*144*3];
    FILE* fptr = fopen("/home/prems/Documents/BigBuckBunny_CIF.yuv", "r");
    fseek(fptr, 800*352*144*3, SEEK_SET);
    fread(y_data, 1, 352*144*3, fptr);
    fclose(fptr);
    u_data = y_data+352*288;
    v_data = u_data+176*144;
    
    glGenTextures(1, &y_texture);
    glGenTextures(1, &u_texture);
    glGenTextures(1, &v_texture);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 352, 288, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y_data);
    
    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 176, 144, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u_data);
    
    glActiveTexture(GL_TEXTURE2); 
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 176, 144, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v_data);
    
    program.setUniformValue("y_texture", 0);
    program.setUniformValue("u_texture", 1);
    program.setUniformValue("v_texture", 2);
}

void Gl_widget::paintGL()
{
    //QColor color(0, 0xff, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glFlush();
    
    //memset(texture+60*352, 0, 352*100);
    
	//glActiveTexture(GL_TEXTURE0);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 352, 288, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture);
          
	glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);                      // Draw A Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8f, 0.8f, 0.0f);              // Top Left
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.8f, 0.8f, 0.0f);              // Top Right
        glTexCoord2f(1.0f,1.0f); glVertex3f( 0.8f,-0.8f, 0.0f);              // Bottom Right
        glTexCoord2f(0.0f,1.0f); glVertex3f(-0.8f,-0.8f, 0.0f);              // Bottom Left
    glEnd(); 
	glFlush();
    glDisable(GL_TEXTURE_2D);
}

void Gl_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);*/
}

