/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <string.h>
#include <stdlib.h>

#include <media.h>
#include <video_widget.h>

#include <QMouseEvent>

static const char shader_program[] =
"uniform int color;\n"
"uniform int format;\n"
"uniform sampler2D texture_0;\n"
"uniform sampler2D texture_1;\n"
"uniform sampler2D texture_2;\n"
"void extract_yuv(out float y, out float u, out float v)\n"
"{\n"
"    if (format == 0x30323449) {\n"
"        y = texture2D(texture_0, gl_TexCoord[0].st).r;\n"
"        y =  1.1643 * (y - 0.0625);\n"
"        u = texture2D(texture_1, gl_TexCoord[0].st).r - 0.5;\n"
"        v = texture2D(texture_2, gl_TexCoord[0].st).r - 0.5;\n"
"    } else if (format == 0x32595559) {\n"
"        y = texture2D(texture_0, gl_TexCoord[0].st).r;\n"
"        y = 1.1643*(y-0.0625);\n"
"        u = texture2D(texture_1, gl_TexCoord[0].st).g-0.5;\n"
"        v = texture2D(texture_1, gl_TexCoord[0].st).a-0.5;\n"
"    }\n"
"}\n"
"void main(void)\n"
"{\n"
"    float y, u, v;\n"
"    float red, green, blue;\n"
"    extract_yuv(y, u, v);\n"
"    red = y+1.5958*v;\n"
"    green = y-0.39173*u-0.81290*v;\n"
"    blue = y+2.017*u;\n"
"    if (color == 0)\n"
"        gl_FragColor = vec4(y, y, y, 1.0);\n"
"    else\n"
"        gl_FragColor = vec4(red, green, blue, 1.0);\n"
"}\n";

Video_widget::Video_widget(QWidget* _control, QWidget* parent)
    :QGLWidget(parent)
    , controls(_control)
    , format(0)
	, is_changed(false)
    , video_width(0)
    , video_height(0)
    , scale(1.0)
	, texture_count(0)
	, program(this)
{
    if (0 != controls)
    {
        controls->setParent(this);
        controls->hide();
    }
	setFocusPolicy(Qt::StrongFocus);
    connect(this, SIGNAL(update_frame()), this, SLOT(repaint()));
}

Video_widget::~Video_widget()
{
	delete_textures();
    if (0 != controls)
    {
        controls->setParent(0);
    }
}

void Video_widget::show_frame(unsigned char* _yuv, int fmt, int width, int height)
{
    mutex.lock();
	if ((fmt != format) || (width != video_width) || (height != video_height))
	{
		delete_textures();
		is_changed = true;
		format = fmt;
		video_width = width;
		video_height = height;
	}
	switch (fmt)
	{
		case Media::YUY2:
			texture_data[0] = _yuv;
			texture_data[1] = _yuv;
			break;
		case Media::YV12:
			texture_data[0] = _yuv;
			texture_data[2] = texture_data[0]+(width*height);
			texture_data[1] = texture_data[2]+((width*height)>>2);
			break;
		case Media::I420:
			texture_data[0] = _yuv;
			texture_data[1] = texture_data[0]+(width*height);
			texture_data[2] = texture_data[1]+((width*height)>>2);
			break;
		case Media::UYVY:
			break;
		
	}
    mutex.unlock();
    emit update_frame();
}

void Video_widget::initializeGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void Video_widget::paintGL()
{
    mutex.lock();
	if (is_changed)
	{
		create_textures();
		is_changed = false;
	}
	for (int i = 0; i < texture_count; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width[i], texture_height[i], 
            texture_format[i], GL_UNSIGNED_BYTE, texture_data[i]);
	}
    mutex.unlock();

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 0.0f); glVertex3f(-scale, scale, 0.0f);
	    glTexCoord2f(1.0f, 0.0f); glVertex3f(scale, scale, 0.0f);
	    glTexCoord2f(1.0f,1.0f); glVertex3f(scale, -scale, 0.0f);
	    glTexCoord2f(0.0f,1.0f); glVertex3f(-scale, -scale, 0.0f);
	glEnd(); 
	glFlush();
	glDisable(GL_TEXTURE_2D);
}

void Video_widget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
    if (0 != controls)
    {
        controls->resize(width-50, 30);
        controls->move(25, height-40);
    }
}

void Video_widget::moveEvent(QMoveEvent* event)
{
	(void)event;
    repaint();
}

void Video_widget::closeEvent(QCloseEvent* event)
{
	(void)event;
    emit renderer_close();
}
    
void Video_widget::mousePressEvent(QMouseEvent* event)
{
    if (0 != controls)
    {
        if (event->button() & Qt::LeftButton)
        {
            program.setUniformValue("color", controls->isVisible());
            if (controls->isVisible())
            {
                controls->hide();
            }
            else
            {
                controls->show();
            }
        }
    }
}

void Video_widget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Up:
            if ((scale+0.1f) <= 1.0f)
            {
                scale += 0.1f;
            }
            break;
        case Qt::Key_Down:
            if (scale > 0.5f)
            {
                scale -= 0.1f;
            }
            break;
    }
}

void Video_widget::create_textures()
{
	switch (format)
	{
		case Media::YUY2:
			create_yuy2_textures();
			break;
		case Media::YV12:
		case Media::I420:
			create_i420_textures();
			break;
		case Media::UYVY:
			create_uyvy_textures();
			break;
	}
}

