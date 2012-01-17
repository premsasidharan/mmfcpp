/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _AV_PARAMETERS_H
#define _AV_PARAMETERS_H

#include <mutex.h>
#include <libavformat/avformat.h>

struct Av_param
{
    AVCodecContext* codec_ctx;
};

#endif
