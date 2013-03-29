#include <gl_widget.h>

#include <stdint.h>

#include <QtGui>
#include <QPixmap>

const GLfloat Gl_widget::tex_coord[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
const GLfloat Gl_widget::vertex_coord[][8] = {{-1.0f, +1.0f, +0.0f, +1.0f, +0.0f, +0.0f, -1.0f, +0.0f},
                                              {+0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +0.0f},
                                              {-1.0f, +0.0f, +0.0f, +0.0f, +0.0f, -1.0f, -1.0f, -1.0f},
                                              {+0.0f, +0.0f, +1.0f, +0.0f, +1.0f, -1.0f, +0.0f, -1.0f},
                                              {-1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, -1.0f}};

Gl_widget::Gl_widget(int w, int h, const QString& path, QGLFormat& fmt, QWidget* parent)
	:QGLWidget(fmt, parent)
    , fb_id(0)
    , buff_id(0)
    , prev_id(0)
    , min_thr(25)
    , max_thr(60)
    , v_width(w)
    , v_height(h)
    , nmes_filter(this)
    , gray_filter(this)
    , edge_filter(this)
    , yuv420_filter(this)
    , binary_filter(this)
    , smooth_filter(this)
    , timer(this)
    , offline(w, h, path, this, this)
{
    setWindowTitle("Edge: "+path);
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	delete_textures();
}

bool Gl_widget::init_yuv420_shader()
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

bool Gl_widget::init_gaussian_shader()
{
    smooth_filter.bindAttributeLocation("inVertex", 0);
    smooth_filter.bindAttributeLocation("inTexCoord", 1);
    smooth_filter.bindAttributeLocation("texCoord", 2);
    smooth_filter.bindAttributeLocation("fragColor", 3);

	bool status = smooth_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && smooth_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/gauss_smooth.fs");

    glBindFragDataLocation(smooth_filter.programId(), 0, "fragColor");
    status = status && smooth_filter.link();

    return status;
}

bool Gl_widget::init_gray_shader()
{
    gray_filter.bindAttributeLocation("inVertex", 0);
    gray_filter.bindAttributeLocation("inTexCoord", 1);
    gray_filter.bindAttributeLocation("texCoord", 2);
    gray_filter.bindAttributeLocation("fragColor", 3);

	bool status = gray_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && gray_filter.addShaderFromSourceFile(QGLShader::Fragment,"shader/gray.fs");

    glBindFragDataLocation(gray_filter.programId(), 0, "fragColor");
    status = status && gray_filter.link();  

    return status;
}

bool Gl_widget::init_edge_shader()
{
    edge_filter.bindAttributeLocation("inVertex", 0);
    edge_filter.bindAttributeLocation("inTexCoord", 1);
    edge_filter.bindAttributeLocation("texCoord", 2);
    edge_filter.bindAttributeLocation("fragColor", 3);

	bool status = edge_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && edge_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/edge.fs");

    glBindFragDataLocation(edge_filter.programId(), 0, "fragColor");
    status = status && edge_filter.link(); 
    return status;
}

bool Gl_widget::init_nmes_shader()
{
    nmes_filter.bindAttributeLocation("inVertex", 0);
    nmes_filter.bindAttributeLocation("inTexCoord", 1);
    nmes_filter.bindAttributeLocation("texCoord", 2);
    nmes_filter.bindAttributeLocation("fragColor", 3);

	bool status = nmes_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && nmes_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/nmes.fs");

    glBindFragDataLocation(nmes_filter.programId(), 0, "fragColor");
    status = status && nmes_filter.link(); 
    return status;
}

bool Gl_widget::init_binary_shader()
{
    binary_filter.bindAttributeLocation("inVertex", 0);
    binary_filter.bindAttributeLocation("inTexCoord", 1);
    binary_filter.bindAttributeLocation("texCoord", 2);
    binary_filter.bindAttributeLocation("fragColor", 3);

	bool status = binary_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && binary_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/binary.fs");

    glBindFragDataLocation(binary_filter.programId(), 0, "fragColor");
    status = status && binary_filter.link(); 
    return status;
}

void Gl_widget::init_shaders()
{
    bool status = init_yuv420_shader();
    status = status && init_gaussian_shader();
    status = status && init_gray_shader();
    status = status && init_edge_shader();
    status = status && init_nmes_shader();
    status = status && init_binary_shader();
    if (false == status)
    {
        exit(0);
    }
    program[0] = &smooth_filter;
    program[1] = &edge_filter;
    program[2] = &nmes_filter;
    program[3] = &binary_filter;
}

void Gl_widget::initializeGL()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        qDebug() << "Error: glewInit() failed\n";
        exit(0);
    }

    glGenFramebuffers(1, &fb_id);

    init_shaders();
    init_textures();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    timer.setInterval(42);
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
    timer.start();
}

