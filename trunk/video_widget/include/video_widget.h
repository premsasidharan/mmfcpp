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
	enum State {Pause, Play};
	enum Pos {TL, TR, BL, BR, FS};
	enum Mode {Y, U, V, R, G, B, RGB, GRID_NYUV, GRID_NRGB};

    Video_widget(QWidget* parent = 0);
    ~Video_widget();

public:
	uint64_t slider_value();
	void set_slider_value(uint64_t pos);
	void set_slider_range(uint64_t _start, uint64_t _end);

	bool is_controls_visible();
	void show_playback_controls(bool en);

    void set_display_mode(Mode m);

	void set_stereo_mode(int mode);

	void set_playback_control_state(State state);

	void show_text(const char* text = 0);

	void set_views(int views);
    void show_frame(int view, int fmt, int width, int height, uint8_t* yuv);

signals:
    void update_frame();
    void renderer_close();
	void pb_control(int status);
	void seek(uint64_t start, uint64_t end);

protected:
	void init();
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

    void create_font_disp_lists();

    void render_text();
	void render_slider();
	void render_playback_controls();
    void render_frame(Pos pos, Mode mode);
	void render_notch(float x, float y, float width, float height);
	void render_triangle(float x, float y, float width, float height);
	void render_rectangle(float x, float y, float width, float height);

    void closeEvent(QCloseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event); 
	void mouseReleaseEvent(QMouseEvent* event);

    void create_textures();
    void create_yuy2_textures(int view);
    void create_i420_textures(int view);

    int format_code(int view) const;

	void set_texture_data(int i, uint8_t* yuv);

    void draw_text(GLfloat x, GLfloat y, const char* text);

private:
    QMutex mutex;

	int view_count;
	int stereo_mode;

    int format[2];
    int video_width[2];
    int video_height[2];

    Mode mode;
	uint64_t end;
	uint64_t start;
	State pb_state;
    bool is_changed;
	bool is_visible;

    float scale;
    int texture_count[2];
    QGLShaderProgram program;

    enum {MAX_TEXTURE_COUNT = 3};

    int char_width;
    int char_height;
    char* disp_text;
    GLfloat font_color[3];

    GLuint font_offset;

	bool slide_flag;

    GLuint texture[2][MAX_TEXTURE_COUNT];
    GLenum texture_format[2][MAX_TEXTURE_COUNT];
    GLsizei texture_width[2][MAX_TEXTURE_COUNT];
    GLsizei texture_height[2][MAX_TEXTURE_COUNT];
    GLint texture_int_format[2][MAX_TEXTURE_COUNT];

    unsigned char* texture_data[2][MAX_TEXTURE_COUNT];

    static const char shader_program[];
};

#endif
