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

#include <buffer.h>
#include <media_debug.h>
#include <audio_sample_cloner.h>

const Port Audio_sample_cloner::input_port[] = {{Media::AUDIO_PCM, "pcm"}};
const Port Audio_sample_cloner::output_port[] = {{Media::AUDIO_PCM, "pcm"}, {Media::AUDIO_PCM, "sample_pcm"}};

Audio_sample_cloner::Audio_sample_cloner(const char* _name)
    :Abstract_media_object(_name)
    , packet_count(0)
    , queue(10)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
    create_output_ports(output_port, 2);
}

Audio_sample_cloner::~Audio_sample_cloner()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Audio_sample_cloner::run()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::state state = Media::stop;
    /*while (is_running)
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

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }*/
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

Media::status Audio_sample_cloner::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Audio_sample_cloner::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Audio_sample_cloner::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    //is_running = 1;
    //thread.start(this);
    return Media::ok;
}

Media::status Audio_sample_cloner::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    //is_running = 0;
    cv.signal();
    //thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Audio_sample_cloner::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (Media::play == get_state())
    {
        ++packet_count;
        if (0 == (packet_count % 10))
        {
            Buffer* buff_clone = buffer->split(0, buffer->get_buffer_size(), buffer->type(), buffer->get_parameter_size());
            memcpy(buff_clone->parameter(), buffer->parameter(), buffer->get_parameter_size());
            buff_clone->set_pts(buffer->pts());
            buff_clone->set_data_size(buffer->get_data_size());
            push_data(1, buff_clone);
        }
        push_data(0, buffer);
        MEDIA_LOG("%s, Buffer: 0x%llx, pts: %llu, Status: %d", object_name(), (unsigned long long)buffer, buffer->pts(), status);
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