void Video_widget::create_yuy2_textures()
{
	program.addShaderFromSourceCode(QGLShader::Fragment,
        "uniform int color;\n"
        "uniform sampler2D texture_0;\n"
        "uniform sampler2D texture_1;\n"
        "uniform sampler2D texture_2;\n"
        "void main(void)\n"
        "{\n"
        "    float y, u, v, red, green, blue;\n"	
        "    y = texture2D(texture_0, gl_TexCoord[0].st).r;\n"
        "    u = texture2D(texture_1, gl_TexCoord[0].st).g-0.5;\n"
        "    v = texture2D(texture_1, gl_TexCoord[0].st).a-0.5;\n"
        "    y = 1.1643*(y-0.0625);\n"
        "    red = y+1.5958*v;\n"
        "    green = y-0.39173*u-0.81290*v;\n"
        "    blue = y+2.017*u;\n"
        "    gl_FragColor = vec4(red, green, blue, 1.0);\n"
        "}");

    program.link();
    program.bind();

    glGenTextures(2, texture);
	texture_count = 2;

	texture_width[0] = video_width;
	texture_height[0] = video_height;
    
	texture_width[1] = video_width>>1;
	texture_height[1] = video_height;
	
	texture_format[0] = GL_LUMINANCE_ALPHA;
	texture_format[1] = GL_RGBA;

	texture_int_format[0] = GL_LUMINANCE_ALPHA;
	texture_int_format[1] = GL_RGBA;

	for (int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(GL_TEXTURE_2D, 0, texture_int_format[i], texture_width[i], texture_height[i], 0, 
            texture_format[i], GL_UNSIGNED_BYTE, 0);
    }
    
    program.setUniformValue("width", 640.0f);
    program.setUniformValue("color", 1);
    program.setUniformValue("format", 0x32595559);
    program.setUniformValue("texture_0", 0);
    program.setUniformValue("texture_1", 1);
}

void Video_widget::create_i420_textures()
{
	qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment,
        shader_program
		//"uniform int fmt;\n"
        /*"uniform sampler2D texture_y;\n"
		"uniform sampler2D texture_u;\n"
		"uniform sampler2D texture_v;\n"
		"void main(void)\n"
		"{\n"
		"   float y, u, v, red, green, blue;\n"
		"   y = texture2D(texture_y, gl_TexCoord[0].st).r;\n"
		"   y =  1.1643 * (y - 0.0625);\n"
		"   u = texture2D(texture_u, gl_TexCoord[0].st).r - 0.5;\n"
		"   v = texture2D(texture_v, gl_TexCoord[0].st).r - 0.5;\n"
		"   red = y+1.5958*v;\n"
		"   green = y-0.39173*u-0.81290*v;\n"
		"   blue = y+2.017*u;\n"
        "   if (fmt >= 1)\n"
		"      gl_FragColor = vec4(red, red, red, 1.0);\n"
        "   else\n"
		"      gl_FragColor = vec4(red, green, blue, 1.0);\n"
		"}"*/);
    
    qDebug() << program.link();
    qDebug() << program.bind();

    glGenTextures(3, texture);
	texture_count = 3;

	texture_width[0] = video_width;
	texture_height[0] = video_height;
    
	texture_width[1] = texture_width[2] = video_width>>1;
	texture_height[1] = texture_height[2] = video_height>>1;
	
	texture_format[0] = GL_LUMINANCE;
	texture_format[1] = GL_LUMINANCE;
	texture_format[2] = GL_LUMINANCE;

	texture_int_format[0] = GL_LUMINANCE;
	texture_int_format[1] = GL_LUMINANCE;
	texture_int_format[2] = GL_LUMINANCE;

	for (int i = 0; i < 3; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(GL_TEXTURE_2D, 0, texture_int_format[i], texture_width[i], 
            texture_height[i], 0, texture_format[i], GL_UNSIGNED_BYTE, 0);
    }
    program.setUniformValue("color", 1);
    program.setUniformValue("format", 0x30323449);
    program.setUniformValue("texture_0", 0);
    program.setUniformValue("texture_1", 1);
    program.setUniformValue("texture_2", 2);
}

void Video_widget::create_uyvy_textures()
{
}

void Video_widget::delete_textures()
{
	if (texture_count > 0)
	{
		program.removeAllShaders();
		program.release();
		glDeleteTextures(texture_count, texture);
		texture_count = 0;
	}
}

/*
"uniform int color;\n"
"uniform float width;\n"
"uniform sampler2D texture_0;\n"
"uniform sampler2D texture_1;\n"
"void main(void)\n"
"{\n"
"    float xcoord;\n"
"    float y, u, v, red, green, blue;\n"	  
"    float fx, fy;\n"
"    fx = gl_TexCoord[0].x;\n"	  
"    fy = gl_TexCoord[0].y;\n"
"    y = texture2D(texture_0, vec2(fx, fy)).s;\n"
"    y =  1.1643 * (y - 0.0625);\n"
"    xcoord = fx*width;\n"
"    if (mod(xcoord, 2.0) == 0.0)\n"
"    {\n"
"        u = texture2D(texture_0, vec2(fx, fy)).t;\n"
"        v = texture2D(texture_0, vec2((fx+(1.0/width)), fy)).t;\n"
"    }\n"
"    else\n"
"    {\n"
"        v = texture2D(texture_0, vec2(fx, fy)).t;\n"
"        u = texture2D(texture_0, vec2((fx-(1.0/width)), fy)).t;\n"     
"    }\n"
"    red = y+1.5958*v;\n"
"    green = y-0.39173*u-0.81290*v;\n"
"    blue = y+2.017*u;\n"
"    if (1 == color)\n"
"        gl_FragColor = vec4(v, 0.0, 0.0, 1.0);\n"
"     else\n"
"        gl_FragColor = vec4(y, 0.0, 0.0, 1.0);\n"
"}"*/
