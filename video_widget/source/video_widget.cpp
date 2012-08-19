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

const char Video_widget::shader_program[] =
    "uniform int format;\n"
    "uniform int mode;\n"
    "uniform sampler2D texture_0;\n"
    "uniform sampler2D texture_1;\n"
    "uniform sampler2D texture_2;\n"
    "void extract_yuv(out float y, out float u, out float v)\n"
    "{\n"
    "    y = 0.0; u = 0.0; v = 0.0;\n"
    "    if (format == 1) {\n"
    "        y = texture2D(texture_0, gl_TexCoord[0].st).r;\n"
    "        y = 1.1643 * (y - 0.0625);\n"
    "        u = texture2D(texture_1, gl_TexCoord[0].st).r - 0.5;\n"
    "        v = texture2D(texture_2, gl_TexCoord[0].st).r - 0.5;\n"
    "    } else if (format == 2) {\n"
    "        y = texture2D(texture_0, gl_TexCoord[0].st).r;\n"
    "        y = 1.1643*(y-0.0625);\n"
    "        u = texture2D(texture_1, gl_TexCoord[0].st).g-0.5;\n"
    "        v = texture2D(texture_1, gl_TexCoord[0].st).a-0.5;\n"
    "    } else if (format == 3) {\n"
    "        y = texture2D(texture_0, gl_TexCoord[0].st).g;\n"
    "        y = 1.1643*(y-0.0625);\n"
    "        u = texture2D(texture_1, gl_TexCoord[0].st).r-0.5;\n"
    "        v = texture2D(texture_1, gl_TexCoord[0].st).b-0.5;\n"
    "    }\n"
    "}\n"
    "void display(in float y, in float u, in float v)\n"
    "{\n"
    "    float red = 0.0, green = 0.0, blue = 0.0;\n"
    "    if (mode >= 3) {\n"
    "        red = y+1.5958*v;\n"
    "        green = y-0.39173*u-0.81290*v;\n"
    "        blue = y+2.017*u;\n"
    "    }\n"
    "    if (mode == 0)\n"
    "        gl_FragColor = vec4(y, y, y, 1.0);\n"
    "    else if (mode == 1)\n"
    "        gl_FragColor = vec4(u, u, u, 1.0);\n"
    "    else if (mode == 2)\n"
    "        gl_FragColor = vec4(v, v, v, 1.0);\n"
    "    else if (mode == 3)\n"
    "        gl_FragColor = vec4(red, 0.0, 0.0, 1.0);\n"
    "    else if (mode == 4)\n"
    "        gl_FragColor = vec4(0.0, green, 0.0, 1.0);\n"
    "    else if (mode == 5)\n"
    "        gl_FragColor = vec4(0.0, 0.0, blue, 1.0);\n"
    "    else\n"
    "        gl_FragColor = vec4(red, green, blue, 1.0);\n"
    "}\n"
    "void main(void)\n"
    "{\n"
    "    float y, u, v;\n"
    "    extract_yuv(y, u, v);\n"
    "    display(y, u, v);\n"
    "}\n";

Video_widget::Video_widget(QWidget* _control, QWidget* parent)
    :QGLWidget(parent)
    , controls(_control)
    , format(Media::I420)
    , video_width(0)
    , video_height(0)
    , is_changed(false)
    , is_grayscale(false)
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

void Video_widget::set_gray_scale(const bool gray)
{
    is_grayscale = gray;
    repaint();
}

void Video_widget::show_frame(unsigned char* _yuv, int fmt, int width, int height)
{
    mutex.lock();
    if ((fmt != format) || (width != video_width) || (height != video_height))
    {
        delete_textures();
        format = fmt;
        video_width = width;
        video_height = height;

        is_changed = true;
    }

    switch (fmt)
    {
        case Media::YUY2:
        case Media::UYVY:
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
        case Media::I422:
            texture_data[0] = _yuv;
            texture_data[1] = texture_data[0]+(width*height);
            texture_data[2] = texture_data[1]+((width*height)>>1);
            break;
        case Media::I444:
            texture_data[0] = _yuv;
            texture_data[1] = texture_data[0]+(width*height);
            texture_data[2] = texture_data[1]+(width*height);
            break;
    }
    mutex.unlock();
    emit update_frame();
}

