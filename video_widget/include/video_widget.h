/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _VIDEO_WIDGET_H_
#define _VIDEO_WIDGET_H_

#include <GL/glew.h>

#include <QMutex>
#include <QGLWidget>
#include <QGLShaderProgram>

#include <gl_buffer_manager.h>

class Video_widget:public QGLWidget
{
    Q_OBJECT
public:
	enum State {Init, Pause, Play};
	enum Pos {TL, TR, BL, BR, FS};
	enum Mode {Y, U, V, R, G, B, RGB, GRID_NYUV, GRID_NRGB};
    enum {BLEND = 0x0100, SPLIT = 0x0200};
    enum Stereo_mode {NONE = 0x00, VIEW_0 = BLEND|0x01, VIEW_1 = BLEND|0x02, ADD = BLEND|0x03, DIFF_1_2 = BLEND|0x04, INT_LEAVE = SPLIT|0x01,
                      HORZ_SPLIT_1_2 = SPLIT|0x02, HORZ_SPLIT_2_1 = SPLIT|0x03, VERT_SPLIT_1_2 = SPLIT|0x04, VERT_SPLIT_2_1 = SPLIT|0x05};

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

    State playback_state() const { return pb_state; };
	void set_playback_control_state(State state);

	void show_text(const char* text = 0);

    void show_frame(int view, int fmt, int width, int height, uint8_t* yuv, GLuint gl_buff = 0);
    
    Abstract_buffer_manager* get_buffer_manager(int view);

signals:
    void update_frame();
    void renderer_close();
	void pb_control(int status);
	void seek(uint64_t start, uint64_t end);

protected:
	void init();
    void paintGL();
    void init_shader();
    void initializeGL();
    void resizeGL(int width, int height);
    
    bool init_yuv_rgb_shader();
    bool init_yuv_rgb_blend_shader();
    bool init_yuv_rgb_split_shader();

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

    void setup_shader(int i, int sid, Video_widget::Mode mode);
    void texture_ids(int fmt, int& id0, int& id1, int& id2);

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
    QGLShaderProgram* program[3];

    QGLShaderProgram yuv_rgb;
    QGLShaderProgram yuv_rgb_blend;
    QGLShaderProgram yuv_rgb_split;

    enum {MAX_TEXTURE_COUNT = 3};

    int char_width;
    int char_height;
    char* disp_text;
    GLfloat font_color[3];

    GLuint gl_buffer[2];
    GLuint font_offset;

	bool slide_flag;

    GLuint texture[2][MAX_TEXTURE_COUNT];
    GLenum texture_format[2][MAX_TEXTURE_COUNT];
    GLsizei texture_width[2][MAX_TEXTURE_COUNT];
    GLsizei texture_height[2][MAX_TEXTURE_COUNT];
    GLint texture_int_format[2][MAX_TEXTURE_COUNT];

    unsigned char* texture_data[2][MAX_TEXTURE_COUNT];
    
    Gl_buffer_manager buff_manager1;
    Gl_buffer_manager buff_manager2;

    static const GLfloat pos_coeff[][4];
    static const GLfloat yuv_coeff[][7];
    static const GLfloat rgb_coeff[][12];
    static const GLfloat blend_coeff[][6];

    static const char vertex_shader_text[];
    static const char yuv_rgb_shader_text[];
    static const char yuv_rgb_blend_shader_text[];
    static const char yuv_rgb_split_shader_text[];
};

#endif
