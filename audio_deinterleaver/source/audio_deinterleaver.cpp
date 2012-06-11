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
#include <audio_deinterleaver.h>

const Port Audio_deinterleaver::input_port[] = {{Media::AUDIO_PCM, "pcm"}};
const Port Audio_deinterleaver::output_port[] = {{Media::AUDIO_DEINT_PCM, "pcm"}};

Audio_deinterleaver::Audio_deinterleaver(const char* _name)
    :Abstract_media_object(_name)
    , is_running(0)
    , queue(10)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
    create_output_ports(output_port, 1);
}

Audio_deinterleaver::~Audio_deinterleaver()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Audio_deinterleaver::run()
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

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                deinterleave();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

Media::status Audio_deinterleaver::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Audio_deinterleaver::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Audio_deinterleaver::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
    return Media::ok;
}

Media::status Audio_deinterleaver::on_disconnect(int port, Abstract_media_object* pobj)
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

Media::status Audio_deinterleaver::input_data(int port, Buffer* buffer)
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

void Audio_deinterleaver::deinterleave()
{
    Buffer* buffer = queue.pop(500);
    if (0 != buffer)
    {
        Pcm_param* param = (Pcm_param *) buffer->parameter();
        int frames = buffer->get_data_size()/((param->bits_per_sample/8)*(param->channel_count));
        Buffer* deint_buff = Buffer::request(frames*sizeof(uint32_t)*param->channel_count, Media::AUDIO_DEINT_PCM, buffer->get_parameter_size());
        memcpy(deint_buff->parameter(), buffer->parameter(), buffer->get_parameter_size());
        deint_buff->set_data_size(buffer->get_data_size());
        deint_buff->set_pts(buffer->pts());
        deint_buff->set_flags(buffer->flags());
        if (buffer->flags() & LAST_PKT)
        {
            set_state(Media::stop);
        }
        switch (param->bits_per_sample)
        {
            case 8:
                deinterleave_8bit_data((int32_t*)deint_buff->data(), (uint8_t*)buffer->data(), frames, param->channel_count);
                break;
            case 16:
                deinterleave_16bit_data((int32_t*)deint_buff->data(), (int16_t*)buffer->data(), frames, param->channel_count);
                break;
            case 24:
                deinterleave_24bit_data((int32_t*)deint_buff->data(), (uint8_t*)buffer->data(), frames, param->channel_count);
                break;
        }
        Buffer::release(buffer);
        push_data(0, deint_buff);
    }
}

void Audio_deinterleaver::deinterleave_8bit_data(int32_t* dest, uint8_t* src, int frames, int channels)
{
    int i, j;
    int32_t *chan_offset[10];
    for (i = 0; i < channels; i++)
    {
        chan_offset[i] = &dest[i*frames];
    }
    for (i = 0; i < frames; i++)
    {
        for (j = 0; j < channels; j++)
        {
            (*chan_offset[j]) = (*src);
            ++chan_offset[j];
            ++src;
        }
    }
}

void Audio_deinterleaver::deinterleave_16bit_data(int32_t* dest, int16_t* src, int frames, int channels)
{
    int i, j;
    int32_t *chan_offset[10];
    for (i = 0; i < channels; i++)
    {
        chan_offset[i] = &dest[i*frames];
    }
    for (i = 0; i < frames; i++)
    {
        for (j = 0; j < channels; j++)
        {
            (*chan_offset[j]) = (int)(*src);
            ++chan_offset[j];
            ++src;
        }
    }
}

void Audio_deinterleaver::deinterleave_24bit_data(int32_t* dest, uint8_t* src, int frames, int channels)
{
    
}
