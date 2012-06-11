/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _VIDEO_RENDERER_H_
#define _VIDEO_RENDERER_H_

#include <media.h>
#include <thread.h>
#include <priority_queue.h>
#include <yuv_parameters.h>
#include <abstract_media_object.h>

#include <child_clock.h>
#include <video_widget.h>

class Video_renderer:public Abstract_media_object
{
public:
    friend class Thread<Video_renderer>;

    Video_renderer(const char* _name, Child_clock* clk, Video_widget* _window);
    ~Video_renderer();

public:
    int current_position() const;

protected:
    int run();

    Media::status on_start(int start_time, int end_time);
    Media::status on_stop(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

    void play_video();

private:
    Buffer* prev;
    int curr_pos;
    int is_running;
    mutable Mutex mutex;
    Video_widget* window;
    Condition_variable cv;
    Child_clock* child_clk;
    Condition_variable stop_cv;
    Thread<Video_renderer> thread;
    Priority_queue<unsigned long long, Buffer*> queue;

    const static Port input_port[];
};

#endif
