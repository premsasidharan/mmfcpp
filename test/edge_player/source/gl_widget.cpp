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

Gl_widget::Gl_widget(int width, int height, const QString& path, QGLFormat& fmt, QWidget* parent)
	:QGLWidget(fmt, parent)
    , fb_id(0)
    , buff_id(0)
    , prev_id(0)
    , video_width(width)
    , video_height(height)
    , y_texture(0)
    , u_texture(0)
    , v_texture(0)
    , nmes_texture(0)
    , edge_texture(0)
    , smooth_texture(0)
    , nmes_filter(this)
    , gray_filter(this)
    , edge_filter(this)
    , yuv420_filter(this)
    , binary_filter(this)
    , smooth_filter(this)
    , timer(this)
    , offline(width, height, path, this, this)
{
    setWindowTitle("Edge: "+path);
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	delete_yuv_textures();
}

bool Gl_widget::init_yuv420_shader()
{
    yuv420_filter.bindAttributeLocation("inVertex", 0);
    yuv420_filter.bindAttributeLocation("inTexCoord", 1);
    yuv420_filter.bindAttributeLocation("texCoord", 2);
    yuv420_filter.bindAttributeLocation("fragColor", 3);

	bool status = yuv420_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

	status = status && yuv420_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D texture_0;\n"
        "uniform sampler2D texture_1;\n"
        "uniform sampler2D texture_2;\n"
        "void main(void)\n"
        "{\n"
        "    float y, u, v, red, green, blue;\n"	
        "    y = texture2D(texture_0, texCoord.st).r;\n"
        "    u = texture2D(texture_1, texCoord.st).r-0.5;\n"
        "    v = texture2D(texture_2, texCoord.st).r-0.5;\n"
        "    y = 1.1643*(y-0.0625);\n"
        "    red = y+1.5958*v;\n"
        "    green = y-0.39173*u-0.81290*v;\n"
        "    blue = y+2.017*u;\n"
        "    fragColor = vec4(red, green, blue, 1.0);\n"
        "}");
    
    glBindFragDataLocation(yuv420_filter.programId(), 0, "fragColor");
    status = status && yuv420_filter.link();

    return status;
}

bool Gl_widget::init_smoothing_shader()
{
    smooth_filter.bindAttributeLocation("inVertex", 0);
    smooth_filter.bindAttributeLocation("inTexCoord", 1);
    smooth_filter.bindAttributeLocation("texCoord", 2);
    smooth_filter.bindAttributeLocation("fragColor", 3);

	bool status = smooth_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

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

	bool status = gray_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

	status = status && gray_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D texture_0;\n"
        "void main(void)\n"
        "{\n"
        "    float y = texture2D(texture_0, texCoord.st).r;\n"
        "    fragColor = vec4(y, y, y, 1.0);\n"
        "}");


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

	bool status = edge_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

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

	bool status = nmes_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

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

	bool status = binary_filter.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

	status = status && binary_filter.addShaderFromSourceFile(QGLShader::Fragment, "shader/binary.fs");

    glBindFragDataLocation(binary_filter.programId(), 0, "fragColor");
    status = status && binary_filter.link(); 
    return status;
}

void Gl_widget::init_shaders()
{
    bool status = init_yuv420_shader();
    status = status && init_smoothing_shader();
    status = status && init_gray_shader();
    status = status && init_edge_shader();
    status = status && init_nmes_shader();
    status = status && init_binary_shader();
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
    init_yuv_textures();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    timer.setInterval(42);
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
    timer.start();
}

