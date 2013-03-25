#include <gl_widget.h>

#include <QtGui>

#include <cuda.h>
#include <cuda_runtime_api.h>

#include <histogram.h>

Gl_widget::Gl_widget(int width, int height, const QString& path, QWidget* parent)
	:QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    , video_width(width)
    , video_height(height)
    , y_texture(0)
    , u_texture(0)
    , v_texture(0)
    , y_data(0)
    , u_data(0)
    , v_data(0)
    , file(path)
    , timer(this)
    , program(this)
{
	setFocusPolicy(Qt::StrongFocus);
    
    y_data = new unsigned char[(width*height*3)>>1];
    u_data = y_data+(width*height);
    v_data = u_data+((width*height)>>2);
    file.open(QIODevice::ReadOnly);
}

Gl_widget::~Gl_widget()
{
    file.close();
    
    glDeleteTextures(1, &y_texture);
    glDeleteTextures(1, &u_texture);
    glDeleteTextures(1, &v_texture);

    u_data = 0;
    v_data = 0;
    delete [] y_data;
    y_data = 0;
}

void Gl_widget::initializeGL()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        qDebug() << "Error: glewInit() failed\n";
        exit(0);
    }

    qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment,
     "uniform sampler2D y_texture;\n"
     "uniform sampler2D u_texture;\n"
     "uniform sampler2D v_texture;\n"
     "void main(void)\n"
     "{\n"
     "   float y, u, v, red, green, blue;\n"
     "   y = texture2D(y_texture, gl_TexCoord[0].st).r;\n"
     "   y =  1.1643 * (y - 0.0625);\n"
     "   u = texture2D(u_texture, gl_TexCoord[0].st).r - 0.5;\n"
     "   v = texture2D(v_texture, gl_TexCoord[0].st).r - 0.5;\n"
     "   red = y+1.5958*v;\n"
     "   green = y-0.39173*u-0.81290*v;\n"
     "   blue = y+2.017*u;\n"
     "   gl_FragColor = vec4(red, green, blue, 1.0);\n"
     "}");
     
    qDebug() << program.link();
    
    glGenTextures(1, &y_texture);
    glGenTextures(1, &u_texture);
    glGenTextures(1, &v_texture);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_width, video_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y_data);
    
    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_width>>1, video_height>>1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u_data);
    
    glActiveTexture(GL_TEXTURE2); 
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_width>>1, video_height>>1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v_data);

    glGenBuffers(3, hist_obj);    
    for (int i = 0; i < 3; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, hist_obj[i]);
        glBufferData(GL_ARRAY_BUFFER, 256*2*sizeof(GL_INT), 0, GL_DYNAMIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    timer.setInterval(40);
    timer.start();
    
    cudaGLSetGLDevice(0);
}

void Gl_widget::paintGL()
{
    glViewport(0, 0, width(), height());
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
    
    file.read((char*)y_data, (video_width*video_height*3)>>1);
    
    program.bind();
    program.setUniformValue("y_texture", 0);
    program.setUniformValue("u_texture", 1);
    program.setUniformValue("v_texture", 2);

	glActiveTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, y_data);
	glActiveTexture(GL_TEXTURE1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width>>1, video_height>>1, GL_LUMINANCE, GL_UNSIGNED_BYTE, u_data);
	glActiveTexture(GL_TEXTURE2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width>>1, video_height>>1, GL_LUMINANCE, GL_UNSIGNED_BYTE, v_data);   
   
    compute_histogram(y_texture, u_texture, v_texture, hist_obj, video_width, video_height);

	glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f);              // Top Left
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);              // Top Right
        glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
        glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
    glEnd(); 
	glFlush();
    glDisable(GL_TEXTURE_2D);
    program.release();

	glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 256.0, 0.0, (video_width*video_height)>>4, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

    glBindBuffer(GL_ARRAY_BUFFER, hist_obj[0]);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertexPointer(2, GL_INT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, 256);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, hist_obj[1]);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertexPointer(2, GL_INT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 1, 255);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, hist_obj[2]);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertexPointer(2, GL_INT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, 256);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Gl_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void Gl_widget::on_timeout()
{
    updateGL();
}

