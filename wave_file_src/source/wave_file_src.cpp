/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <buffer.h>
#include <media_debug.h>
#include <wave_file_src.h>

const Port Wave_file_src::output_port[] = {{Media::AUDIO_PCM, "pcm"}};

Wave_file_src::Wave_file_src(const char* _name)
    :Abstract_media_object(_name)
    , is_running(0)
    , packet_size(0)
    , sample_count(0)
    , packet_count(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    create_output_ports(output_port, 1);
}

Wave_file_src::~Wave_file_src()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Wave_file_src::set_file_path(const char* path)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Path: %s", object_name(), path);
    if (1 == file.open(path))
    {
		packet_count = 0;
        packet_size = 1024*file.frame_size();
        return 1;
    }
    return 0;
}

int Wave_file_src::channels() const
{
    return file.channel_count();
}

int Wave_file_src::duration() const
{
    int frame_rate = file.sample_rate();
	if (0 != frame_rate)
    {
        return (int)(100000.0*((double)file.frames_count()/(double)frame_rate));
    }
    return 0;
}

int Wave_file_src::run()
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
                process_wave_file();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

void Wave_file_src::process_wave_file()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    int data_size = 0;
    Buffer* buffer = Buffer::request(packet_size, Media::AUDIO_PCM, sizeof(Pcm_param));
    if (file.read((unsigned char*)buffer->data(), packet_size, data_size))
    {
        sample_count += data_size;
        data_size *= file.frame_size();
        buffer->set_pts(packet_count++);
        buffer->set_data_size(data_size);

        Pcm_param* param = (Pcm_param *) buffer->parameter();
        param->channel_count = file.channel_count();
        param->samples_per_sec = file.sample_rate();
        param->avg_bytes_per_sec = file.sample_rate()*file.channel_count()*(file.bits_per_sample()/8);
        param->bits_per_sample = file.bits_per_sample();

        buffer->set_flags((packet_count == 1)?FIRST_PKT:0);
        if (sample_count >= file.frames_count())//data_size != packet_size)
        {
            buffer->set_flags(buffer->flags()|LAST_PKT);
            //file.close();
            MEDIA_WARNING("%s, last packet", object_name());
        }
        push_data(0, buffer);

        if (sample_count >= file.frames_count())//data_size != packet_size)
        {
            set_state(Media::stop);
        }
    }
    else
    {
        Buffer::release(buffer);
        buffer = 0;
        //MEDIA_ERROR("%s, File read failed", object_name());
    }
}

Media::status Wave_file_src::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (start_time >= 0)
    {
        int offset = (int)((double)start_time*(double)file.sample_rate()/100000.0);
        sample_count = offset; //TODO:
        packet_count = offset;
        file.seek(offset, SEEK_SET);
    }
    else
    {
        MEDIA_ERROR("Error, Invalid start time: %d", start_time);
    }
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Wave_file_src::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    return Media::ok;
}

Media::status Wave_file_src::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 1;
    thread.start(this);
    return Media::ok;
}

Media::status Wave_file_src::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}
