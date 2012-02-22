/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AUDIO_DEINTERLEAVER_H_
#define _AUDIO_DEINTERLEAVER_H_

#include <media.h>
#include <thread.h>
#include <priority_queue.h>
#include <pcm_parameters.h>
#include <abstract_media_object.h>

class Audio_deinterleaver:public Abstract_media_object
{
public:
    friend class Thread<Audio_deinterleaver>;

    Audio_deinterleaver(const char* _name);
    ~Audio_deinterleaver();

protected:
    int run();

    Media::status on_start(int start_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

private:
    int is_running;
    Condition_variable cv;
    Thread<Audio_deinterleaver> thread;
    Priority_queue<unsigned long long, Buffer*> queue;

    const static Port input_port[];
    const static Port output_port[];
};

#endif
