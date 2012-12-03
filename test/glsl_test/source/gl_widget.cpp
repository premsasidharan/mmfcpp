#include <gl_widget.h>

#include <stdint.h>

#include <QtGui>
#include <QPixmap>

Gl_widget::Gl_widget(int width, int height, const QString& path, QGLFormat& fmt, QWidget* parent)
	:QGLWidget(fmt, parent)
    , video_width(width)
    , video_height(height)
    , prev_id(0)
    , y_texture(0)
    , u_texture(0)
    , v_texture(0)
    , timer(this)
    , program(this)
    , offline(width, height, path, this, this)
{
    glewInit();
	setFocusPolicy(Qt::StrongFocus);
}

Gl_widget::~Gl_widget()
{
	//delete_yuv_textures();
}

void Gl_widget::initializeGL()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        qDebug() << "Error: glewInit() failed\n";
        exit(0);
    }

    init_yuv_textures();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    program.bindAttributeLocation("inVertex", 0);
    program.bindAttributeLocation("texCoord", 1);
    program.bindAttributeLocation("texCoord", 2);
    program.bindAttributeLocation("fragColor", 3);

	bool status = program.addShaderFromSourceCode(QGLShader::Vertex,
		"#version 130\n"
        "in vec4 inVertex;\n"
        "in vec2 inTexCoord;\n"
        "out vec4 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = inVertex;\n"
        "    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
        "}");

	status = status && program.addShaderFromSourceCode(QGLShader::Fragment,
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

    if (false == status)
    {
        exit(0);
    }
    
    glBindFragDataLocation(program.programId(), 0, "fragColor");
    qDebug() << program.link();  

    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    timer.setInterval(42);
    timer.start();
}

void Gl_widget::init_yuv_textures()
{
    glGenTextures(1, &y_texture);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_width, video_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glGenTextures(1, &u_texture);
    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (video_width>>1), (video_height>>1), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

    glGenTextures(1, &u_texture);
    glActiveTexture(GL_TEXTURE2); 
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (video_width>>1), (video_height>>1), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
}

void Gl_widget::delete_yuv_textures()
{
    glDeleteTextures(1, &y_texture);
    glDeleteTextures(1, &u_texture);
    glDeleteTextures(1, &v_texture);
}

void Gl_widget::paintGL()
{
    bool flag = offline.isRenderBufferEmpty();
    GLuint buff_id = flag?prev_id:offline.renderBuffer();

    if (0 == prev_id && true == flag)
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
	    program.bind();
        program.setUniformValue("texture_0", 0);
        program.setUniformValue("texture_1", 1);
        program.setUniformValue("texture_2", 2);

        int loc = program.attributeLocation("inTexCoord");

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
	    glActiveTexture(GL_TEXTURE0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
        
        int index = video_width*video_height;
	    glActiveTexture(GL_TEXTURE1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (video_width>>1), (video_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)index);
        
        index += ((video_width*video_height)>>2);
        glActiveTexture(GL_TEXTURE2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (video_width>>1), (video_height>>1), GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)index);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
       
	    glClear(GL_COLOR_BUFFER_BIT);
        
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
            glVertexAttrib2f(loc, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);       // Top Left
            glVertexAttrib2f(loc, 1.0f, 0.0f); glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);        // Top Right
            glVertexAttrib2f(loc, 1.0f, 1.0f); glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);       // Bottom Right
            glVertexAttrib2f(loc, 0.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);      // Bottom Left
        glEnd(); 
        glDisable(GL_TEXTURE_2D);
	    program.release();
    }
	glFinish();

    if (0 != prev_id && false == flag)
    {
        offline.addToFreeQueue(prev_id);
    }

    prev_id = buff_id;
	
}

void Gl_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
}

void Gl_widget::on_timeout()
{
    updateGL();
}

