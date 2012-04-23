/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <stdlib.h>
#include <string.h>

#include <yuv_file.h>

Yuv_file::Yuv_file(const char* path, int w, int h, Media::type _fmt)
    :width(w)
    , height(h)
    , file(0)
    , frame_count(0)
    , fmt(_fmt)
    , file_path(0)
{
    int length = strlen(path);
    file_path = new char[1+length];
    strncpy(file_path, path, length);
}

Yuv_file::~Yuv_file()
{
    close();
    
    delete [] file_path;
    file_path = 0;
}

int Yuv_file::frame_size() const
{
    int size = 0;
    
    switch (fmt)
    {
        case Media::I420:
        case Media::YV12:
            size = (width*height*3)>>1;
            break;
        case Media::YUY2:
        case Media::UYVY:
            size = (width*height)<<1;
            break;
        default:
            size = 0;
    }
    return size;
}

void Yuv_file::close()
{
    if (0 != file)
    {
        fclose(file);
        file = 0;
    }
}
