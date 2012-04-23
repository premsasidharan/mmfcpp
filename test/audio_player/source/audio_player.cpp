/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <audio_player.h>
#include <player_window.h>

Audio_player::Audio_player()
    :QObject(0)
    , src("wave")
    , sink("alsa", "default", 1)
    , deinter("deinterleaver")
    , track_viewer("track_viewer")
    , window(*this)
{
    ::connect(src, sink);
    ::connect(sink, deinter);
    ::connect(deinter, track_viewer);
    sink.attach(Media::last_pkt_rendered, this);
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer_elapsed()));
    timer.setInterval(250);
}

Audio_player::~Audio_player()
{
    sink.detach(Media::last_pkt_rendered, this);
    ::disconnect(src, sink);
    ::disconnect(sink, deinter);
    ::disconnect(deinter, track_viewer);
}

void Audio_player::show()
{
    window.show();
}

int Audio_player::stop(int& time)
{
    int ret = 0;
    timer.stop();
    ret = ::stop(src, time);
    on_timer_elapsed();
    return ret;
}

int Audio_player::start(int time)
{
    qDebug() << "Audio_player::start : " << time;
    timer.start();
    return ::start(src, time, src.duration());
}

int Audio_player::set_file_path(const char* path)
{
    return src.set_file_path(path);
}

int Audio_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    timer.stop();
    on_timer_elapsed();
    qDebug() << "event_handler: " << event;
    return 0;
}

int Audio_player::channels() const
{
    return src.channels();
}

int Audio_player::duration() const
{
    return src.duration();
}

int Audio_player::current_position() const
{
    return sink.current_position();
}

void Audio_player::on_timer_elapsed()
{
    window.set_current_position(current_position());
    Buffer* buffer = track_viewer.get_buffer();
    if (0 != buffer)
    {
        Pcm_param* param = (Pcm_param *) buffer->parameter();
        int frames = buffer->get_data_size()/((param->bits_per_sample/8)*(param->channel_count));
        for (int i = 0; i < (int)param->channel_count; i++)
        {
            window.set_track_data(i, (int32_t*)buffer->data(), frames, (1<<(param->bits_per_sample-1)));
        }
        Buffer::release(buffer);
    }
}