/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _PCM_PARAMETERS_H
#define _PCM_PARAMETERS_H

struct Pcm_param
{
	unsigned int channel_count;
	unsigned int samples_per_sec;
	unsigned int avg_bytes_per_sec;
	unsigned int bits_per_sample;
};

#endif