void Gl_widget::init_textures()
{
    int t_width[] = {v_width, v_width>>1, v_width>>1, v_width,
                       v_width, v_width, v_width};
    int t_height[] = {v_height, v_height>>1, v_height>>1, v_height,
                       v_height, v_height, v_height};
    int t_int_fmt[] = {GL_LUMINANCE, GL_LUMINANCE, GL_LUMINANCE, GL_RGBA16F,
                      GL_RGBA16F, GL_RGBA16F, GL_RGBA16F};
    int t_fmt[] = {GL_LUMINANCE, GL_LUMINANCE, GL_LUMINANCE, 
                     GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA};
    int t_type[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT,
                      GL_FLOAT, GL_FLOAT};
    
    glGenTextures(7, texture);
    for (int i = 0; i < 7; i++)
    {
        glActiveTexture(GL_TEXTURE0+i); 
        glBindTexture(GL_TEXTURE_2D, texture[LUMA+i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, t_int_fmt[i], t_width[i], t_height[i], 0, t_fmt[i], t_type[i], 0);
    }
}

void Gl_widget::delete_textures()
{
    glDeleteTextures(7, texture);
}

void Gl_widget::render_to_texture()
{
    GLenum fbo_buffs[] = {GL_COLOR_ATTACHMENT0};

	glViewport(0, 0, v_width, v_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, +1.0f, -1.0f, +1.0f, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
	glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v_width, v_height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    for (int i = STAGE_GAUSS_SMOOTH; i <= STAGE_BINARY; i++)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[SMOOTH+i], 0);

        program[i]->bind();
        switch (i)
        {
            case STAGE_GAUSS_SMOOTH:
                program[i]->setUniformValue("texture_0", 0);
                break;
            case STAGE_EDGE:
                program[i]->setUniformValue("texture_0", 3);
                break;
            case STAGE_NMES:
                program[i]->setUniformValue("texture_0", 4);
                break;
            case STAGE_BINARY:
                program[i]->setUniformValue("texture_0", 5);
                program[i]->setUniformValue("min", ((GLfloat)min_thr)/255.0f);
                program[i]->setUniformValue("max", ((GLfloat)max_thr)/255.0f);
                break;
        }
        render_quad(program[i]->attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
        program[i]->release();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
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
 
    render_to_texture();

    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, +1.0, -1.0, +1.0, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    int u_offset = (v_width*v_height);
    int v_offset = u_offset+(u_offset>>2);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
    glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v_width, v_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    glActiveTexture(GL_TEXTURE1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), (v_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)u_offset);
    glActiveTexture(GL_TEXTURE2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), (v_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)v_offset);

    yuv420_filter.bind();
    yuv420_filter.setUniformValue("texture_0", 0);
    yuv420_filter.setUniformValue("texture_1", 1);
    yuv420_filter.setUniformValue("texture_2", 2);
    render_quad(yuv420_filter.attributeLocation("inTexCoord"), 0, 0, 0);
    yuv420_filter.release();

    for (int i = 0; i < 3; i++)
    {
        gray_filter.bind();
        gray_filter.setUniformValue("texture_0", i+4);
        render_quad(gray_filter.attributeLocation("inTexCoord"), i+1, 0, 0);
        gray_filter.release();
    }
	glFinish();
	
}

void Gl_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
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

void Gl_widget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Up:
            if (max_thr < 255)
            {
                ++max_thr;
            }
            break;
        case Qt::Key_Down:
            if (max_thr > 1 && min_thr < max_thr)
            {
                --max_thr;
            }
            break;
        case Qt::Key_Left:
            if (min_thr > 1)
            {
                --min_thr;
            }
            break;
        case Qt::Key_Right:
            if (min_thr < 255 && min_thr < max_thr)
            {
                ++min_thr;
            }
            break;
    }
    printf("\nThresholds, min: %d, max: %d", min_thr, max_thr);
    fflush(stdout);
}

