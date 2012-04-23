/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <media.h>
#include <buffer.h>
#include <media_debug.h>
#include <track_viewer.h>

const Port Track_viewer::input_port[] = {{Media::AUDIO_DEINT_PCM, "pcm"}};

Track_viewer::Track_viewer(const char* _name)
    :Abstract_media_object(_name)
    , is_running(0)
    , queue(10)
    , curr_buff(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
}

Track_viewer::~Track_viewer()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Track_viewer::run()
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
                cv.wait();
                break;

            case Media::pause:
                MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
                cv.wait();
                break;

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                display();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

Media::status Track_viewer::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Track_viewer::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Track_viewer::on_pause(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::pause);
    cv.signal();
    return Media::ok;
}

Media::status Track_viewer::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
    return Media::ok;
}

Media::status Track_viewer::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (is_running)
    {
        is_running = 0;
        cv.signal();
        thread.join();
    }
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Track_viewer::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (Media::play == get_state())
    {
        queue.push(buffer->pts(), buffer, 500);
    }
    else
    {
        Buffer::release(buffer);
        //sleep(1);
        MEDIA_WARNING("Packet received under non-play state: %s, Buffer: 0x%llx, pts: %llu", object_name(), (unsigned long long)buffer, buffer->pts());
        return Media::non_play_state;
    }
    return Media::ok;
}

void Track_viewer::display()
{
    Buffer* buff = queue.pop(500);
    if (0 != buff)
    {
        mutex.lock();
        if (0 != curr_buff)
        {
            Buffer::release(curr_buff);
        }
        curr_buff = buff;
        if (buff->flags() & LAST_PKT)
        {
            set_state(Media::stop);
        }
        mutex.unlock();
    }
}

Buffer* Track_viewer::get_buffer()
{
    Buffer* buff = 0;
    mutex.lock();
    if (0 != curr_buff)
    {
        buff = curr_buff->clone();
    }
    mutex.unlock();
    return buff;
}
