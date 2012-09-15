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
    Video_widget(QWidget* parent = 0);
    ~Video_widget();

public:
	uint64_t current_pos();
    void set_mode(int _mode);
	void set_value(uint64_t pos);
	bool is_progress_bar_enabled();
	void enable_progress_bar(bool en);
	void set_pb_control_status(int status);
	void set_slider_range(uint64_t _start, uint64_t _end);
    void show_frame(unsigned char* _yuv, int fmt, int width, int height, const char* text = 0);

signals:
    void update_frame();
    void renderer_close();
	void pb_control(int status);
	void seek(uint64_t start, uint64_t end);

protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

    void create_font_disp_lists();

    void render_text();
	void render_slider();
	void render_pb_controls();
    void render_frame(int disp_mode, int mode);
	void render_notch(float x, float y, float width, float height);
	void render_triangle(float x, float y, float width, float height);
	void render_rectangle(float x, float y, float width, float height);

    void moveEvent(QMoveEvent* event);
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event); 
	void mouseReleaseEvent(QMouseEvent* event);

    void create_textures();
    void create_yuy2_textures();
    void create_i420_textures();

    int format_code() const;

    void draw_text(GLfloat x, GLfloat y, const char* text);

private:
    QMutex mutex;

    int format;
    int video_width;
    int video_height;

    int mode;
	uint64_t end;
	uint64_t start;
	int pb_status;
    bool is_changed;

	bool show_pb;

    float scale;
    int texture_count;
    QGLShaderProgram program;

    enum {MAX_TEXTURE_COUNT = 3};

    int char_width;
    int char_height;
    char* disp_text;
    GLfloat font_color[3];

    GLuint font_offset;

	bool slide_flag;

    GLuint texture[MAX_TEXTURE_COUNT];
    GLenum texture_format[MAX_TEXTURE_COUNT];
    GLsizei texture_width[MAX_TEXTURE_COUNT];
    GLsizei texture_height[MAX_TEXTURE_COUNT];
    GLint texture_int_format[MAX_TEXTURE_COUNT];

    unsigned char* texture_data[MAX_TEXTURE_COUNT];

    static const char shader_program[];
};

#endif
