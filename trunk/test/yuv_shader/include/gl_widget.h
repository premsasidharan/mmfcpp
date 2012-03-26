#ifndef GL_WIDGET_H
#define GL_WIDGET_H

#include <QGLWidget>
#include <QGLShader>
#include <QGLShaderProgram>

class Gl_widget: public QGLWidget
{
    Q_OBJECT

public:
    Gl_widget(QWidget *parent = 0);
    ~Gl_widget();

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

private:
    GLuint y_texture;
    GLuint u_texture;
    GLuint v_texture;
    
    unsigned char* y_data;
    unsigned char* u_data;
    unsigned char* v_data;
    
    //QGLShader shader;
    QGLShaderProgram program;
};

#endif

