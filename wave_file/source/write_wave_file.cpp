/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <string.h>
#include <write_wave_file.h>

Write_wave_file::Write_wave_file()
    :Wave_file()
{
}

Write_wave_file::~Write_wave_file()
{
    close();
}

int Write_wave_file::open(char* path, int sample_rate, int channels, int bits_per_sample)
{
    close();
    file = fopen(path, "w");
    if (0 == file)
    {
        printf("Unable to Open file: %s\n", path);
        return 0;
    }
    memcpy(hdr.riff_id, "RIFF", 4);
    memcpy(hdr.wave.format, "WAVE", 4);
    memcpy(hdr.wave.fmt_id, "fmt ", 4);
    memcpy(hdr.wave.data_id, "data", 4);	
    
    hdr.wave.data_size = 0;
    hdr.wave.size = sizeof(Wave_params);
    hdr.wave.params.audio_format = 1;
    hdr.wave.params.num_channels = channels;
    hdr.wave.params.sample_rate = sample_rate;
    hdr.wave.params.byte_rate = sample_rate*channels*(bits_per_sample/8);
    hdr.wave.params.block_align = channels*(bits_per_sample/8);
    hdr.wave.params.bits_per_sample = bits_per_sample;
    hdr.size = hdr.wave.data_size+sizeof(Wave_data);
    
    int size = fwrite(&hdr, 1, sizeof(Wave_hdr), file);
    if (size != sizeof(Wave_hdr))
    {
        close();
        return 0;
    }
    return 1;
}

int Write_wave_file::write(unsigned char* buffer, int frames)
{
    if (0 == file ||
        0 == frames ||
        0 == frame_size())
    {
        return 0;
    }
    int bytes = 0;
    int size = frames*frame_size();
    unsigned char* ptr = 0;
    do
    {
        ptr = buffer+bytes;
        bytes += fwrite(ptr, 1, size, file);
        if (ferror(file) > 0)
        {
            return 0;
        }
        size -= bytes;
    } while (size > 0);
    hdr.wave.data_size += bytes;
    hdr.size = hdr.wave.data_size+sizeof(Wave_data);
    return 1;
}

void Write_wave_file::close()
{
    if (0 != file)
    {
        fseek(file, 0, SEEK_SET);
        fwrite(&hdr, sizeof(Wave_hdr), 1, file);
    }
    close_file();
}

