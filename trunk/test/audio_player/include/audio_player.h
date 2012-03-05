/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AUDIO_PLAYER_H
#define _AUDIO_PLAYER_H

#include <QObject>

#include <player_window.h>

#include <track_viewer.h>
#include <wave_file_src.h>
#include <audio_renderer.h>
#include <audio_deinterleaver.h>
#include <audio_sample_cloner.h>

class Audio_player:public QObject, public Observer
{
    Q_OBJECT
public:
    Audio_player();
    ~Audio_player();

public:
    void show();
    
    int stop(int& time);
    int start(int time);

    int channels() const;
    int duration() const;
    int current_position() const;
    int set_file_path(const char* path);
    void set_pcm_widget(Pcm_widget* widget);

    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);
    
protected slots:
    void on_timer_elapsed();

private:
    Wave_file_src src;
    Audio_renderer sink;
    Audio_deinterleaver deinter;
    Track_viewer track_viewer;
    
    QTimer timer;
    Player_window window;
};

#endif

