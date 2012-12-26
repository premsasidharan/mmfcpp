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

    void init_shaders();
	void init_yuv_textures();
	void delete_yuv_textures();

    bool init_edge_shader();
    bool init_nmes_shader();
    bool init_gray_shader();
    bool init_yuy2_shader();
    bool init_binary_shader();
    bool init_smoothing_shader();

    void render_to_texture();
    void render_quad(int loc, int id, GLuint* fbo_buffs, int size);

    void closeEvent(QCloseEvent* event);
    
protected slots:
    void render_frame();

private:
    GLuint fb_id;
    
    GLuint buff_id;
    GLuint prev_id;

    int video_width;
    int video_height;

    GLuint y_texture;
    GLuint uv_texture;
    GLuint nmes_texture; //non maximum edge suppression texture
    GLuint edge_texture;
    GLuint binary_texture;
    GLuint smooth_texture;
    
    QGLShaderProgram nmes_filter; //non maximum edge suppression
    QGLShaderProgram gray_filter;
    QGLShaderProgram yuy2_filter;
    QGLShaderProgram edge_filter;
    QGLShaderProgram binary_filter;
    QGLShaderProgram smooth_filter;

    Offline_widget offline;

    static const GLfloat tex_coord[];
    static const GLfloat vertex_coord[][8];

    static const GLfloat coeffs_fx[];
    static const GLfloat coeffs_fy[];
    static const GLfloat gauss_coeffs[];
};

#endif

