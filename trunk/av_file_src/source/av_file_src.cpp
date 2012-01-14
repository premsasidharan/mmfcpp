/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <buffer.h>
#include <media_debug.h>
#include <av_file_src.h>

extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int av_lock_callback(void **mutex, enum AVLockOp op);

const Port Av_file_src::output_port[] = {{Media::VIDEO_FFMPEG_PKT, "video"}, {Media::AUDIO_FFMPEG_PKT, "audio"}};

Av_file_src::Av_file_src(const char* _name)
	:Abstract_media_object(_name)
	, is_running(0)
	, file_path(0)
    , audio_stream(-1)
    , video_stream(-1)
    , fmt_ctx(0)
    , audio_ctx(0)
    , video_ctx(0)    
    , audio_pkt_count(0)
    , video_pkt_count(0)
    , audio_duration(-1)
    , video_duration(-1)
{
	MEDIA_TRACE_OBJ_PARAM("%s", _name);
	create_output_ports(output_port, 2);    
    av_register_all();
}

Av_file_src::~Av_file_src()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	delete [] file_path;
	file_path = 0;
    
    if (0 != fmt_ctx)
    {
        av_close_input_file(fmt_ctx);
        fmt_ctx = 0;
    }
}

int Av_file_src::set_file_path(const char* path)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	
    if (0 != file_path)
    {
        delete [] file_path;
	}
    file_path = new char[strlen(path)+1];
	strcpy(file_path, path);
    
	av_lockmgr_register(av_lock_callback);
    if (0 != fmt_ctx)
    {
        av_close_input_file(fmt_ctx);
        fmt_ctx = 0;
    }
    int status = 0;
    if (0 == av_open_input_file(&fmt_ctx, file_path, 0, 0, 0))
    { 	
        if (av_find_stream_info(fmt_ctx) < 0)
        {
            MEDIA_ERROR("Couldn't find stream information for %s", file_path);
        }
        else
        {
            for (int i = 0; i < (int)fmt_ctx->nb_streams; i++)
            {
                if (fmt_ctx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
                {
                    video_stream = i;
                    video_ctx = fmt_ctx->streams[i]->codec;
                    video_duration = fmt_ctx->streams[i]->duration;
                }
                else if (fmt_ctx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
                {
                    audio_stream = i;
                    audio_ctx = fmt_ctx->streams[i]->codec;
                    audio_duration = fmt_ctx->streams[i]->duration;
                }
            }
            if (-1 != audio_stream || -1 != video_stream)
            {
                status = 1;
            }
            else
            {
                MEDIA_ERROR("File %s doesn't contain audio and video streams", file_path);
            }
        }		
    }
    else
    {
        fmt_ctx = 0;
        MEDIA_ERROR("Failed to open file: %s", file_path);
    }
	return status;
}

int Av_file_src::run()
{
    fflush(stdin);
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
				MEDIA_WARNING("%s, State: %s", object_name(), "STOP");
				cv.wait();
				break;

			case Media::pause:
				MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
				cv.wait();
				break;

			case Media::play:
				//MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
				process_av_file();
				break;

			default:
				MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
				break;
		}
	}
	MEDIA_WARNING("Exiting Thread: %s", object_name());
	return 0;
}

int Av_file_src::process_av_file()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    
    Buffer* buffer = Buffer::request(sizeof(AVPacket), 0, sizeof(Av_param));
    AVPacket* packet = (AVPacket *) buffer->data();
    memset(packet, 0, sizeof(AVPacket));
    int status = av_read_frame(fmt_ctx, packet);
    
    if (0 == status)
    {
        buffer->set_data_size(sizeof(AVPacket));
        buffer->set_pts(packet->pts);
        buffer->set_flags(0);
        if (packet->stream_index == video_stream)
        {
            MEDIA_LOG("Video Packet pts: %ld, Duration: %ld, dts: %ld, Size: %d, duration: %d", packet->pts, packet->duration, packet->dts, packet->size, video_duration);
            if (0 == video_pkt_count)
            {
                buffer->set_flags(FIRST_PKT);
                MEDIA_LOG("First Packet:%s", "Video");
            }
            ++video_pkt_count;
            buffer->set_type(Media::VIDEO_FFMPEG_PKT);
            if (video_duration <= (packet->pts+packet->duration))
            {
                buffer->set_flags(buffer->flags()|LAST_PKT);
                MEDIA_LOG("Last Packet:%s", "Video");
            }
            ((Av_param *)buffer->parameter())->codec_ctx = video_ctx;
            push_data(0, buffer);
        }
        else if (packet->stream_index == audio_stream)
        {
            MEDIA_LOG("Audio Packet pts: %ld, Duration: %ld, dts: %ld, Size: %d, duration: %d", packet->pts, packet->duration, packet->dts, packet->size, audio_duration);
            if (0 == audio_pkt_count)
            {
                buffer->set_flags(FIRST_PKT);
                MEDIA_LOG("First Packet:%s", "Audio");
            }
            ++audio_pkt_count;
            buffer->set_type(Media::AUDIO_FFMPEG_PKT);
            if (audio_duration <= (packet->pts+packet->duration))
            {
                buffer->set_flags(buffer->flags()|LAST_PKT);
                MEDIA_LOG("Last Packet:%s", "Audio");
            }
            ((Av_param *)buffer->parameter())->codec_ctx = audio_ctx;
            push_data(1, buffer);
        }
        else
        {
            av_free_packet(packet);
            Buffer::release(buffer);
            buffer = 0;
        }
    }
    else
    {
        Buffer::release(buffer);
        buffer = 0;
        //TODO: Stop pipe
        set_state(Media::stop);
    }
	return Media::ok;
}

Media::status Av_file_src::on_start(int start_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::play);
	cv.signal();
	return Media::ok;
}

Media::status Av_file_src::on_stop(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::stop);
	cv.signal();
	return Media::ok;
}

Media::status Av_file_src::on_pause(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::pause);
	cv.signal();
	return Media::ok;
}

Media::status Av_file_src::on_connect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
	return Media::ok;
}

Media::status Av_file_src::on_disconnect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
	is_running = 0;
	cv.signal();
	thread.join();
	MEDIA_LOG("Thread stopped: %s", object_name());
	return Media::ok;
}

int av_lock_callback(void **mutex_ptr, enum AVLockOp ops)
{
	switch (ops)
	{
		case AV_LOCK_CREATE:
			{
				*mutex_ptr = new Mutex; //	 Create a mutex.
				//printf("\nCreate Mutex");
			}
			break;
		case AV_LOCK_OBTAIN:
			{
				Mutex* mutex = (Mutex *) (*mutex_ptr);
				mutex->lock(); // 	 Lock the mutex.
				//printf("\nMutex Lock");
			}
			break;
		case AV_LOCK_RELEASE:
			{
				Mutex* mutex = (Mutex *) (*mutex_ptr);
				mutex->unlock(); // 	 Unlock the mutex.
				//printf("\nMutex Unlock");
			}
			break;
		case AV_LOCK_DESTROY:
			{
				Mutex* mutex = (Mutex *) *mutex_ptr;
				delete mutex; //	 Free mutex resources.
				*mutex_ptr = 0;
				//printf("\nDestroy Mutex");
			}
	}
	//fflush(stdin);
	return 0;
}

