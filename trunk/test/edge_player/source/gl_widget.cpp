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

const GLfloat Gl_widget::coeffs_fx[] = {-1.0f, 0.0f, 1.0f, -2.0f, 0.0f, 2.0f, -1.0f, 0.0f, 1.0f};
const GLfloat Gl_widget::coeffs_fy[] = {1.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, -2.0f, -1.0f};
const GLfloat Gl_widget::gauss_coeffs[] = {(2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0), 
                                           (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                                           (5.0/159.0), (12.0/159.0), (15.0/159.0), (12.0/159.0), (5.0/159.0),
                                           (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                                           (2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0)};

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

	status = status && smooth_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform vec2 coord[25];\n"
        "uniform float coeffs[25];\n"
        "uniform sampler2D texture_0;\n"
        "void main(void)\n"
        "{\n"
        "    float y = 0.0f;\n"
        "    vec2 pos = texCoord.st;\n"
        "    vec2 current = vec2(pos.x, (1.0f-pos.y));\n"
        "    for (int i = 0; i < coord.length(); i++) {\n"
        "        pos = current+coord[i];\n"
        "        y += (coeffs[i]*texture2D(texture_0, pos).r);\n"
        "    }\n"
        "    fragColor = vec4(y, y, y, 1.0);\n"
        "}");

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

	status = status && edge_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform vec2 offset[9];\n"
        "uniform float coeffs_fx[9];\n"
        "uniform float coeffs_fy[9];\n"
        "uniform sampler2D texture_0;\n"
        "const float pi = 3.14159265358979323846;\n"
        "const float pi_mul_2 = (pi*2.0);\n"
        "const float theta_360 = (pi*2.0);\n"
        "const float theta_22_5 = (pi/8.0);\n"
        "const float theta_67_5 = ((3.0*pi)/8.0);\n"
        "const float theta_112_5 = ((5.0*pi)/8.0);\n"
        "const float theta_157_5 = ((7.0*pi)/8.0);\n"
        "const float theta_202_5 = ((9.0*pi)/8.0);\n"
        "const float theta_247_5 = ((11.0*pi)/8.0);\n"
        "const float theta_292_5 = ((13.0*pi)/8.0);\n"
        "const float theta_337_5 = ((15.0*pi)/8.0);\n"
        "void main(void)\n"
        "{\n"
        "    float theta;\n"
        "    float y = 0.0f, gx = 0.0f, gy = 0.0f;\n"
        "    vec2 current = vec2(texCoord.s, (1.0f-texCoord.t));\n"
        "    for (int i = 0; i < offset.length(); i++) {\n"
        "        y = texture2D(texture_0, (current+offset[i])).r;\n"
        "        gx += (y*coeffs_fx[i]);\n"
        "        gy += (y*coeffs_fy[i]);\n"
        "    }\n"
		"	 y = sqrt((gx*gx)+(gy*gy));\n"
        "    theta = atan(gy, gx);\n"
        "    if ((theta >= theta_22_5 && theta < theta_67_5)||(theta >= theta_202_5 && theta < theta_247_5))\n"
        "        theta = 0.25f;\n"
        "    else if ((theta >= theta_67_5 && theta < theta_112_5)||(theta >= theta_247_5 && theta < theta_292_5))\n"
        "        theta = 0.5f;\n"
        "    else if ((theta >= theta_112_5 && theta < theta_157_5)||(theta >= theta_292_5 && theta < theta_337_5))\n"
        "        theta = 0.75f;\n"
        "    else\n"
        "        theta = 0.0f;\n"
        "    fragColor = vec4(y, theta, 0.0f, 1.0f);\n"
        "}");

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

	status = status && nmes_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
		"#define ne_dir 0\n"
		"#define n_dir  1\n"
		"#define nw_dir 2\n"
		"#define e_dir  3\n"
		"#define w_dir  5\n"
		"#define se_dir 6\n"
		"#define s_dir  7\n"
		"#define sw_dir 8\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform vec2 offset[9];\n"
        "uniform sampler2D texture_0;\n"
        "const int first[]=int[](e_dir, ne_dir, n_dir, se_dir);\n"
        "const int second[]=int[](w_dir, sw_dir, s_dir, nw_dir);\n"
        "void main(void)\n"
        "{\n"
        "    vec2 current = vec2(texCoord.s, 1.0f-texCoord.t);\n"
        "    float color = texture2D(texture_0, current).r;\n"
        "    int i = int(ceil(4.0*texture2D(texture_0, current).g));\n"
        "    vec2 pos1 = current+offset[first[i]];\n"
        "    vec2 pos2 = current+offset[second[i]];\n"
        "    if ((color <= texture2D(texture_0, pos1).r) || (color <= texture2D(texture_0, pos2).r))\n"
        "        color = 0.0f;\n"
        "    fragColor = vec4(color, color, color, 1.0f);\n"
        "}");

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

	status = status && binary_filter.addShaderFromSourceCode(QGLShader::Fragment,
		"#version 130\n"
        "in vec4 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform float min;\n"
        "uniform float max;\n"
		"uniform vec2 coord[8];\n"
        "uniform sampler2D texture_0;\n"
        "void main(void)\n"
        "{\n"
        "    vec2 current = vec2(texCoord.s, 1.0-texCoord.t);\n"
        "    float color = texture2D(texture_0, current).r;\n"
        "    if (color >= max)\n"
        "        color = 1.0;\n"
        "    else if (color >= min) {\n"
		"        int i;\n"
		"        vec2 pos;\n"
        "        bool flag = true;\n"
		"        for (i = 0; i < 8 && flag; i++) {\n"
        "            pos = current+coord[i];\n"
		"            if (max >= texture2D(texture_0, pos).r)\n"
		"               flag = false;\n"
		"		}\n"
		"       color = (flag)?0.0f:1.0f;\n"
        "    } else\n"
        "       color = 0.0f;\n"
        "    fragColor = vec4(color, color, color, 1.0f);\n"
        "}");

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
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
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
	GLfloat fx = 1.0/(float)video_width;
	GLfloat fy = 1.0/(float)video_height;   
    GLenum fbo_buffs[] = {GL_COLOR_ATTACHMENT0};

    GLfloat coords[] = {-2*fx, 2*fy, -fx, 2*fy, 0.0, 2*fy, fx, 2*fy, 2*fx, 2*fy, 
                        -2*fx, fy, -fx, fy, 0.0, fy, fx, fy, 2*fx, fy, 
                        -2*fx, 0.0, -fx, 0.0, 0.0, 0.0, fx, 0.0, 2*fx, 0.0, 
                        -2*fx, -fy, -fx, -fy, 0.0, -fy, fx, -fy, 2*fx, -fy,
                        -2*fx, -2*fy, -fx, -2*fy, 0.0, -2*fy, fx, -2*fy, 2*fx, -2*fy};

	GLfloat nbhrs[] = {-fx, fy, 0.0f, fy, fx, fy, -fx, 0.0f, fx, 0.0f, -fx, -fy, 0.0, -fy, fx, -fy};
 	GLfloat nghbr_coords[] = {-fx, fy, 0.0f, fy, fx, fy, -fx, 0.0f, 0.0f, 0.0f, fx, 0.0f, -fx, -fy, 0.0f, -fy, fx, -fy};

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
    smooth_filter.setUniformValueArray("coeffs", gauss_coeffs, 25, 1); 
    smooth_filter.setUniformValueArray("coords", coords, 25, 2); 

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    render_quad(smooth_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    smooth_filter.release();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, edge_texture, 0);

    edge_filter.bind();
    edge_filter.setUniformValue("texture_0", 3);
    edge_filter.setUniformValueArray("coeffs_fx", coeffs_fx, 9, 1); 
    edge_filter.setUniformValueArray("coeffs_fy", coeffs_fy, 9, 1); 
    edge_filter.setUniformValueArray("offset", nghbr_coords, 9, 2);
	glActiveTexture(GL_TEXTURE3);
    render_quad(edge_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    edge_filter.release();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nmes_texture, 0);

    nmes_filter.bind();
    nmes_filter.setUniformValue("texture_0", 4);
    nmes_filter.setUniformValueArray("offset", nghbr_coords, 9, 2);
	glActiveTexture(GL_TEXTURE4);
    render_quad(nmes_filter.attributeLocation("inTexCoord"), 4, fbo_buffs, 1);
    nmes_filter.release();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_id); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, binary_texture, 0);

    binary_filter.bind();
    binary_filter.setUniformValue("texture_0", 5);
    binary_filter.setUniformValue("min", (GLfloat)(30.0/255.0));
    binary_filter.setUniformValue("max", (GLfloat)(90.0/255.0));
	binary_filter.setUniformValueArray("coord", nbhrs, 8, 2);
	glActiveTexture(GL_TEXTURE5);
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

        /*gray_filter.bind();
        gray_filter.setUniformValue("texture_0", 3);
	    glActiveTexture(GL_TEXTURE3);     
        render_quad(gray_filter.attributeLocation("inTexCoord"), 1, 0, 0);
	    gray_filter.release();*/

        gray_filter.bind();
        gray_filter.setUniformValue("texture_0", 4);
	    glActiveTexture(GL_TEXTURE4);     
        render_quad(gray_filter.attributeLocation("inTexCoord"), 1, 0, 0);
	    gray_filter.release();

        gray_filter.bind();
        gray_filter.setUniformValue("texture_0", 5);
	    glActiveTexture(GL_TEXTURE5);     
        render_quad(gray_filter.attributeLocation("inTexCoord"), 2, 0, 0);
	    gray_filter.release();

        gray_filter.bind();
        gray_filter.setUniformValue("texture_0", 6);
	    glActiveTexture(GL_TEXTURE6);     
        render_quad(gray_filter.attributeLocation("inTexCoord"), 3, 0, 0);
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

