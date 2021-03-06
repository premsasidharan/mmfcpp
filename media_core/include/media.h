/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _MEDIA_TYPES_H
#define _MEDIA_TYPES_H

#define AUDIO_MASK 0x800
#define VIDEO_MASK 0x1000

namespace Media
{
    enum state
    {
        init,
        stop,
        play
    };

    enum status
    {
        ok = 0,
        invalid_port = -1,
        invalid_type = -2,
        invalid_object = -3,
        type_mismatch = -4,
        port_not_connected = -5,
        invalid_buffer = -6,
        not_implemented = -7,
        non_play_state = -8 ,
        no_free_port = -9,
        not_connected = -10,
        invalid_event = -11,
        max_ports_reached = -12,
        null_object = -13,
    };

    enum type
    {
		YV12 = VIDEO_MASK|0x001,
		I420 = VIDEO_MASK|0x002,
		I422 = VIDEO_MASK|0x004,
		I444 = VIDEO_MASK|0x008,
		YUY2 = VIDEO_MASK|0x010,
		UYVY = VIDEO_MASK|0x020,
        VIDEO_FFMPEG_PKT = VIDEO_MASK|0x040,

        AUDIO_PCM = AUDIO_MASK|0x001,
        AUDIO_DEINT_PCM = AUDIO_MASK|0x002,
        AUDIO_FFMPEG_PKT = AUDIO_MASK|0x004
    };

    enum events
    {
        input_port_created = 0,
        output_port_created = 1,
        first_pkt_rendered = 2,
        last_pkt_rendered = 3,
        last_event,
    };
};

enum Buffer_flags
{
    FIRST_PKT = 0x0001,
    LAST_PKT = 0x0002,
};

#endif
