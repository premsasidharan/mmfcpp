/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _TRACK_VIEWER_H_
#define _TRACK_VIEWER_H_

#include <stdint.h>

#include <media.h>
#include <thread.h>
#include <priority_queue.h>
#include <pcm_parameters.h>
#include <abstract_media_object.h>

#include <pcm_widget.h>

class Track_viewer:public Abstract_media_object
{
public:
    friend class Thread<Track_viewer>;

    Track_viewer(const char* _name);
    ~Track_viewer();
    
public:
    Buffer* get_buffer();

protected:
    int run();

    Media::status on_start(int start_time);
    Media::status on_stop(int end_time);
    Media::status on_pause(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);
    
    void display();

private:
    int is_running;
    Condition_variable cv;
    Thread<Track_viewer> thread;
    Priority_queue<unsigned long long, Buffer*> queue;
    
    Mutex mutex;
    Buffer* curr_buff;
    
    const static Port input_port[];
};

#endif