void Gl_widget::init_yuv_textures()
{
    glGenTextures(1, &y_texture);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_width, video_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glGenTextures(1, &u_texture);
    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (video_width>>1), (video_height>>1), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glGenTextures(1, &v_texture);
    glActiveTexture(GL_TEXTURE2); 
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (video_width>>1), (video_height>>1), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glGenTextures(1, &smooth_texture);
    glActiveTexture(GL_TEXTURE3); 
    glBindTexture(GL_TEXTURE_2D, smooth_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, video_width, video_height, 0, GL_RGBA, GL_FLOAT, 0);

    glGenTextures(1, &edge_texture);
    glActiveTexture(GL_TEXTURE4); 
    glBindTexture(GL_TEXTURE_2D, edge_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, video_width, video_height, 0, GL_RGBA, GL_FLOAT, 0);

    glGenTextures(1, &nmes_texture);
    glActiveTexture(GL_TEXTURE5); 
    glBindTexture(GL_TEXTURE_2D, nmes_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, video_width, video_height, 0, GL_RGBA, GL_FLOAT, 0);

    glGenTextures(1, &binary_texture);
    glActiveTexture(GL_TEXTURE6); 
    glBindTexture(GL_TEXTURE_2D, binary_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, video_width, video_height, 0, GL_RGBA, GL_FLOAT, 0);
}

void Gl_widget::delete_yuv_textures()
{
    glDeleteTextures(1, &y_texture);
    glDeleteTextures(1, &u_texture);
    glDeleteTextures(1, &v_texture);
    glDeleteTextures(1, &edge_texture);
    glDeleteTextures(1, &nmes_texture);
    glDeleteTextures(1, &binary_texture);
    glDeleteTextures(1, &smooth_texture);
}

void Gl_widget::render_to_texture()
{
    GLenum fbo_buffs[] = {GL_COLOR_ATTACHMENT0};

	glViewport(0, 0, video_width, video_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1, 1.0, 0.0f, 0.0f);

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smooth_texture, 0);

    smooth_filter.bind();
    smooth_filter.setUniformValue("texture_0", 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
	glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    render_quad(smooth_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    smooth_filter.release();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, edge_texture, 0);

    edge_filter.bind();
    edge_filter.setUniformValue("texture_0", 3);
    render_quad(edge_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    edge_filter.release();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nmes_texture, 0);

    nmes_filter.bind();
    nmes_filter.setUniformValue("texture_0", 4);
    render_quad(nmes_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    nmes_filter.release();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, binary_texture, 0);

    binary_filter.bind();
    binary_filter.setUniformValue("texture_0", 5);
    binary_filter.setUniformValue("min", (GLfloat)(30.0/255.0));
    binary_filter.setUniformValue("max", (GLfloat)(80.0/255.0));
    render_quad(binary_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    binary_filter.release();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 
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
    if (0 == prev_id)
    {
        glBegin(GL_QUADS);
            glVertex2f(-1.0f, 1.0f);    // Top Left
            glVertex2f(1.0f, 1.0f);     // Top Right
            glVertex2f(1.0f, -1.0f);    // Bottom Right
            glVertex2f(-1.0f, -1.0f);   // Bottom Left
        glEnd();
        return;
    }

    if (0 != buff_id)
    {
        int pos = (video_width*video_height);
        render_to_texture();

        glViewport(0, 0, width(), height());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1, 1.0, 0.0f, 0.0f);

        //Initialize modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

	    yuv420_filter.bind();
        yuv420_filter.setUniformValue("texture_0", 0);
        yuv420_filter.setUniformValue("texture_1", 1);
        yuv420_filter.setUniformValue("texture_2", 2);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
	    glActiveTexture(GL_TEXTURE0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
	    glActiveTexture(GL_TEXTURE1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (video_width>>1), (video_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)pos);
        pos += (pos >> 2);
	    glActiveTexture(GL_TEXTURE2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (video_width>>1), (video_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)pos);
        render_quad(yuv420_filter.attributeLocation("inTexCoord"), 0, 0, 0);
	    yuv420_filter.release();

        for (int i = 0; i < 3; i++)
        {
            gray_filter.bind();
            gray_filter.setUniformValue("texture_0", i+4);
            render_quad(gray_filter.attributeLocation("inTexCoord"), i+1, 0, 0);
	        gray_filter.release();
        }
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

