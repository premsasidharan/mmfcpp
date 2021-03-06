/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#ifndef _PLAYER_H
#define _PLAYER_H

#include <yuv_window.h>
#include <xv_renderer.h>
#include <av_file_src.h>
#include <audio_renderer.h>
#include <avcodec_audio_decoder.h>
#include <avcodec_video_decoder.h>

#include <QObject>

class Player:public QObject
{
    Q_OBJECT
public:
    Player(const QString path);
    ~Player();

public:
    int start();
    int stop();

protected slots:
    void on_renderer_close();

private:
    Yuv_window window;
    Av_file_src av_src;
    Xv_renderer video_sink;
    Audio_renderer audio_sink;
    Avcodec_audio_decoder audio_decoder;
    Avcodec_video_decoder video_decoder;
};

#endif
