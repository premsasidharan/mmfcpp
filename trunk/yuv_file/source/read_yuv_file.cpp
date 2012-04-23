/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <read_yuv_file.h>

Read_yuv_file::Read_yuv_file(const char* path, int w, int h, Media::type fmt)
    :Yuv_file(path, w, h, fmt)
    , total_frames(0)
{
}

Read_yuv_file::~Read_yuv_file()
{
}

int Read_yuv_file::open()
{
    close();
    file = fopen(file_path, "r");
    if (0 == file)
    {
        return 0;
    }
    return 1;
}

int Read_yuv_file::frame_count() const
{
    struct stat status;
    stat(file_path, &status);
    total_frames = (status.st_size/frame_size());
    return total_frames;
}

int Read_yuv_file::read(void* buffer, int size)
{
    int size_frame = frame_size();
    if (0 == file || size < size_frame)
    {
        return 0;
    }
    int ret_size = fread(buffer, 1, size_frame, file);
    return (ret_size == size_frame);
}

int Read_yuv_file::seek(int frames, int whence)
{
    int size_frame = frame_size();
    if (0 == file)
    {
        return 0;
    }
    long offset = size_frame*frames;
    int status = fseek(file, offset, whence);
    return (0 == status);
}
