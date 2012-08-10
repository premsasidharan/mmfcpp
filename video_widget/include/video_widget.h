/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _VIDEO_WIDGET_H_
#define _VIDEO_WIDGET_H_

#include <QMutex>
#include <QGLWidget>
#include <QGLShaderProgram>

class Video_widget:public QGLWidget
{
    Q_OBJECT
public:
    Video_widget(QWidget* _control, QWidget* parent = 0);
    ~Video_widget();

public:
    void show_frame(unsigned char* _yuv, int fmt, int width, int height);

signals:
    void update_frame();
    void renderer_close();

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

    void moveEvent(QMoveEvent* event);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event); 
    void keyPressEvent(QKeyEvent* event);

	void delete_textures();
	void create_textures();
	void create_yuy2_textures();
	void create_i420_textures();
	void create_uyvy_textures();

	int format_code() const;

private:
    QMutex mutex;
    QWidget* controls;

    int format;
	bool is_changed;
    int video_width;
    int video_height;

    float scale;
	int texture_count;
    QGLShaderProgram program;

	enum TEXTURE_COUNT {MAX_TEXTURE_COUNT = 3};

	GLuint texture[MAX_TEXTURE_COUNT];
	GLenum texture_format[MAX_TEXTURE_COUNT];
	GLsizei texture_width[MAX_TEXTURE_COUNT];
	GLsizei texture_height[MAX_TEXTURE_COUNT];
	GLint texture_int_format[MAX_TEXTURE_COUNT];
    
	unsigned char* texture_data[MAX_TEXTURE_COUNT];

	static const char shader_program[];
};

#endif