void Video_widget::initializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    program.addShaderFromSourceCode(QGLShader::Fragment, shader_program);
    program.link();
}

void Video_widget::render_image(int disp_mode, int mode)
{

    static GLfloat vertex[][4][2] = {{{-1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {-1.0, 0.0}},
                                     {{0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0}},
                                     {{-1.0, 0.0}, {0.0, 0.0}, {0.0, -1.0}, {-1.0, -1.0}},
                                     {{0.0, 0.0}, {1.0, 0.0}, {1.0, -1.0}, {0.0, -1.0}},
                                     {{-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}, {-1.0, -1.0}}};
    program.bind();

    program.setUniformValue("texture_0", 0);
    program.setUniformValue("texture_1", 1);
    program.setUniformValue("texture_2", 2);
    program.setUniformValue("format", format_code());
    program.setUniformValue("mode", mode);

    for (int i = 0; i < texture_count; i++)
    {
        glActiveTexture(GL_TEXTURE0+i);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width[i], texture_height[i], 
            texture_format[i], GL_UNSIGNED_BYTE, texture_data[i]);
    }

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2fv(vertex[disp_mode][0]);
        glTexCoord2f(1.0f, 0.0f); glVertex2fv(vertex[disp_mode][1]);
        glTexCoord2f(1.0f, 1.0f); glVertex2fv(vertex[disp_mode][2]);
        glTexCoord2f(0.0f, 1.0f); glVertex2fv(vertex[disp_mode][3]);
    glEnd(); 
    glDisable(GL_TEXTURE_2D);

    program.release();
}

void Video_widget::paintGL()
{
    if (false == program.isLinked())
    {
        return;
    }

    mutex.lock();
    create_textures();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    int my_mode = is_grayscale?5:4;

    if (my_mode < 5)
    {
        render_image(4, 6);
    }
    else if (my_mode == 5)
    {
        render_image(0, 6);
        render_image(1, 0);
        render_image(2, 1);
        render_image(3, 2);
    }
    else if (my_mode == 6)
    {
        render_image(0, 6);
        render_image(1, 3);
        render_image(2, 4);
        render_image(3, 5);
    }
    mutex.unlock();

    glFlush();
}

void Video_widget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    if (0 != controls)
    {
        controls->resize(width-50, 40);
        controls->move(25, height-50);
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
    if (false == is_changed)
    {
        return;
    }

    switch (format)
    {
        case Media::YUY2:
        case Media::UYVY:
            create_yuy2_textures();
            break;

        case Media::YV12:
        case Media::I420:
        case Media::I422:
        case Media::I444:
            create_i420_textures();
            break;
    }

    is_changed = false;
}

void Video_widget::create_yuy2_textures()
{
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
        glTexImage2D(GL_TEXTURE_2D, 0, texture_int_format[i], texture_width[i], 
            texture_height[i], 0, texture_format[i], GL_UNSIGNED_BYTE, 0);
    }
}

void Video_widget::create_i420_textures()
{
    glGenTextures(3, texture);
    texture_count = 3;

    texture_width[0] = video_width;
    texture_height[0] = video_height;

    switch (format)
    {
        case Media::YV12:
        case Media::I420:
            texture_width[1] = texture_width[2] = video_width>>1;
            texture_height[1] = texture_height[2] = video_height>>1;    
            break;
        case Media::I422:
            texture_width[1] = texture_width[2] = video_width>>1;
            texture_height[1] = texture_height[2] = video_height;  
            break;
        case Media::I444:
            texture_width[1] = texture_width[2] = video_width;
            texture_height[1] = texture_height[2] = video_height;  
            break;
    }

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

int Video_widget::format_code() const
{
    int code = 1;
    switch (format)
    {
        case Media::YV12:
        case Media::I420:
        case Media::I422:
        case Media::I444:
            code = 1;
            break;

        case Media::YUY2:	
            code = 2;		
            break;

        case Media::UYVY:
            code = 3;
            break;
    }
    return code;
}

