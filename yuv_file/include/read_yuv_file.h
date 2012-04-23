/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _READ_YUV_FILE_H_
#define _READ_YUV_FILE_H_

#include <yuv_file.h>

class Read_yuv_file:public Yuv_file
{
public:
    Read_yuv_file(const char* path, int w, int h, Media::type fmt);
    ~Read_yuv_file();
    
public:
    int open();

    int frame_count() const;

    int seek(int offset, int frames);
    int read(void* buffer, int size);

private:
    mutable int total_frames;
};

#endif
