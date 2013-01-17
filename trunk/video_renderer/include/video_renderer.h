/*
 *  Copyright (C) 2012 Prem Sasidharan.
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
#include <abstract_media_object.h>

#include <child_clock.h>

class Video_widget;

class Abstract_text_helper
{
public:
    virtual void read_text(char*, int, uint64_t) = 0;
};

class Video_renderer:public Abstract_media_object
{
public:
    friend class Thread<Video_renderer>;

    Video_renderer(const char* _name, Child_clock* clk);
    ~Video_renderer();

public:
    int current_position() const;

	void update_pts_text();
	void set_render_widget(Video_widget* _window);

    void register_text_helper(Abstract_text_helper* helper);
    void unregister_text_helper();

protected:
    int run();

	void flush();

    Media::status on_start(int start_time, int end_time);
    Media::status on_stop(int end_time);

    Media::status on_connect(int port, Abstract_media_object* pobj);
    Media::status on_disconnect(int port, Abstract_media_object* pobj);

    Media::status input_data(int port, Buffer* buffer);

    void play_video();

private:
    int curr_pos;
    int is_running;
	int view_count;
    Buffer* prev[2];
    mutable Mutex mutex;
    Video_widget* window;
    Condition_variable cv;
    Child_clock* child_clk;
    Condition_variable stop_cv;
    Thread<Video_renderer> thread;
    Priority_queue<unsigned long long, Buffer*> queue1;
    Priority_queue<unsigned long long, Buffer*> queue2;
	Priority_queue<unsigned long long, Buffer*>* queue[2];
	int video_end;
	int video_start;    

    Abstract_text_helper* text_helper;

    enum { MAX_DISP_TEXT_LENGTH = 100 };
    char disp_text[MAX_DISP_TEXT_LENGTH];

    const static Port input_port[];
};

#endif
