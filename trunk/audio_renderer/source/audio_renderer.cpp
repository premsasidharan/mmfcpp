/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <buffer.h>
#include <media_debug.h>
#include <audio_renderer.h>

const Port Audio_renderer::input_port[] = {{Media::AUDIO_PCM/*, 0*/, "pcm"}};

Audio_renderer::Audio_renderer(const char* _name, const char* _device)
    :Abstract_media_object(_name)
    , device(0)
    , is_running(0)
    , pcm_handle(0)
    , queue(10)
    , error(-1)
    , channels(0)
    , bits_per_sample(0)
    , samples_per_sec(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", _name, _device);
    int size = strlen(_device)+1;
    device = new char[size];
    strncpy(device, _device, size);
    create_input_ports(input_port, 1);
}

Audio_renderer::~Audio_renderer()
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    delete [] device;
    device = 0;
}

int Audio_renderer::run()
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
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

            case Media::pause:
                MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
                cv.wait();
                break;

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                play_audio();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

void Audio_renderer::play_audio()
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    Buffer* buffer = queue.pop(500);
    if (0 != buffer)
    {
        Pcm_param* parameter = (Pcm_param*) buffer->parameter();
        //MEDIA_ERROR(": %s, Buffer: %llx, pts: %llu State: %s", object_name(),
        //	(unsigned long long)buffer, buffer->pts(), "PLAY");

        if (channels != parameter->channel_count ||
        bits_per_sample != parameter->bits_per_sample ||
        samples_per_sec != parameter->samples_per_sec)
        {
            channels = parameter->channel_count;
            bits_per_sample = parameter->bits_per_sample;
            samples_per_sec = parameter->samples_per_sec;

            if (0 == pcm_handle)
            {
                error = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
            }

            if (error >= 0)
            {
                error = snd_pcm_set_params(pcm_handle, format(bits_per_sample), SND_PCM_ACCESS_RW_INTERLEAVED, channels, samples_per_sec, 0, 0);
            }

            if (error >= 0)
            {
                error = snd_pcm_prepare(pcm_handle);
            }
        }

        //TODO: AVSync
        if (error >= 0)
        {
            unsigned int samples = buffer->get_data_size()/(channels*(bits_per_sample/8));
            if (samples > 0)
            {
                error = snd_pcm_writei(pcm_handle, buffer->data(), samples);
                if (error < 0)
                {
                    error = snd_pcm_recover(pcm_handle, samples, 0);
                }
            }
        }

        if (error < 0)
        {
            fprintf (stderr, "\n\tALSA Error: %s\n", snd_strerror (error));
        }

        if (buffer->flags() & LAST_PKT)
        {
            set_state(Media::stop);
            if (error >= 0)
            {
                MEDIA_WARNING("%s", "Closing");
                error = snd_pcm_close(pcm_handle);
                pcm_handle = 0;
            }
        }

        Buffer::release(buffer);
        buffer = 0;
    }
    else
    {
        MEDIA_ERROR("Audio Renderer: %s, Null Buffer", object_name());
    }
}

Media::status Audio_renderer::on_start(int start_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Audio_renderer::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    set_state(Media::stop);
    cv.signal();
    //stop_cv.wait();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Audio_renderer::on_pause(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    set_state(Media::pause);
    cv.signal();
    return Media::ok;
}

Media::status Audio_renderer::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    is_running = 1;
    thread.start(this);
    return Media::ok;
}

Media::status Audio_renderer::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Audio_renderer::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    int status = 0;
    if (Media::play == get_state())
    {
        status = queue.push(buffer->pts(), buffer, 500);
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

snd_pcm_format_t Audio_renderer::format(unsigned int bits_per_sample)
{
    MEDIA_TRACE_OBJ_PARAM("%s, device: %s", object_name(), device);
    snd_pcm_format_t fmt = SND_PCM_FORMAT_U8;

    switch (bits_per_sample)
    {
        case 8:
            fmt = SND_PCM_FORMAT_U8;
            break;
        case 16:
            fmt = SND_PCM_FORMAT_S16_LE;
            break;
        case 24:
            fmt = SND_PCM_FORMAT_U24_LE;
            break;
        case 32:
            fmt = SND_PCM_FORMAT_U32_LE;
            break;
    }
    return fmt;
}
