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
#include <xv_renderer.h>
#include <xv_video_frame.h>

const Port Xv_renderer::input_port[] = {{Media::YUV420_PLANAR/*, 0*/, "yuv420"}};

Xv_renderer::Xv_renderer(const char* _name, X11_window* _window)
    :Abstract_media_object(_name)
    , prev(0)
    , is_running(0)
    , window(_window)
    , queue(5)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
}

Xv_renderer::~Xv_renderer()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (0 != prev)
    {
        Buffer::release(prev);
        prev = 0;
    }
}

void Xv_renderer::play_video()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Buffer* buffer = queue.pop(2000);
    if (0 != buffer)
    {
        I420_param* parameter = (I420_param*) buffer->parameter();
        //MEDIA_ERROR(": %s, Buffer: %llx, pts: %llu (%dx%d) State: %s", object_name(),
        //	(unsigned long long)buffer, buffer->pts(), parameter->width, parameter->height, "PLAY");
        window->show_frame((unsigned char*)buffer->data(), Xv_video_frame::video_fmt("I420"), parameter->width, parameter->height);
        if (0 !=  prev)
        {
            Buffer::release(prev);
        }
        prev = buffer;
        //TODO: AVSync
        usleep(41667); //TODO: hard coding
        if (buffer->flags() & LAST_PKT)
        {
            set_state(Media::stop);
        }
        //TODO:Free Last packet
    }
    else
    {
        MEDIA_ERROR("Xv_renderer: %s, Null Buffer", object_name());
    }
}

int Xv_renderer::run()
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
                //stop_cv.signal();
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

Media::status Xv_renderer::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, start_time: %d", object_name(), start_time);
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Xv_renderer::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    //stop_cv.wait();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Xv_renderer::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 1;
    thread.start(this);
    return Media::ok;
}

Media::status Xv_renderer::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Xv_renderer::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);

    int status = 0;
    //while (status == 0)
    {
        if (Media::play == get_state())
        {
            status = queue.push(buffer->pts(), buffer, 2000);
            //MEDIA_ERROR("Xv_renderer: %s, Buffer: 0x%llx, pts: %llu, Status: %d", object_name(), (unsigned long long)buffer, buffer->pts(), status);
        }
        else
        {
            Buffer::release(buffer);
            sleep(1);
            MEDIA_ERROR("Packet received under non-play state: %s, Buffer: 0x%llx, pts: %llu", object_name(), (unsigned long long)buffer, buffer->pts());
            return Media::non_play_state;
        }
    }
    return Media::ok;
}
