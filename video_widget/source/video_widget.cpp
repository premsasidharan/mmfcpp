/*
 *  Copyright (C) 2012 Prem Sasidharan.
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

Video_widget::Video_widget(QWidget* parent)
    :QGLWidget(parent)
	, view_count(1)
	, stereo_mode(Video_widget::INT_LEAVE)
    , mode(Video_widget::RGB)
	, pb_state(Video_widget::Init)
    , is_changed(false)
	, is_visible(false)
    , scale(-0.75)
    , yuv_rgb(this)
    , yuv_rgb_blend(this)
    , yuv_rgb_split(this)
    , char_width(0)
    , char_height(0)
    , disp_text(0)
    , font_offset(0)
	, slide_flag(false)
    , buff_manager1(this, 10)
    , buff_manager2(this, 10)
{
	init();
}

void Video_widget::init()
{
	for (int i = 0; i < 2; i++)
	{
        gl_buffer[i] = 0;
    	video_width[i] = 0;
    	video_height[i] = 0;
    	texture_count[i] = 0;
    	format[i] = Media::I420;
		texture_data[i][0] = texture_data[i][1] = texture_data[i][2] = 0;
	}
    font_color[0] = font_color[1] = font_color[2] = 1.0;
    setFocusPolicy(Qt::StrongFocus);
    connect(this, SIGNAL(update_frame()), this, SLOT(repaint()));
}

Video_widget::~Video_widget()
{
    glDeleteLists(font_offset, 256);
    glDeleteTextures(3, texture[0]);
    glDeleteTextures(3, texture[1]);
}

void Video_widget::set_display_mode(Video_widget::Mode _mode)
{
    mutex.lock();
    mode = _mode;
	mutex.unlock();
    update();
}

void Video_widget::set_stereo_mode(int mode)
{
    mutex.lock();
    stereo_mode = mode;
    view_count = (Video_widget::NONE == stereo_mode)?1:2;
	mutex.unlock();
    update();

}

void Video_widget::set_slider_value(uint64_t pos)
{
	if (false == slide_flag)
	{
		scale = -0.75+((float)(pos-start)/(float)(end-start))*1.6;
	}
}

uint64_t Video_widget::slider_value()
{
	uint64_t pos = start+(uint64_t)((scale+0.75)*((float)(end-start))/1.6);
	return pos;
}

void Video_widget::set_slider_range(uint64_t _start, uint64_t _end)
{
	start = _start;
	end = _end;
}

void Video_widget::set_playback_control_state(Video_widget::State state)
{
	pb_state = state;
	update();
}

bool Video_widget::is_controls_visible()
{
	return is_visible;
}

void Video_widget::show_playback_controls(bool en)
{
    mutex.lock();
	is_visible = en;
	mutex.unlock();
	update();
}

void Video_widget::show_text(const char* text)
{
    mutex.lock();
    disp_text = (char *)text;
	mutex.unlock();
}

void Video_widget::set_texture_data(int i, uint8_t* yuv)
{
    switch (format[i])
    {
        case Media::YUY2:
        case Media::UYVY:
            texture_data[i][0] = yuv;
            texture_data[i][1] = yuv;
            break;
        case Media::YV12:
            texture_data[i][0] = yuv;
            texture_data[i][2] = texture_data[i][0]+(video_width[i]*video_height[i]);
            texture_data[i][1] = texture_data[i][2]+((video_width[i]*video_height[i])>>2);
            break;
        case Media::I420:
            texture_data[i][0] = yuv;
            texture_data[i][1] = texture_data[i][0]+(video_width[i]*video_height[i]);
            texture_data[i][2] = texture_data[i][1]+((video_width[i]*video_height[i])>>2);
            break;
        case Media::I422:
            texture_data[i][0] = yuv;
            texture_data[i][1] = texture_data[i][0]+(video_width[i]*video_height[i]);
            texture_data[i][2] = texture_data[i][1]+((video_width[i]*video_height[i])>>1);
            break;
        case Media::I444:
            texture_data[i][0] = yuv;
            texture_data[i][1] = texture_data[i][0]+(video_width[i]*video_height[i]);
            texture_data[i][2] = texture_data[i][1]+(video_width[i]*video_height[i]);
            break;
    }
}

void Video_widget::show_frame(int view, int fmt, int width, int height, uint8_t* yuv, GLuint gl_buff)
{
	if (view >= view_count)
	{
		return;
	}

    mutex.lock();
    gl_buffer[view] = gl_buff;
    if ((fmt != format[view]) || (width != video_width[view]) || (height != video_height[view]))
    {
        format[view] = fmt;
        video_width[view] = width;
        video_height[view] = height;

        is_changed = true;
    }
	set_texture_data(view, yuv);
    mutex.unlock();
    //emit update_frame();
}

bool Video_widget::init_yuv_rgb_shader()
{
    yuv_rgb.bindAttributeLocation("inVertex", 0);
    yuv_rgb.bindAttributeLocation("inTexCoord", 1);
    yuv_rgb.bindAttributeLocation("texCoord", 2);
    yuv_rgb.bindAttributeLocation("fragColor", 3);

    bool status = yuv_rgb.addShaderFromSourceCode(QGLShader::Vertex, vertex_shader_text);
    status = status && yuv_rgb.addShaderFromSourceCode(QGLShader::Fragment, yuv_rgb_shader_text);
    glBindFragDataLocation(yuv_rgb.programId(), 0, "fragColor");
    status = status && yuv_rgb.link();

    return status;
}

bool Video_widget::init_yuv_rgb_blend_shader()
{
    yuv_rgb_blend.bindAttributeLocation("inVertex", 0);
    yuv_rgb_blend.bindAttributeLocation("inTexCoord", 1);
    yuv_rgb_blend.bindAttributeLocation("texCoord", 2);
    yuv_rgb_blend.bindAttributeLocation("fragColor", 3);

    bool status = yuv_rgb_blend.addShaderFromSourceCode(QGLShader::Vertex, vertex_shader_text);
    status = status && yuv_rgb_blend.addShaderFromSourceCode(QGLShader::Fragment, yuv_rgb_blend_shader_text);
    glBindFragDataLocation(yuv_rgb_blend.programId(), 0, "fragColor");
    status = status && yuv_rgb_blend.link();

    return status;
}

bool Video_widget::init_yuv_rgb_split_shader()
{
    yuv_rgb_split.bindAttributeLocation("inVertex", 0);
    yuv_rgb_split.bindAttributeLocation("inTexCoord", 1);
    yuv_rgb_split.bindAttributeLocation("texCoord", 2);
    yuv_rgb_split.bindAttributeLocation("fragColor", 3);

    bool status = yuv_rgb_split.addShaderFromSourceCode(QGLShader::Vertex, vertex_shader_text);
    status = status && yuv_rgb_split.addShaderFromSourceCode(QGLShader::Fragment, yuv_rgb_split_shader_text);
    glBindFragDataLocation(yuv_rgb_split.programId(), 0, "fragColor");
    status = status && yuv_rgb_split.link();

    return status;
}

void Video_widget::init_shader()
{
    bool status = init_yuv_rgb_shader();
    status = status && init_yuv_rgb_blend_shader();
    status = status && init_yuv_rgb_split_shader();
    if (false == status)
    {
        exit(0);
    }
    else
    {
        program[0] = &yuv_rgb;
        program[1] = &yuv_rgb_blend;
        program[2] = &yuv_rgb_split;
    }
}

void Video_widget::initializeGL()
{
    glewInit();
    glGenTextures(3, texture[0]);
    glGenTextures(3, texture[1]);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    create_font_disp_lists();
    init_shader();
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

void Video_widget::texture_ids(int fmt, int& id0, int& id1, int& id2)
{
    switch (fmt)
    {
        case Media::YUY2:
        case Media::UYVY:
            id0 = 0;
            id1 = 1;
            id2 = 1;
            break;
        case Media::YV12:
        case Media::I420:
        case Media::I422:
        case Media::I444:
            id0 = 0;
            id1 = 1;
            id2 = 2;
            break;
    }
}

void Video_widget::setup_shader(int id, int sid, Video_widget::Mode mode)
{
    int t0, t1, t2;

    texture_ids(format[0], t0, t1, t2);

    program[id]->bind();
    program[id]->setUniformValue("texture_0", t0);
    program[id]->setUniformValue("texture_1", t1);
    program[id]->setUniformValue("texture_2", t2);
    switch (id)
    {
        case 0:
            program[id]->setUniformValueArray("pos_coeff", pos_coeff[0], 4, 1); 
            program[id]->setUniformValueArray("yuv_coeff", yuv_coeff[format_code(0)], 7, 1); 
            program[id]->setUniformValueArray("rgb_coeff", rgb_coeff[mode], 12, 1); 
            break;
        case 1:
            texture_ids(format[1], t0, t1, t2);
            program[id]->setUniformValueArray("pos_coeff", pos_coeff[0], 4, 1); 
            program[id]->setUniformValueArray("yuv_coeff1", yuv_coeff[format_code(0)], 7, 1); 
            program[id]->setUniformValueArray("yuv_coeff2", yuv_coeff[format_code(1)], 7, 1); 
            program[id]->setUniformValueArray("rgb_coeff", rgb_coeff[mode], 12, 1); 
            program[id]->setUniformValueArray("blend_coeff", blend_coeff[sid-1], 6, 1); 
            program[id]->setUniformValue("texture_3", 3+t0);
            program[id]->setUniformValue("texture_4", 3+t1);
            program[id]->setUniformValue("texture_5", 3+t2);
            break;
        case 2:
            texture_ids(format[1], t0, t1, t2);
		    program[id]->setUniformValue("stereo_mode", (sid&0X0F));
            program[id]->setUniformValueArray("pos_coeff1", pos_coeff[(sid&0x30)>>4], 4, 1); 
            program[id]->setUniformValueArray("pos_coeff2", pos_coeff[(sid&0xC0)>>6], 4, 1); 
            program[id]->setUniformValueArray("yuv_coeff1", yuv_coeff[format_code(0)], 7, 1); 
            program[id]->setUniformValueArray("yuv_coeff2", yuv_coeff[format_code(1)], 7, 1); 
            program[id]->setUniformValueArray("rgb_coeff", rgb_coeff[mode], 12, 1); 
            program[id]->setUniformValue("texture_3", 3+t0);
            program[id]->setUniformValue("texture_4", 3+t1);
            program[id]->setUniformValue("texture_5", 3+t2);
            break;
    }

}

void Video_widget::render_frame(Video_widget::Pos pos, Video_widget::Mode mode)
{
    static const GLfloat tex_coord[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    static const GLfloat vertex_coord[][8] = {{-1.0, 1.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0},
                                              {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0},
                                              {-1.0, 0.0, 0.0, 0.0, 0.0, -1.0, -1.0, -1.0},
                                              {0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 0.0, -1.0},
                                              {-1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0}};

    int id = (view_count == 1)?0:(stereo_mode>>8);
    int sid = (stereo_mode&0xFF);

    setup_shader(id, sid, mode);

    for (int view = 0; view < view_count; view++)
    {
        if (0 == gl_buffer[view] && 0 == texture_data[view][0])
        {
            continue;
        }
        
        for (int i = 0; i < texture_count[view]; i++)
        {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gl_buffer[view]);
            glActiveTexture(GL_TEXTURE0+i+(3*view));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width[view][i], texture_height[view][i], 
                texture_format[view][i], GL_UNSIGNED_BYTE, texture_data[view][i]);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }

    int loc = program[id]->attributeLocation("inTexCoord");
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArray(loc);
    glVertexPointer(2, GL_FLOAT, 0, &vertex_coord[pos]); 
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coord);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, tex_coord); 
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(loc);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    program[id]->release();
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

void Video_widget::render_playback_controls()
{
	if (is_visible)
	{
		render_slider();
		if (Video_widget::Pause == pb_state)
		{
			render_rectangle(-0.88, -0.89, 0.02, 0.06);
			render_rectangle(-0.85, -0.89, 0.02, 0.06);
		}
		else if (Video_widget::Play == pb_state)
		{
			render_triangle(-0.88, -0.89, 0.09, 0.06);
		}
	}
}

void Video_widget::paintGL()
{
    mutex.lock();
	if (video_width[0] == 0 || video_height[0] == 0)
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
		render_playback_controls();
	}
    mutex.unlock();
    glFinish();
}

void Video_widget::resizeGL(int _width, int _height)
{
    glViewport(0, 0, _width, _height);
}

void Video_widget::closeEvent(QCloseEvent* event)
{
    (void)event;
    qDebug() << "Video_widget::closeEvent";
    emit renderer_close();
}
    
void Video_widget::mousePressEvent(QMouseEvent* event)
{
    if (is_visible && (event->button() & Qt::LeftButton))
    {
		float fx = (2.0*(float)event->x()/(float)width())-1.0;
		float fy = 1.0-(2.0*(float)event->y()/(float)height());

		if (fx >= -0.88 && fx <= -0.77 && fy <= -0.90 && fy >= -0.95)
		{
			emit pb_control(pb_state);
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
	if (false == is_visible || false == slide_flag)
	{
		return;
	}
    if (event->buttons() & Qt::LeftButton)
    {
		float fx = (2.0*(float)event->x()/(float)width())-1.0;

		if (fx >= -0.75 && fx <= 0.85)
		{
			scale = fx;
			uint64_t time = start+(uint64_t)(((scale+0.75)/1.6)*((float)(end-start)));

			if (Video_widget::Pause == pb_state)
			{
				emit seek(time, end);
			}
			else if (Video_widget::Play == pb_state)
			{
				emit seek(time, time);
			}
			update();
		}
    }
}

void Video_widget::create_textures()
{
    if (false == is_changed)
    {
        return;
    }

	for (int view = 0; view < view_count; view++)
	{
		switch (format[view])
		{
		    case Media::YUY2:
		    case Media::UYVY:
		        create_yuy2_textures(view);
		        break;

		    case Media::YV12:
		    case Media::I420:
		    case Media::I422:
		    case Media::I444:
		        create_i420_textures(view);
		        break;
		}
	}

    is_changed = false;
}

void Video_widget::create_yuy2_textures(int view)
{
    texture_count[view] = 2;

    texture_width[view][0] = video_width[view];
    texture_height[view][0] = video_height[view];

    texture_width[view][1] = video_width[view]>>1;
    texture_height[view][1] = video_height[view];

    texture_format[view][0] = GL_LUMINANCE_ALPHA;
    texture_format[view][1] = GL_RGBA;

    texture_int_format[view][0] = GL_LUMINANCE_ALPHA;
    texture_int_format[view][1] = GL_RGBA;

    for (int i = 0; i < 2; i++)
    {
        glActiveTexture(GL_TEXTURE0+(3*view)+i);
        glBindTexture(GL_TEXTURE_2D, texture[view][i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_int_format[view][i], texture_width[view][i], 
            texture_height[view][i], 0, texture_format[view][i], GL_UNSIGNED_BYTE, 0);
    }
}

void Video_widget::create_i420_textures(int view)
{
    texture_count[view] = 3;

    texture_width[view][0] = video_width[view];
    texture_height[view][0] = video_height[view];

    switch (format[view])
    {
        case Media::YV12:
        case Media::I420:
            texture_width[view][1] = texture_width[view][2] = video_width[view]>>1;
            texture_height[view][1] = texture_height[view][2] = video_height[view]>>1;    
            break;
        case Media::I422:
            texture_width[view][1] = texture_width[view][2] = video_width[view]>>1;
            texture_height[view][1] = texture_height[view][2] = video_height[view];  
            break;
        case Media::I444:
            texture_width[view][1] = texture_width[view][2] = video_width[view];
            texture_height[view][1] = texture_height[view][2] = video_height[view];  
            break;
    }

    texture_format[view][0] = GL_LUMINANCE;
    texture_format[view][1] = GL_LUMINANCE;
    texture_format[view][2] = GL_LUMINANCE;

    texture_int_format[view][0] = GL_LUMINANCE;
    texture_int_format[view][1] = GL_LUMINANCE;
    texture_int_format[view][2] = GL_LUMINANCE;

    for (int i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE0+(3*view)+i);
        glBindTexture(GL_TEXTURE_2D, texture[view][i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_int_format[view][i], texture_width[view][i], 
            texture_height[view][i], 0, texture_format[view][i], GL_UNSIGNED_BYTE, 0);
    }
}

int Video_widget::format_code(int view) const
{
    int code = 0;
    switch (format[view])
    {
        case Media::YV12:
        case Media::I420:
        case Media::I422:
        case Media::I444:
            code = 0;
            break;

        case Media::YUY2:	
            code = 1;		
            break;

        case Media::UYVY:
            code = 2;
            break;
    }
    return code;
}
    
Abstract_buffer_manager* Video_widget::get_buffer_manager(int view)
{
    return (view == 0)?&buff_manager1:&buff_manager2;
}
