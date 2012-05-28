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
#include <QHBoxLayout>
#include <QPushButton>

#include <master_clock.h>

#include <video_widget.h>
#include <yuv_file_src.h>
#include <video_renderer.h>

class Video_player:public QWidget, public Observer
{
    Q_OBJECT
public:
    Video_player();
    ~Video_player();
    
public:
    void show();
    
    int start(int start, int end);
    int stop(int& time);
    int duration() const;
    int set_parameters(int width, int height, Media::type fmt, float fps, const char* path);
    
protected:
    void initialize();
    void connect_signals_slots();
    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);

protected slots:
    void on_timeout();
    void on_play_pause();
    void slider_pressed();
    void slider_released();

private:
    QTimer timer;
    QSlider slider;
    int trick_mode;
    QPushButton button;
    QHBoxLayout layout;
    
    Media::state state;
    Master_clock master;
    
    Video_widget window;
    Yuv_file_src source;
    Video_renderer sink;
};

#endif
