/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AVCODEC_AUDIO_DECODER_H
#define _AVCODEC_AUDIO_DECODER_H

#include <stdio.h>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include <media.h>
#include <thread.h>
#include <av_parameters.h>
#include <priority_queue.h>
#include <abstract_media_object.h>

class Avcodec_audio_decoder:public Abstract_media_object
{
public:
    friend class Thread<Avcodec_audio_decoder>;

    Avcodec_audio_decoder(const char* _name);
    ~Avcodec_audio_decoder();

protected:
    Media::status on_start(int start_time, int end_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

protected:
    int run();
    void decode();
    void close_codec();
    int check_and_update_codec(AVCodecContext* codec_ctx);

private:
    int is_running;
    char* file_path;
    Condition_variable cv;
    Thread<Avcodec_audio_decoder> thread;
    Priority_queue<unsigned long long, Buffer*> queue;

    AVCodecContext* audio_ctx;

    const static Port input_port[];
    const static Port output_port[];
};

#endif
