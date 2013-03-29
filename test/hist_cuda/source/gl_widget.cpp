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
                                              {-1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, -1.0f}};

Gl_widget::Gl_widget(int w, int h, const QString& path, QGLFormat& fmt, QWidget* parent)
	:QGLWidget(fmt, parent)
    , buff_id(0)
    , prev_id(0)
    , v_width(w)
    , v_height(h)
    , yuv420_filter(this)
    , timer(this)
    , offline(w, h, path, this, this)
{
    setWindowTitle("Edge: "+path);
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	delete_textures();
    glDeleteBuffers(3, hist_obj); 
}

bool Gl_widget::init_shader()
{
    yuv420_filter.bindAttributeLocation("inVertex", 0);
    yuv420_filter.bindAttributeLocation("inTexCoord", 1);
    yuv420_filter.bindAttributeLocation("texCoord", 2);
    yuv420_filter.bindAttributeLocation("fragColor", 3);

	bool status = yuv420_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && yuv420_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/yuv_p_rgb.fs");
    
    glBindFragDataLocation(yuv420_filter.programId(), 0, "fragColor");
    status = status && yuv420_filter.link();

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
    glGenBuffers(3, hist_obj);    
    for (int i = 0; i < 3; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, hist_obj[i]);
        glBufferData(GL_ARRAY_BUFFER, 256*2*sizeof(GL_INT), 0, GL_DYNAMIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    timer.setInterval(42);
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
    timer.start();
}

void Gl_widget::init_textures()
{
    int t_width[] = {v_width, v_width>>1, v_width>>1};
    int t_height[] = {v_height, v_height>>1, v_height>>1};
    
    glGenTextures(3, texture);
    for (int i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE0+i); 
        glBindTexture(GL_TEXTURE_2D, texture[LUMA+i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, t_width[i], t_height[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    }
}

void Gl_widget::delete_textures()
{
    glDeleteTextures(3, texture);
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

void Gl_widget::paintGL()
{
    if (0 == prev_id || 0 == buff_id)
    {
        return;
    }

    int u_offset = (v_width*v_height);
    int v_offset = u_offset+(u_offset>>2);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
    glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v_width, v_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    glActiveTexture(GL_TEXTURE1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), (v_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)u_offset);
    glActiveTexture(GL_TEXTURE2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), (v_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)v_offset);

    int max[3];
    compute_histogram(texture, hist_obj, max, v_width, v_height);

    yuv420_filter.bind();
    yuv420_filter.setUniformValue("texture_0", 0);
    yuv420_filter.setUniformValue("texture_1", 1);
    yuv420_filter.setUniformValue("texture_2", 2);
    render_quad(yuv420_filter.attributeLocation("inTexCoord"), 4, 0, 0);
    yuv420_filter.release();
	glFinish();
	   
    max[0] = (max[0]*3)>>1;
    max[1] = (max[1]*3)>>1;
    max[2] = (max[2]*3)>>1;

    GLfloat color[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    for (int i = 0; i < 3; i++)
    {
	    glViewport(0, 0, width(), height());
        glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, 256.0, 0.0, max[i], 0.0, 1.0);
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

void Gl_widget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, +1.0, -1.0, +1.0, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Gl_widget::time_out()
{
    bool flag = offline.isRenderBufferEmpty();
    buff_id = flag?prev_id:offline.renderBuffer();
    if (0 != prev_id && false == flag)
    {
        offline.addToFreeQueue(prev_id);
    }
    prev_id = buff_id;
    updateGL();
}

void Gl_widget::closeEvent(QCloseEvent* event)
{
    (void)event;
    offline.stop_thread();
}


