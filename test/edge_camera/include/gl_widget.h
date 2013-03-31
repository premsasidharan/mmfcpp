#ifndef GL_WIDGET_H
#define GL_WIDGET_H

#include <GL/glew.h>

#include <QFile>
#include <QTimer>
#include <QGLWidget>
#include <QGLShader>
#include <QGLShaderProgram>

#include <gl_thread.h>

class Gl_widget:public QGLWidget
{
    Q_OBJECT
public:
    Gl_widget(int w, int h, const QString& path, QGLFormat& fmt, QWidget *parent = 0);
    ~Gl_widget();

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

    void init_shaders();
	void init_textures();
	void delete_textures();

    bool init_edge_shader();
    bool init_nmes_shader();
    bool init_gray_shader();
    bool init_yuy2_shader();
    bool init_binary_shader();
    bool init_gaussian_shader();

    void render_to_texture();
    void render_quad(int loc, int id, GLuint* fbo_buffs, int size);

    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    
protected slots:
    void render_frame(uint8_t* data);

private:
    enum {LUMA, CHROMA_UV, SMOOTH, EDGE, NMES, BINARY};
    enum {STAGE_GAUSS_SMOOTH, STAGE_EDGE, STAGE_NMES, STAGE_BINARY};

    GLuint fb_id;

    int min_thr;
    int max_thr;

    int v_width;
    int v_height;
	uint8_t* yuv_data;

    GLuint texture[6];
    QGLShaderProgram* program[4];
    
    QGLShaderProgram nmes_filter; //non maximum edge suppression
    QGLShaderProgram gray_filter;
    QGLShaderProgram yuy2_filter;
    QGLShaderProgram edge_filter;
    QGLShaderProgram yuv420_filter;
    QGLShaderProgram binary_filter;
    QGLShaderProgram smooth_filter;

    QMutex mutex;
    Gl_thread thread;

    static const GLfloat tex_coord[];
    static const GLfloat vertex_coord[][8];
};

#endif

