/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _YUV_PLAYER_H_
#define _YUV_PLAYER_H_

#include <QTimer>
#include <QSlider>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QMainWindow>

#include <yuv_dlg.h>

#include <master_clock.h>

#include <video_widget.h>
#include <yuv_file_src.h>
#include <video_renderer.h>

#include <ui_main_window.h>

class Yuv_player;

class Text_helper: public Abstract_text_helper
{
public:
    Text_helper(Yuv_player* p);
    ~Text_helper();

public:
    void read_text(char* text, int length, uint64_t time);

private:
    Yuv_player* player;
};

class Yuv_player:public QMainWindow, public Ui_yuv_player, public Observer
{
    Q_OBJECT
public:
    friend class Text_helper;

    enum Text_mode {no_text, time_code, frame_count};

    Yuv_player();
    ~Yuv_player();
    
public:
    int stop(int& time);
    int start(int start, int end);

    float fps() const;
    int duration() const;

    void set_text_mode(Text_mode mode);
    int set_parameters(int width, int height, Media::type fmt, float fps, const char* path);
    
protected:
    void init();
	void init_player();
	void init_actions();
    void connect_signals_slots();
    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);

    void closeEvent(QCloseEvent* event);

protected slots:
    void time_out();
	void file_open();
	void help_about();
	void change_screen_size();
	void show_hide_progress_bar();
	void playback_control(int status);
	void change_disp_mode(QAction* action);
	void change_text_mode(QAction* action);
	void slider_seek(uint64_t _start, uint64_t _end);

private:
    int trick_mode;

    QTimer timer;

	Yuv_dlg dlg;

    Media::state state;
    Master_clock master;
    
    Yuv_file_src source;
    Video_renderer sink;
    
    Text_mode text_mode;
    Text_helper text_helper;

	QActionGroup* mode_grp;
	QActionGroup* text_grp;
};

#endif
