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
#include <yuv_file_src.h>

const Port Yuv_file_src::output_port[] = {{Media::YUY2|Media::YV12|Media::I420|Media::UYVY, "yuv"}};

Yuv_file_src::Yuv_file_src(const char* _name)
    :Abstract_media_object(_name)
    , start_flag(0)
    , trick_mode(0)
    , is_running(0)
    , frame_rate(0.0)
    , file(0)
    , data_size(0)
    , start_frame(0)
    , frame_count(0)
    , total_frames(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_output_ports(output_port, 1);
}

Yuv_file_src::~Yuv_file_src()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    delete file; file = 0;
}

int Yuv_file_src::set_parameters(const char* path, Media::type fmt, float fps, int width, int height)
{
    MEDIA_TRACE_OBJ_PARAM("%s width: %d, height: %d", object_name(), _width, _height);
    if (fps <= 0.0f)
    {
        MEDIA_ERROR("%s - Invalid frame rate", object_name());
        return -1;
    }
    mutex.lock();
    delete file;
    file = new Read_yuv_file(path, width, height, fmt);
    if (0 != file->open())
    {
        data_size = file->frame_size();
        total_frames = file->frame_count();
        file->close();
    }
    mutex.unlock();
    frame_rate = fps;
    MEDIA_LOG("No Frames: %llu, Frame Size: %u", total_frames, data_size);

    return 1;
}

int Yuv_file_src::duration() const
{
	if (frame_rate > 0.0f)
    {
        return (int)(100000.0*((double)total_frames/(double)frame_rate));
    }
    return 0;
}
    
int Yuv_file_src::run()
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
                MEDIA_ERROR("%s, State: %s", object_name(), "STOP");
                mutex.lock();
                if (0 != file)
                {
                    file->close();
                }
                mutex.unlock();
                stop_cv.signal();
                cv.wait();
                break;

            case Media::pause:
                MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
                cv.wait();
                break;

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                process_yuv_file();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

int Yuv_file_src::process_yuv_file()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    mutex.lock();
    if (0 == file)
    {
        MEDIA_ERROR("%s, Invalid file", object_name());
        set_state(Media::stop);
        mutex.unlock();
        return 0;
    }
    if (start_flag)
    {
        file->open();
        frame_count = start_frame;
        file->seek(start_frame, SEEK_SET);
        start_flag = 0;
    }
    Buffer* buffer = Buffer::request(data_size, file->format(), sizeof(Yuv_param));
    file->read(buffer->data(), data_size);
    mutex.unlock();
    if (frame_rate > 0.0f)
    {
        buffer->set_pts((int)(100000.0*((double)frame_count/(double)frame_rate)));
    }
    else
    {
        buffer->set_pts(0);
    }
    ++frame_count;
    Yuv_param* param = (Yuv_param *) buffer->parameter();
    param->width = file->video_width();
    param->height = file->video_height();
    if (frame_count == (start_frame+1))
    {
        buffer->set_flags(FIRST_PKT);
    }
    else
    {
        buffer->set_flags(0);
    }
    
    if (frame_count >= end_frame || frame_count == total_frames)
    {
        buffer->set_flags(buffer->flags()|LAST_PKT);
    }
    push_data(0, buffer);

    if ((total_frames == frame_count) || frame_count >= end_frame)
    {
        set_state(Media::stop);
    }

    return Media::ok;
}

Media::status Yuv_file_src::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    mutex.lock();
    start_flag = 1;
    end_frame = (int) ((double)frame_rate*(double)end_time/(double)100000.0);
    start_frame = (int) ((double)frame_rate*(double)start_time/(double)100000.0);
    mutex.unlock();
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Yuv_file_src::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    stop_cv.wait();
    return Media::ok;
}

Media::status Yuv_file_src::on_pause(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::pause);
    cv.signal();
    return Media::ok;
}

Media::status Yuv_file_src::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
    return Media::ok;
}

Media::status Yuv_file_src::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}
