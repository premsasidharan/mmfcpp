/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _YUV_FILE_SRC_H
#define _YUV_FILE_SRC_H

#include <stdio.h>

#include <media.h>
#include <thread.h>
#include <yuv_parameters.h>
#include <abstract_media_object.h>

class Yuv_file_src:public Abstract_media_object
{
public:
	friend class Thread<Yuv_file_src>;

	Yuv_file_src(const char* _name);
	~Yuv_file_src();

public:
	int set_parameters(const char* path, int _width, int _height);

protected:
	Media::status on_start(int start_time);
	Media::status on_stop(int end_time);
	Media::status on_pause(int end_time);

	Media::status on_connect(int port, Abstract_media_object* pobj);
	Media::status on_disconnect(int port, Abstract_media_object* pobj);

protected:
	int run();
	int process_yuv_file();

private:
	int width;
	int height;
	FILE* file;
	int is_running;
	char* file_path;
	Condition_variable cv;
	unsigned int data_size;
	Thread<Yuv_file_src> thread;
	unsigned long long int frame_count;
	unsigned long long int total_frames;

	const static Port output_port[];
};

#endif

