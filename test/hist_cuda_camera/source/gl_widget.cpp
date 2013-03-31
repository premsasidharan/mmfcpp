#include <gl_widget.h>

#include <stdint.h>

#include <QtGui>
#include <QPixmap>

#include <histogram.h>

const GLfloat Gl_widget::tex_coord[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
const GLfloat Gl_widget::vertex_coord[][8] = {{-1.0f, +1.0f, +0.0f, +1.0f, +0.0f, +0.0f, -1.0f, +0.0f},
                                              {+0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +0.0f},
                                              {-1.0f, +0.0f, +0.0f, +0.0f, +0.0f, -1.0f, -1.0f, -1.0f},
                                              {+0.0f, +0.0f, +1.0f, +0.0f, +1.0f, -1.0f, +0.0f, -1.0f},
                                              {-1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, -1.0f},
                                              {-1.0f, +1.0f, +0.0f, +1.0f, +0.0f, -1.0f, -1.0f, -1.0f},
                                              {+0.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, -1.0f}};

Gl_widget::Gl_widget(int w, int h, const QString& path, QGLFormat& fmt, QWidget* parent)
	:QGLWidget(fmt, parent)
    , v_width(w)
    , v_height(h)
    , yuv_data(0)
    , yuy2_filter(this)
    , thread(w, h, path)
{
    setWindowTitle("Camera: "+path);
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	delete_textures();
    delete_hist_buffers();
}

bool Gl_widget::init_shader()
{
    yuy2_filter.bindAttributeLocation("inVertex", 0);
    yuy2_filter.bindAttributeLocation("inTexCoord", 1);
    yuy2_filter.bindAttributeLocation("texCoord", 2);
    yuy2_filter.bindAttributeLocation("fragColor", 3);

	bool status = yuy2_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && yuy2_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/yuy2_rgb.fs");
    
    glBindFragDataLocation(yuy2_filter.programId(), 0, "fragColor");
    status = status && yuy2_filter.link();

    return status;
}

void Gl_widget::initializeGL()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        qDebug() << "Error: glewInit() failed\n";
        exit(0);
    }

    init_shader();
    init_textures();
    init_hist_buffers();

    glGenBuffers(3, hist_obj);    
    for (int i = 0; i < 3; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, hist_obj[i]);
        glBufferData(GL_ARRAY_BUFFER, 256*2*sizeof(GL_INT), 0, GL_DYNAMIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    connect(&thread, SIGNAL(update_widget(uint8_t*)), this, SLOT(render_frame(uint8_t*)));

    thread.start();
}

void Gl_widget::init_textures()
{
    glGenTextures(2, texture);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, texture[LUMA]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, v_width, v_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0);

    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, texture[CHROMA_UV]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, v_width>>1, v_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

void Gl_widget::init_hist_buffers()
{
    glGenBuffers(3, hist_obj);    
    for (int i = 0; i < 3; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, hist_obj[i]);
        glBufferData(GL_ARRAY_BUFFER, 256*2*sizeof(GL_INT), 0, GL_DYNAMIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Gl_widget::delete_textures()
{
    glDeleteTextures(2, texture);
}

void Gl_widget::delete_hist_buffers()
{
    glDeleteBuffers(3, hist_obj); 
}

void Gl_widget::render_quad(int loc, int id, GLuint* fbo_buffs, int size)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArray(loc);
    glVertexPointer(2, GL_FLOAT, 0, vertex_coord[id]); 
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coord);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, tex_coord);    
    if (0 != fbo_buffs)
    {
        glDrawBuffers(size, fbo_buffs); 
    }
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(loc);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Gl_widget::plote_histogram(int *hist_max)
{
    int pos[] = {0, 100, 200};
    GLfloat color[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    int max_value = (hist_max[0] < hist_max[1])?hist_max[1]:hist_max[0];
    if (max_value < hist_max[2])
    {
        max_value = hist_max[2];
    }

    for (int i = 0; i < 3; i++)
    {
        glViewport(0, pos[i], width(), height()/4);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(+0.0f, +256.0f, 0.0f, max_value, +0.0f, +1.0f);
        glMatrixMode(GL_MODELVIEW);

        glBindBuffer(GL_ARRAY_BUFFER, hist_obj[i]);
        glColor3fv(&color[3*i]);
        glVertexPointer(2, GL_INT, 0, 0);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_LINE_STRIP, 0, 256);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Gl_widget::paintGL()
{
    if (0 == yuv_data)
    {
        return;
    }

	glViewport(0, 0, width(), height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0f, +1.0f, -1.0f, +1.0f, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v_width, v_height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, yuv_data);
    glActiveTexture(GL_TEXTURE1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), v_height, GL_RGBA, GL_UNSIGNED_BYTE, yuv_data);

    int hist_max[3] = {1, 1, 1};
    compute_histogram(texture, hist_obj, hist_max, v_width, v_height);

    yuy2_filter.bind();
    yuy2_filter.setUniformValue("texture_0", 0);
    yuy2_filter.setUniformValue("texture_1", 1);
    render_quad(yuy2_filter.attributeLocation("inTexCoord"), 4, 0, 0);
    yuy2_filter.release();

    plote_histogram(hist_max);

    glFinish();	
}

void Gl_widget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, +1.0f, -1.0f, +1.0f, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Gl_widget::render_frame(uint8_t* data)
{
    mutex.lock();
    yuv_data = data;
    mutex.unlock();
    repaint();
}

void Gl_widget::closeEvent(QCloseEvent* event)
{
    (void)event;
    yuv_data = 0;
    thread.stop();
}

