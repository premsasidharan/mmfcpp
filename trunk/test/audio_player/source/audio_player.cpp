/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <audio_player.h>

Audio_player::Audio_player(QObject* parent)
    :QObject(parent)
    , src("wave")
    , sink("alsa", "default")
{
    ::connect(src, sink);
    sink.attach(Media::last_pkt_rendered, this);
}

Audio_player::~Audio_player()
{
    sink.detach(Media::last_pkt_rendered, this);
    ::disconnect(src, sink);
}

int Audio_player::stop(int& time)
{
    return ::stop(src, time);
}

int Audio_player::start(int time)
{
    return ::start(src, time);
}

int Audio_player::set_file_path(const char* path)
{
    return src.set_file_path(path);
}

int Audio_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    printf("\nevent_handler: %d\n", event);
    return 0;
}

int Audio_player::duration() const
{
    return src.get_total_frames();
}

int Audio_player::current_position() const
{
    return sink.current_frame();
}
