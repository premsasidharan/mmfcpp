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

Video_widget::Video_widget(QWidget* parent)
    :QGLWidget(parent)
    , format(Media::I420)
    , video_width(0)
    , video_height(0)
    , mode(Video_widget::RGB)
    , is_changed(false)
	, show_pb(false)
	, pb_status(1)
    , scale(-0.75)
    , texture_count(0)
    , program(this)
    , char_width(0)
    , char_height(0)
    , disp_text(0)
    , font_offset(0)
	, slide_flag(false)
{
    font_color[0] = font_color[1] = font_color[2] = 1.0;
	texture_data[0] = texture_data[1] = texture_data[2] = 0;
    setFocusPolicy(Qt::StrongFocus);
    connect(this, SIGNAL(update_frame()), this, SLOT(repaint()));
}

Video_widget::~Video_widget()
{
    glDeleteLists(font_offset, 256);
    glDeleteTextures(3, texture);
}

void Video_widget::set_display_mode(Video_widget::Mode _mode)
{
    mode = _mode;
    update();
}

void Video_widget::set_value(uint64_t pos)
{
	if (false == slide_flag)
	{
		scale = -0.75+((float)(pos-start)/(float)(end-start))*1.6;
	}
}

uint64_t Video_widget::current_pos()
{
	uint64_t pos = start+(uint64_t)((scale+0.75)*((float)(end-start))/1.6);
	return pos;
}

void Video_widget::set_slider_range(uint64_t _start, uint64_t _end)
{
	start = _start;
	end = _end;
}

void Video_widget::set_pb_control_status(int status)
{
	pb_status = status;
	update();
}

bool Video_widget::is_progress_bar_enabled()
{
	return show_pb;
}

void Video_widget::enable_progress_bar(bool en)
{
	show_pb = en;
	update();
}

void Video_widget::show_text(const char* text)
{
    mutex.lock();
    disp_text = (char *)text;
	mutex.unlock();
}

void Video_widget::show_frame(unsigned char* _yuv, int fmt, int width, int height)
{
    mutex.lock();
    if ((fmt != format) || (width != video_width) || (height != video_height))
    {
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
    glGenTextures(3, texture);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    create_font_disp_lists();
    program.addShaderFromSourceCode(QGLShader::Fragment, shader_program);
    program.link();
}

void Video_widget::create_font_disp_lists()
{
	QFont font("Arial", 18, QFont::Bold);
	QFontMetrics fm(font);
	
    font_offset = glGenLists(256);
	for (int i = 0; i <= 255; i++)
	{
        char ch = (char)i;
		QImage image(fm.width(ch), fm.height(), QImage::Format_Mono);
		image.fill(0);
		QPainter painter;
		painter.begin(&image);
		painter.setFont(font);
		painter.setPen(QPen(Qt::white));
		painter.drawText(QRect(0, 0, fm.width(ch), fm.height()), Qt::AlignCenter, QString("%1").arg(ch));
		painter.end();
		image = image.mirrored();
		
        glNewList(font_offset+ch, GL_COMPILE);
		glBitmap(fm.width(ch), fm.height(), 0, 0, fm.width(ch), 0, image.bits());
		glEndList();
	}
    char_width = fm.width('W');
    char_height = fm.height();
}

void Video_widget::draw_text(GLfloat x, GLfloat y, const char* text)
{
    glRasterPos2f(x, y);
	glListBase(font_offset);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *) text);
}

void Video_widget::render_frame(Video_widget::Pos pos, Video_widget::Mode mode)
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
        glTexCoord2f(0.0f, 0.0f); glVertex2fv(vertex[pos][0]);
        glTexCoord2f(1.0f, 0.0f); glVertex2fv(vertex[pos][1]);
        glTexCoord2f(1.0f, 1.0f); glVertex2fv(vertex[pos][2]);
        glTexCoord2f(0.0f, 1.0f); glVertex2fv(vertex[pos][3]);
    glEnd(); 
    glDisable(GL_TEXTURE_2D);

    program.release();
}

void Video_widget::render_text()
{
    if (0 != disp_text)
    {
        glColor3fv(font_color);
        GLfloat x = 1.0-((float)(char_width*(2+strlen(disp_text)))/(float)width());
        GLfloat y = -1.0+((float)(char_height*2)/(float)height());
        draw_text(x, y, disp_text);
    }
}

void Video_widget::render_notch(float x, float y, float width, float height)
{
	glBegin(GL_POLYGON);
	glVertex2f(x-width, y);
	glVertex2f(x, y+(height/2.0));
	glVertex2f(x+width, y); 
	glVertex2f(x+width, y-height); 
	glVertex2f(x-width, y-height); 
	glEnd();
}

