/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AV_FILE_SRC_H
#define _AV_FILE_SRC_H

#include <stdio.h>

extern "C"
{
#include <libavformat/avformat.h>
}

#include <media.h>
#include <thread.h>
#include <av_parameters.h>
#include <abstract_media_object.h>

class Av_file_src:public Abstract_media_object
{
public:
    friend class Thread<Av_file_src>;

    Av_file_src(const char* _name);
    ~Av_file_src();

public:
    int set_file_path(const char* path);

protected:
    Media::status on_start(int start_time, int end_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

protected:
    int run();
    int process_av_file();
    static int av_lock_callback(void **mutex, enum AVLockOp op);

private:
    int is_running;
    char* file_path;
    Condition_variable cv;
    Thread<Av_file_src> thread;

    int audio_stream;
    int video_stream;
    AVFormatContext* fmt_ctx;
    AVCodecContext* audio_ctx;
    AVCodecContext* video_ctx;

    int audio_pkt_count;
    int video_pkt_count;

    long long int audio_duration;
    long long int video_duration;

    const static Port output_port[];
};

#endif
