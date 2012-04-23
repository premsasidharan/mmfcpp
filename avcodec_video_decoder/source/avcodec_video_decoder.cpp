/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <buffer.h>
#include <media_debug.h>
#include <yuv_parameters.h>
#include <avcodec_video_decoder.h>

const Port Avcodec_video_decoder::input_port[] = {{Media::VIDEO_FFMPEG_PKT, "video"}};
const Port Avcodec_video_decoder::output_port[] = {{Media::I420, "video"}};

Avcodec_video_decoder::Avcodec_video_decoder(const char* _name)
    :Abstract_media_object(_name)
    , is_running(0)
    , queue(3)
    , frame(0)
    , yuv_frame(0)
    , video_codec(0)
    , video_ctx(0)
    , img_convert_ctx(0)
    , frame_count(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    create_input_ports(input_port, 1);
    create_output_ports(output_port, 1);
}

Avcodec_video_decoder::~Avcodec_video_decoder()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Avcodec_video_decoder::run()
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

Media::status Avcodec_video_decoder::on_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::play);
    cv.signal();
    return Media::ok;
}

Media::status Avcodec_video_decoder::on_stop(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    cv.signal();
    MEDIA_LOG("on_stop: %s", object_name());
    return Media::ok;
}

Media::status Avcodec_video_decoder::on_pause(int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::pause);
    cv.signal();
    return Media::ok;
}

Media::status Avcodec_video_decoder::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    if (0 == is_running)
    {
        is_running = 1;
        thread.start(this);
    }
    return Media::ok;
}

Media::status Avcodec_video_decoder::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
    is_running = 0;
    cv.signal();
    thread.join();
    MEDIA_LOG("Thread stopped: %s", object_name());
    return Media::ok;
}

Media::status Avcodec_video_decoder::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    int status = 0;
    if (Media::play == get_state())
    {
        AVPacket* packet = (AVPacket*)buffer->data();
        MEDIA_LOG("%s, Buffer: 0x%llx, dts: %llu, pts: %llu, Status: %d", object_name(), (unsigned long long)buffer, packet->dts, buffer->pts(), status);
        status = queue.push(packet->dts, buffer, 2000);
    }
    else
    {
        AVPacket* packet = (AVPacket *)buffer->data();
        av_free_packet(packet);
        packet = 0;
        Buffer::release(buffer);
        sleep(1);
        MEDIA_ERROR("Packet received under non-play state: %s, Buffer: 0x%llx, pts: %llu", object_name(), (unsigned long long)buffer, buffer->pts());
        return Media::non_play_state;
    }
    return Media::ok;
}

void Avcodec_video_decoder::decode()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Buffer* buffer = queue.pop(2000);
    if (0 != buffer)
    {
        int flag = 0;
        AVPacket* packet = (AVPacket *)buffer->data();
        Av_param* param = (Av_param *)buffer->parameter();

        if (1 == check_and_update_codec(param->codec_ctx))
        {
            flag = 0;
            avcodec_decode_video2(video_ctx, frame, &flag, packet);
            if (flag || (buffer->flags() & LAST_PKT))
            {
                int frame_size = avpicture_get_size(PIX_FMT_YUV420P, video_ctx->width, video_ctx->height);
                Buffer* frame_buffer = Buffer::request(frame_size, Media::I420, sizeof(Yuv_param));

                Yuv_param* frame_param = (Yuv_param *) frame_buffer->parameter();
                frame_param->width = video_ctx->width;
                frame_param->height = video_ctx->height;

                frame_buffer->set_flags(0);
                frame_buffer->set_pts(packet->pts);

                avpicture_fill((AVPicture *)yuv_frame, (uint8_t*)frame_buffer->data(), PIX_FMT_YUV420P, video_ctx->width, video_ctx->height);
                sws_scale(img_convert_ctx, frame->data, frame->linesize, 0, video_ctx->height, yuv_frame->data, yuv_frame->linesize);

                if (frame_count == 0)
                {
                    frame_buffer->set_flags(FIRST_PKT);
                    MEDIA_LOG("Video Decoder: FIRST_PKT, pts: %ld", packet->pts);
                }

                if (buffer->flags() & LAST_PKT)
                {
                    frame_buffer->set_flags(frame_buffer->flags()|LAST_PKT);
                    MEDIA_LOG("Video Decoder: LAST_PKT, pts: %ld", packet->pts);
                }
                ++frame_count;

                MEDIA_LOG("pts: %ld, dts: %ld", packet->pts, packet->dts);
                push_data(0, frame_buffer);
            }
        }
        else
        {
            MEDIA_ERROR("Decode %s", "Failed");
        }

        if (buffer->flags() & LAST_PKT)
        {
            set_state(Media::stop);
        }

        av_free_packet(packet);
        Buffer::release(buffer);
    }
    else
    {
        MEDIA_ERROR("Video Decoder: %s, Null Buffer", object_name());
    }
}

void Avcodec_video_decoder::close_codec()
{
    if (0 != video_ctx)
    {
        avcodec_close(video_ctx);
        video_ctx = 0;
    }
    if (0 != frame)
    {
        av_free(frame);
        frame = 0;
    }
    if (0 != yuv_frame)
    {
        av_free(yuv_frame);
        yuv_frame = 0;
    }
}

int Avcodec_video_decoder::check_and_update_codec(AVCodecContext* codec_ctx)
{
    int status = 1;
    if (codec_ctx != video_ctx)
    {
        close_codec();
        video_ctx = codec_ctx;
        AVCodec* codec = avcodec_find_decoder(video_ctx->codec_id);

        if (0 != codec)
        {
            avcodec_open(codec_ctx, codec);
            frame = avcodec_alloc_frame();
            yuv_frame = avcodec_alloc_frame();
            img_convert_ctx = sws_getContext(video_ctx->width, video_ctx->height,
            video_ctx->pix_fmt, video_ctx->width, video_ctx->height,
            PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);

        }
        else
        {
            status = 0;
            MEDIA_ERROR("Error, codec %d, doesn't exist", video_ctx->codec_id);
        }
    }
    return status;
}