void Video_widget::render_rectangle(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x+width, y); 
	glVertex2f(x+width, y-height); 
	glVertex2f(x, y-height); 
	glEnd();
}

void Video_widget::render_triangle(float x, float y, float width, float height)
{
	glBegin(GL_TRIANGLES);
	glVertex2f(x, y);
	glVertex2f(x+width, y-(height/2)); 
	glVertex2f(x, y-height); 
	glEnd();
}

void Video_widget::render_slider()
{
    glEnable(GL_BLEND);	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 0.3);
	render_rectangle(-0.9, -0.88, 1.8, 0.08);
    glDisable(GL_BLEND);	

	glColor4f(0.0, 0.0, 0.0, 0.6);
	render_rectangle(-0.75, -0.91, 1.6, 0.02);
	render_notch(scale, -0.91, 0.02, 0.04);
}

void Video_widget::render_pb_controls()
{
	if (show_pb)
	{
		render_slider();
		if (0 == pb_status)
		{
			render_rectangle(-0.88, -0.89, 0.02, 0.06);
			render_rectangle(-0.85, -0.89, 0.02, 0.06);
		}
		else
		{
			render_triangle(-0.88, -0.89, 0.09, 0.06);
		}
	}
}

void Video_widget::paintGL()
{
    if (false == program.isLinked())
    {
        return;
    }
    mutex.lock();
	if (video_width == 0 || video_height == 0)
	{
		glColor3f(0.3, 0.3, 0.3);
		render_rectangle(-1.0, 1.0, 2.0, 2.0);
	}
	else
	{
		create_textures();
		switch (mode)
		{
			case Video_widget::GRID_NYUV:
		    	render_frame(Video_widget::TL, Video_widget::RGB);
		    	render_frame(Video_widget::TR, Video_widget::Y);
		    	render_frame(Video_widget::BL, Video_widget::U);
		    	render_frame(Video_widget::BR, Video_widget::V);
				break;

			case Video_widget::GRID_NRGB:
		    	render_frame(Video_widget::TL, Video_widget::RGB);
		    	render_frame(Video_widget::TR, Video_widget::R);
		    	render_frame(Video_widget::BL, Video_widget::G);
		    	render_frame(Video_widget::BR, Video_widget::B);
				break;

			case Video_widget::Y:
			case Video_widget::U:
			case Video_widget::V:
			case Video_widget::R:
			case Video_widget::G:
			case Video_widget::B:
			case Video_widget::RGB:
				render_frame(Video_widget::FS, mode);
				break;
		}
		render_text();
		render_pb_controls();
	}
    mutex.unlock();
    glFlush();
}

void Video_widget::resizeGL(int _width, int _height)
{
    glViewport(0, 0, _width, _height);
}

void Video_widget::moveEvent(QMoveEvent* event)
{
    (void)event;
    //repaint();
}

void Video_widget::closeEvent(QCloseEvent* event)
{
    (void)event;
    emit renderer_close();
}
    
void Video_widget::mousePressEvent(QMouseEvent* event)
{
    if (show_pb && (event->button() & Qt::LeftButton))
    {
		float fx = (2.0*(float)event->x()/(float)width())-1.0;
		float fy = 1.0-(2.0*(float)event->y()/(float)height());

		if (fx >= -0.88 && fx <= -0.77 && fy <= -0.90 && fy >= -0.95)
		{
			emit pb_control(pb_status);
		}

		if (fx >= (scale-0.01) && fx <= (scale+0.01) && fy <= -0.90 && fy >= -0.95)
		{
			slide_flag = true;
		}
    }
}
 
void Video_widget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() & Qt::LeftButton)
    {
		slide_flag = false;
    }
}
   
void Video_widget::mouseMoveEvent(QMouseEvent* event)
{
	if (false == show_pb && false == slide_flag)
	{
		return;
	}
    if (event->buttons() & Qt::LeftButton)
    {
		float fx = (2.0*(float)event->x()/(float)width())-1.0;
		float fy = 1.0-(2.0*(float)event->y()/(float)height());

		if (fx >= -0.75 && fx <= 0.85)
		{
			scale = fx;
			uint64_t time = start+(uint64_t)(((scale+0.75)/1.6)*((float)(end-start)));

			if (0 == pb_status)
			{
				emit seek(time, end);
			}
			else
			{
				emit seek(time, time);
			}
			//repaint();
		}
    }
}

void Video_widget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_N:
            showNormal();
            break;

        case Qt::Key_F:
            showFullScreen();
            break;

        /*case Qt::Key_Up:
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
            break;*/
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

