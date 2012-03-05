/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AUDIO_RENDERER_H
#define _AUDIO_RENDERER_H

#include <alsa/asoundlib.h>

#include <media.h>
#include <thread.h>
#include <priority_queue.h>
#include <pcm_parameters.h>
#include <abstract_media_object.h>

class Audio_renderer:public Abstract_media_object
{
public:
    friend class Thread<Audio_renderer>;

    Audio_renderer(const char* _name, const char* _device, int enable_output = 0);
    ~Audio_renderer();

public:
	int current_position() const;

protected:
    int run();
    void play_audio();
    snd_pcm_format_t format(unsigned int bits_per_sample);

    Media::status on_start(int start_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

private:
    char* device;
    int is_running;
    snd_pcm_t* pcm_handle;
    Condition_variable cv;
    Condition_variable stop_cv;
    Thread<Audio_renderer> thread;
    Priority_queue<unsigned long long, Buffer*> queue;

	mutable Mutex mutex;

    int error;
    int is_output;
    int sample_count;
	double curr_time;
    unsigned int channels;
    unsigned int bits_per_sample;
    unsigned int samples_per_sec;

    const static Port input_port[];
    const static Port output_port[];
};

#endif
