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
    Gl_widget(int w, int h, const QString& path, QGLFormat& fmt, QWidget *parent = 0);
    ~Gl_widget();

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

    bool init_shader();
	void init_textures();
    void init_hist_buffers();

	void delete_textures();
    void delete_hist_buffers();
    void plote_histogram(int *max);

    void render_quad(int loc, int id, GLuint* fbo_buffs, int size);

    void closeEvent(QCloseEvent* event);
    
protected slots:
    void time_out();

private:
    enum {LUMA, CHROMA_U, CHROMA_V};
    
    GLuint buff_id;
    GLuint prev_id;

    int v_width;
    int v_height;

    GLuint texture[3];
    GLuint hist_obj[3];
    
    QGLShaderProgram yuv420_filter;

    QTimer timer;
    Offline_widget offline;

    static const GLfloat tex_coord[];
    static const GLfloat vertex_coord[][8];
};

#endif

