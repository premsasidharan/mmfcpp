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

    bool init_nms_shader();
    bool init_diff_shader();
    bool init_thresh_shader();
    bool init_gaussian_shader();
    bool init_adjust_shader();

    bool init_rgb_shader();
    bool init_gray_shader();
    bool init_yuy2_shader();

    void render_to_texture();
    void render_quad(int loc, int id, GLuint* fbo_buffs, int size);

    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    
protected slots:
    void render_frame(uint8_t* data);

private:
    enum {LUMA, CHROMA_UV, X2_Y2_XY, X2_Y2_XY_SMOOTH, R, CORNER, CROSS};
    enum {STAGE_DIFF, STAGE_GAUSS_SMOOTH, STAGE_THRESHOLD, STAGE_NMS, STAGE_ADJUST};

    GLuint fb_id;

    int v_width;
    int v_height;
    int threshold;
    uint8_t* yuv_data;

    GLuint texture[7];
    QGLShaderProgram* program[5];
    
    QGLShaderProgram nms_filter;
    QGLShaderProgram diff_filter;
    QGLShaderProgram thresh_filter;
    QGLShaderProgram smooth_filter;
    QGLShaderProgram adjust_filter;

    QGLShaderProgram rgb_filter;
    QGLShaderProgram gray_filter;
    QGLShaderProgram yuy2_filter;

    QMutex mutex;
    Gl_thread thread;

    static const GLfloat tex_coord[];
    static const GLfloat vertex_coord[][8];
};

#endif

