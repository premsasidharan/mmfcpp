/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <buffer.h>
#include <media_debug.h>
#include <video_renderer.h>

const Port Video_renderer::input_port[] = 
    {{
        Media::YUY2 |
        Media::YV12 |
        Media::I420 |
        Media::I422 |
        Media::I444 |
        Media::UYVY, 
        "yuv"
    }};

Video_renderer::Video_renderer(const char* _name, Child_clock* clk)
    :Abstract_media_object(_name)
    , prev(0)
    , curr_pos(0)
    , is_running(0)
    , window(0)
    , child_clk(clk)
    , queue(5)
    , text_helper(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
}

Video_renderer::~Video_renderer()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (0 != prev)
    {
        Buffer::release(prev);
        prev = 0;
    }
}

void Video_renderer::play_video()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Buffer* buffer = queue.pop(2000);
    if (0 != buffer)
    {
        Yuv_param* parameter = (Yuv_param*) buffer->parameter();
        //MEDIA_ERROR(": %s, Buffer: %llx, pts: %llu (%dx%d) State: %s", object_name(),
        //	(unsigned long long)buffer, buffer->pts(), parameter->width, parameter->height, "PLAY");
        if (0 != text_helper)
        {
            text_helper->read_text(disp_text, 100, buffer->pts());
        }
        window->show_frame((unsigned char*)buffer->data(), buffer->type(), parameter->width, parameter->height, (0==text_helper)?0:disp_text);
        mutex.lock();
        curr_pos = buffer->pts();
        mutex.unlock();
        if (0 !=  prev)
        {
            Buffer::release(prev);
        }
        prev = buffer;
        child_clk->wait_for_sync(curr_pos);
        if (buffer->flags() & LAST_PKT)
        {
            set_state(Media::stop);
            Media_params params;
            memset(&params, 0, sizeof(Media_params));
            notify(Media::last_pkt_rendered, params);
        }
        //TODO:Free Last packet
    }
    else
    {
        MEDIA_ERROR("Video_renderer: %s, Null Buffer", object_name());
    }
}

int Video_renderer::run()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::state state = Media::stop;
    while (is_running)
    {
        state = get_state();
        switch (state)
        {
            case Media::init:
                MEDIA_LOG("%s, State: %s", object_name(), "INIT");
                cv.wait();
                break;

            case Media::stop:
                MEDIA_LOG("%s, State: %s", object_name(), "STOP");
                stop_cv.signal();
                cv.wait();
                break;

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                play_video();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

Media::status Video_renderer::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, start_time: %d", object_name(), start_time);
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Video_renderer::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    stop_cv.wait();
    MEDIA_LOG("Stop state: %s", object_name());
    return Media::ok;
}

Media::status Video_renderer::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 1;
    thread.start(this);
    return Media::ok;
}

Media::status Video_renderer::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Video_renderer::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    int status = 0;
    if (Media::play == get_state())
    {
        status = queue.push(buffer->pts(), buffer, 500);
        MEDIA_LOG("Video_renderer: %s, Buffer: 0x%llx, pts: %llu, Status: %d", object_name(), (unsigned long long)buffer, buffer->pts(), status);
    }
    else
    {
        Buffer::release(buffer);
        //sleep(1);
        MEDIA_ERROR("Packet received under non-play state: %s, Buffer: 0x%llx, pts: %llu", object_name(), (unsigned long long)buffer, buffer->pts());
        return Media::non_play_state;
    }
    return Media::ok;
}

int Video_renderer::current_position() const
{
	int time = 0;
	mutex.lock();
	time = curr_pos;
	mutex.unlock();
	return time;
}

void Video_renderer::set_render_widget(Video_widget* _window)
{
	mutex.lock();
	window = _window;
	mutex.unlock();
}

void Video_renderer::register_text_helper(Abstract_text_helper* helper)
{
    mutex.lock();
    text_helper = helper;
    mutex.unlock();
}

void Video_renderer::unregister_text_helper()
{
    mutex.lock();
    text_helper = 0;
    mutex.unlock();
}

