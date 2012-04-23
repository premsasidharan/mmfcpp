/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _XV_RENDERER_H
#define _XV_RENDERER_H

#include <media.h>
#include <thread.h>
#include <priority_queue.h>
#include <yuv_parameters.h>
#include <abstract_media_object.h>

#include <yuv_window.h>

class Xv_renderer:public Abstract_media_object
{
public:
    friend class Thread<Xv_renderer>;

    Xv_renderer(const char* _name, Yuv_window* _window);
    ~Xv_renderer();

protected:
    int run();

    Media::status on_start(int start_time, int end_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

    void play_video();

private:
    Buffer* prev;
    int is_running;
    Yuv_window* window;
    Condition_variable cv;
    Condition_variable stop_cv;
    Thread<Xv_renderer> thread;
    Priority_queue<unsigned long long, Buffer*> queue;

    const static Port input_port[];
};

#endif
