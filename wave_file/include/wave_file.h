/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _WAVE_FILE_H_
#define _WAVE_FILE_H_

#include <stdio.h>
#include <wave_hdr.h>

class Wave_file
{
public:
    Wave_file();
    virtual ~Wave_file();

public:
    int frames_count() const;
    int sample_rate() const { return hdr.wave.params.sample_rate; };
    int channel_count() const { return hdr.wave.params.num_channels; };
    int bits_per_sample() const { return hdr.wave.params.bits_per_sample; };
    int frame_size() const { return hdr.wave.params.num_channels*(hdr.wave.params.bits_per_sample/8); };

public:
    int is_eof();
    virtual void close() = 0;

protected:
    void close_file();

protected:
    FILE* file;
    Wave_hdr hdr;
};

#endif

