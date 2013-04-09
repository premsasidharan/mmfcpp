#include <gl_widget.h>

#include <stdint.h>

#include <QtGui>
#include <QPixmap>

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
    , fb_id(0)
    , v_width(w)
    , v_height(h)
    , threshold(20)
    , yuv_data(0)
    , nms_filter(this)
    , diff_filter(this)
    , thresh_filter(this)
    , smooth_filter(this)
    , adjust_filter(this)
    , rgb_filter(this)
    , yuy2_filter(this)
    , thread(w, h, path)
{
    setWindowTitle("Corner detection: "+path);
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	delete_textures();
}

bool Gl_widget::init_yuy2_shader()
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

bool Gl_widget::init_adjust_shader()
{
    adjust_filter.bindAttributeLocation("inVertex", 0);
    adjust_filter.bindAttributeLocation("inTexCoord", 1);
    adjust_filter.bindAttributeLocation("texCoord", 2);
    adjust_filter.bindAttributeLocation("fragColor", 3);

	bool status = adjust_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && adjust_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/adjust.fs");

    glBindFragDataLocation(adjust_filter.programId(), 0, "fragColor");
    status = status && adjust_filter.link();

    return status;
}

bool Gl_widget::init_rgb_shader()
{
    rgb_filter.bindAttributeLocation("inVertex", 0);
    rgb_filter.bindAttributeLocation("inTexCoord", 1);
    rgb_filter.bindAttributeLocation("texCoord", 2);
    rgb_filter.bindAttributeLocation("fragColor", 3);

	bool status = rgb_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && rgb_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/rgb.fs");

    glBindFragDataLocation(rgb_filter.programId(), 0, "fragColor");
    status = status && rgb_filter.link();  

    return status;
}

bool Gl_widget::init_diff_shader()
{
    diff_filter.bindAttributeLocation("inVertex", 0);
    diff_filter.bindAttributeLocation("inTexCoord", 1);
    diff_filter.bindAttributeLocation("texCoord", 2);
    diff_filter.bindAttributeLocation("fragColor", 3);

	bool status = diff_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && diff_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/diff.fs");

    glBindFragDataLocation(diff_filter.programId(), 0, "fragColor");
    status = status && diff_filter.link(); 
    return status;
}

bool Gl_widget::init_nms_shader()
{
    nms_filter.bindAttributeLocation("inVertex", 0);
    nms_filter.bindAttributeLocation("inTexCoord", 1);
    nms_filter.bindAttributeLocation("texCoord", 2);
    nms_filter.bindAttributeLocation("fragColor", 3);

	bool status = nms_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && nms_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/nms.fs");

    glBindFragDataLocation(nms_filter.programId(), 0, "fragColor");
    status = status && nms_filter.link(); 
    return status;
}

bool Gl_widget::init_thresh_shader()
{
    thresh_filter.bindAttributeLocation("inVertex", 0);
    thresh_filter.bindAttributeLocation("inTexCoord", 1);
    thresh_filter.bindAttributeLocation("texCoord", 2);
    thresh_filter.bindAttributeLocation("fragColor", 3);

	bool status = thresh_filter.addShaderFromSourceFile(QGLShader::Vertex, "shader/vertex.vs");
	status = status && thresh_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/eigen.fs");

    glBindFragDataLocation(thresh_filter.programId(), 0, "fragColor");
    status = status && thresh_filter.link(); 
    return status;
}

void Gl_widget::init_shaders()
{
    bool status = init_yuy2_shader();
    status = status && init_gray_shader();
    status = status && init_rgb_shader();

    status = status && init_diff_shader();
    status = status && init_nms_shader();
    status = status && init_thresh_shader();
    status = status && init_gaussian_shader();
    status = status && init_adjust_shader();

    program[0] = &diff_filter;
    program[1] = &smooth_filter;
    program[2] = &thresh_filter;
    program[3] = &nms_filter;
    program[4] = &adjust_filter;

    if (false == status)
    {
        exit(0);
    }
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

    connect(&thread, SIGNAL(update_widget(uint8_t*)), this, SLOT(render_frame(uint8_t*)));

    thread.start();
}

void Gl_widget::init_textures()
{
    int t_width[] = {v_width, v_width>>1, v_width, v_width, v_width, v_width, v_width};
    int t_int_fmt[] = {GL_LUMINANCE_ALPHA, GL_RGBA, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F};
    int t_fmt[] = {GL_LUMINANCE_ALPHA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA};
    int t_type[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT};

    glGenTextures(7, texture);
    for (int i = 0; i < 7; i++)
    {
        glActiveTexture(GL_TEXTURE0+i); 
        glBindTexture(GL_TEXTURE_2D, texture[LUMA+i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, t_int_fmt[i], t_width[i], v_height, 0, t_fmt[i], t_type[i], 0);
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

    glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v_width, v_height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, yuv_data);
    glActiveTexture(GL_TEXTURE1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (v_width>>1), v_height, GL_RGBA, GL_UNSIGNED_BYTE, yuv_data);

    for (int i = STAGE_DIFF; i <= STAGE_ADJUST; i++)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[X2_Y2_XY+i], 0);

        program[i]->bind();
        switch (i)
        { 
            case STAGE_DIFF:
                program[i]->setUniformValue("texture_0", 0);
                break;
            case STAGE_GAUSS_SMOOTH:
                program[i]->setUniformValue("texture_0", 2);
                break;
            case STAGE_THRESHOLD:
                program[i]->setUniformValue("texture_0", 3);
                break;
            case STAGE_NMS:
                program[i]->setUniformValue("texture_0", 4);
                program[i]->setUniformValue("threshold", ((float)threshold)/255.0f);
                break;
            case STAGE_ADJUST:
                program[i]->setUniformValue("texture_0", 5);
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
    if (0 == yuv_data)
    {
        return;
    }

    render_to_texture();

    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, +1.0f, -1.0f, +1.0f, 0.0f, +1.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    yuy2_filter.bind();
    yuy2_filter.setUniformValue("texture_0", 0);
    yuy2_filter.setUniformValue("texture_1", 1);
    render_quad(yuy2_filter.attributeLocation("inTexCoord"), 4, 0, 0);
    yuy2_filter.release();

    /*for (int i = 0; i < 3; i++)
    {
        rgb_filter.bind();
        rgb_filter.setUniformValue("texture_0", i+4);
        render_quad(rgb_filter.attributeLocation("inTexCoord"), 1+i, 0, 0);
        rgb_filter.release();
    }*/

    glEnable(GL_BLEND);	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rgb_filter.bind();
    rgb_filter.setUniformValue("texture_0", 6);
    render_quad(rgb_filter.attributeLocation("inTexCoord"), 4, 0, 0);
    rgb_filter.release();
    glDisable(GL_BLEND);
    glFinish();
}

void Gl_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
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

void Gl_widget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Up:
            ++threshold;
            break;
        case Qt::Key_Down:
            --threshold;
            break;
    }
    printf("\nThreshold: %d (%f)", threshold, ((float)threshold)/255.0f);
    fflush(stdout);
}
