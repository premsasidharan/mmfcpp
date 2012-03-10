/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <string.h>
#include <read_wave_file.h>

Read_wave_file::Read_wave_file()
    :Wave_file()
{
}

Read_wave_file::~Read_wave_file()
{
    close();
}

int Read_wave_file::open(const char* path)
{
    close();
    file = fopen(path, "r");
    if (0 == file)
    {
        printf("Unable to Open file: %s\n", path);
        return 0;
    }
    fread(&hdr, 1, sizeof(Wave_hdr), file);
    int status = (memcmp(hdr.riff_id, "RIFF", 4) == 0);
    status = (status == 1)?(memcmp(hdr.wave.format, "WAVE", 4) == 0):status;
    status = (status == 1)?(memcmp(hdr.wave.fmt_id, "fmt ", 4) == 0):status;
    status = (status == 1)?(memcmp(hdr.wave.data_id, "data", 4) == 0):status;
    if (status == 0)
    {
        close();
        printf("Invalid file format: %s\n", path);
        return 0;
    }
    return 1;
}

void Read_wave_file::close()
{
    close_file();
}

int Read_wave_file::read(unsigned char* buffer, int buff_size, int& frames_read)
{
    if (0 == file ||
        0 == buff_size ||
        0 == frame_size())
    {
        frames_read = 0;
        return 0;
    }
    if (feof(file) || ferror(file))
    {
        frames_read = 0;
        return 0;
    }
    int size = frame_size();
    int bytes = (buff_size/size)*size;
    frames_read = fread(buffer, 1, bytes, file)/size;        
    return 1;
}

int Read_wave_file::seek(long offset, int whence)
{
    if (0 == file)
    {
        return 0;
    }
    int ret = 0;
    long byte_offset = offset*frame_size();
    switch (whence)
    {
        case SEEK_SET:
            ret = fseek(file, sizeof(Wave_hdr)+byte_offset, SEEK_SET);
            break;
        case SEEK_CUR:
            ret = fseek(file, byte_offset, SEEK_CUR);
            break;
        case SEEK_END:
            ret = fseek(file, byte_offset, SEEK_CUR);
            break;
        default:
            ret = -1;
    }
    return ret;
}
