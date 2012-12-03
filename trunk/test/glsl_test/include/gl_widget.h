#ifndef GL_WIDGET_H
#define GL_WIDGET_H

#include <GL/glew.h>

#include <QFile>
#include <QTimer>
#include <QGLWidget>
#include <QGLShader>
#include <QGLShaderProgram>

#include <offline_widget.h>

class Gl_widget: public QGLWidget
{
    Q_OBJECT
public:
    Gl_widget(int width, int height, const QString& path, QGLFormat& fmt, QWidget *parent = 0);
    ~Gl_widget();

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

	void init_yuv_textures();
	void delete_yuv_textures();
    
protected slots:
    void on_timeout();

private:
    int video_width;
    int video_height;
    
    GLuint prev_id;
    GLuint y_texture;
    GLuint u_texture;
    GLuint v_texture;
    
    QTimer timer;
    QGLShaderProgram program;

    Offline_widget offline;
};

#endif

