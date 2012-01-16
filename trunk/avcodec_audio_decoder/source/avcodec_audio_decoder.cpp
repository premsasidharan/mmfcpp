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
#include <pcm_parameters.h>
#include <avcodec_audio_decoder.h>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

const Port Avcodec_audio_decoder::input_port[] = {{Media::AUDIO_FFMPEG_PKT, "audio"}};
const Port Avcodec_audio_decoder::output_port[] = {{Media::AUDIO_PCM, "audio"}};

Avcodec_audio_decoder::Avcodec_audio_decoder(const char* _name)
	:Abstract_media_object(_name)
	, is_running(0)
	, queue(10)
    , audio_ctx(0)
{
	MEDIA_TRACE_OBJ_PARAM("%s", _name);
	create_input_ports(input_port, 1);
	create_output_ports(output_port, 1);
}

Avcodec_audio_decoder::~Avcodec_audio_decoder()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Avcodec_audio_decoder::run()
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
                decode();
				break;

			default:
				MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
				break;
		}
	}
	MEDIA_WARNING("Exiting Thread: %s", object_name());
	return 0;
}

Media::status Avcodec_audio_decoder::on_start(int start_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::play);
	cv.signal();
	return Media::ok;
}

Media::status Avcodec_audio_decoder::on_stop(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::stop);
	cv.signal();
	MEDIA_LOG("on_stop: %s", object_name());
	return Media::ok;
}

Media::status Avcodec_audio_decoder::on_pause(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::pause);
	cv.signal();
	return Media::ok;
}

Media::status Avcodec_audio_decoder::on_connect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
	return Media::ok;
}

Media::status Avcodec_audio_decoder::on_disconnect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    if (1 == is_running)
    {
        is_running = 0;
        cv.signal();
        thread.join();
    }
	MEDIA_LOG("Thread stopped: %s", object_name());
	return Media::ok;
}

Media::status Avcodec_audio_decoder::input_data(int port, Buffer* buffer)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name()); 
    int status = 0;
	if (Media::play == get_state())
	{
    	AVPacket* packet = (AVPacket*)buffer->data();
		status = queue.push(packet->dts, buffer, 2000);
		MEDIA_LOG("Audio Decoder: %s, Buffer: 0x%llx, dts: %llu, pts: %llu, Status: %d", object_name(), (unsigned long long)buffer, packet->dts, buffer->pts(), status);
	}
	else
	{
    	AVPacket* packet = (AVPacket *)buffer->data();
        av_free_packet(packet); packet = 0;
		Buffer::release(buffer);
		sleep(1);			
		MEDIA_ERROR("Audio Decoder: Packet received under non-play state: %s, Buffer: 0x%llx, pts: %llu", object_name(), (unsigned long long)buffer, buffer->pts());
		return Media::non_play_state;
	}          
	return Media::ok;
}

void Avcodec_audio_decoder::decode()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());    
    Buffer* buffer = queue.pop(2000);   
	if (0 != buffer)
	{
        AVPacket* packet = (AVPacket *)buffer->data();
        Av_param* param = (Av_param *)buffer->parameter();
        
        MEDIA_LOG("packet pts: %lld, dts: %lld", packet->pts, packet->dts);
        
        if (1 == check_and_update_codec(param->codec_ctx))
        {
            int frame_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
            Buffer* audio_buffer = Buffer::request(AVCODEC_MAX_AUDIO_FRAME_SIZE, Media::AUDIO_PCM, sizeof(Pcm_param));	
            
            Pcm_param* param = (Pcm_param *) audio_buffer->parameter();
            param->channel_count = audio_ctx->channels;
            param->samples_per_sec = audio_ctx->sample_rate;
            param->avg_bytes_per_sec = audio_ctx->sample_rate*audio_ctx->channels*(audio_ctx->bits_per_coded_sample/8);
            param->bits_per_sample = audio_ctx->bits_per_coded_sample;
        
            int status = avcodec_decode_audio3(audio_ctx, (int16_t *)audio_buffer->data(), &frame_size, packet);
            
            audio_buffer->set_flags(buffer->flags());
            audio_buffer->set_pts(packet->pts);
            audio_buffer->set_data_size(frame_size);	
            MEDIA_LOG("channels: %d, samples_per_sec: %d, avg_bytes_per_sec: %d, bits_per_sample: %d, frame_size: %d 0x%x status: %d",
                audio_ctx->channels, audio_ctx->sample_rate, param->avg_bytes_per_sec, audio_ctx->bits_per_coded_sample, 
                frame_size, audio_buffer->data(), status);
                
            if (status < 0)
            {
                MEDIA_ERROR("Audio Decode Failed, status: %d", status);
            }
            
            if (buffer->flags() & FIRST_PKT)
            {
                MEDIA_LOG("Audio Decoder: %s", "First Packet");
            }
            if (buffer->flags() & LAST_PKT)
            {
                MEDIA_LOG("Audio Decoder: %s", "Last Packet");
            }
            
            push_data(0, audio_buffer);
            MEDIA_LOG("Packet: 0x%x, size: %d, %d, %d", packet->data, packet->size, frame_size, AVCODEC_MAX_AUDIO_FRAME_SIZE);
            
            if (buffer->flags() & LAST_PKT)
            {
                set_state(Media::stop);
            }
        }
        else
        {
            MEDIA_ERROR("Audio Decode Failed %s", object_name());
        }
        
        av_free_packet(packet); 
        packet = 0;
        Buffer::release(buffer);
    }
    else
    {
        MEDIA_ERROR("Audio Decoder: %s, Null Buffer", object_name());
    }
}

void Avcodec_audio_decoder::close_codec()
{
    if (0 != audio_ctx)
    {
        avcodec_close(audio_ctx);
        audio_ctx = 0;
    }
}

int Avcodec_audio_decoder::check_and_update_codec(AVCodecContext* codec_ctx)
{
    int status = 1;
    if (codec_ctx != audio_ctx)
    {
        close_codec();
        audio_ctx = codec_ctx;
        AVCodec* codec = avcodec_find_decoder(audio_ctx->codec_id);
        
        if (0 != codec)
        {
            avcodec_open(audio_ctx, codec);              
        }
        else
        {
            status = 0;
            MEDIA_ERROR("Error, codec %d, doesn't exist", audio_ctx->codec_id);
        }
    }
    return status;
}
