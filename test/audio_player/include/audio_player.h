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

#include <wave_file_src.h>
#include <audio_renderer.h>

class Audio_player:public QObject, public Observer
{
    Q_OBJECT
public:
    Audio_player(QObject* parent = 0);
    ~Audio_player();

public:
    int wait();
    int stop(int& time);
    int start(int time);

    int duration() const;
    int current_position() const;
    int set_file_path(const char* path);

    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);

private:
    Wave_file_src src;
    Audio_renderer sink;
};

#endif

