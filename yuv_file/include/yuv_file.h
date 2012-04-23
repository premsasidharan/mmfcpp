/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _YUV_FILE_H_
#define _YUV_FILE_H_

#include <stdio.h>

#include <media.h>
#include <media_debug.h>

class Yuv_file
{
public:
    Yuv_file(const char* path, int w, int h, Media::type _fmt);
    ~Yuv_file();

public:
    void close();
    int frame_size() const;
    
    Media::type format() const { return fmt; };
    int video_width() const { return width; };
    int video_height() const { return height; };
    
    int is_open() const { return (0 != file); };
    
protected:
    int width;
    int height;
    FILE* file;
    int frame_count;
    Media::type fmt;
    char* file_path;
};

#endif
