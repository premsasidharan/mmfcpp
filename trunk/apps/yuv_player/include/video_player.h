/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include <QTimer>
#include <QSlider>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>

#include <master_clock.h>

#include <video_widget.h>
#include <yuv_file_src.h>
#include <video_renderer.h>

#include <ui_player.h>

class Video_player;

class Text_helper: public Abstract_text_helper
{
public:
    Text_helper(Video_player* p);
    ~Text_helper();

public:
    void read_text(char* text, int length, uint64_t time);

private:
    Video_player* player;
};

class Video_player:public QWidget, public Ui_player, public Observer
{
    Q_OBJECT
public:
    friend class Text_helper;

    enum Text_mode {no_text, time_code, frame_count};

    Video_player();
    ~Video_player();
    
public:
    void show();
    
    int stop(int& time);
    int start(int start, int end);

    float fps() const;
    int duration() const;

    void set_text_mode(Text_mode mode);
    int set_parameters(int width, int height, Media::type fmt, float fps, const char* path);
    
protected:
    void initialize();
    void connect_signals_slots();

    void show_extra_controls(bool ok);
    void resizeEvent(QResizeEvent* event);

    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);

protected slots:
    void time_out();
    void play_pause();
    void slider_pressed();
    void slider_released();
    void more_controls(int state);
    void mode_change(bool status);
    void text_mode_change(bool status);

private:
    int trick_mode;

    QTimer timer;
    
    Text_mode text_mode;
    Text_helper text_helper;

    Media::state state;
    Master_clock master;
    
    Video_widget window;
    Yuv_file_src source;
    Video_renderer sink;
};

#endif
