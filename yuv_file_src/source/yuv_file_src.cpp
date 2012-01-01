/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <buffer.h>
#include <media_debug.h>
#include <yuv_file_src.h>

const Port Yuv_file_src::output_port[] = {{Media::YUV420_PLANAR/*, 0*/, "yuv420"}};

Yuv_file_src::Yuv_file_src(const char* _name)
	:Abstract_media_object(_name)
	, width(0)
	, height(0)
	, file(0)
	, is_running(0)
	, file_path(0)
	, data_size(0)
	, frame_count(0)
	, total_frames(0)
{
	MEDIA_TRACE_OBJ_PARAM("%s", _name);
	create_output_ports(output_port, 1);
}

Yuv_file_src::~Yuv_file_src()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	delete [] file_path;
	file_path = 0;
}

int Yuv_file_src::set_parameters(const char* path, int _width, int _height)
{
	MEDIA_TRACE_OBJ_PARAM("%s width: %d, height: %d", object_name(), _width, _height);
	delete [] file_path;
	file_path = new char[strlen(path)+1];
	strcpy(file_path, path);

	width = _width;
	height = _height;

	file = fopen(file_path, "r");
	if (file == 0)
	{
		return -1;
	}
	struct stat status;
	stat(path, &status);
	
	data_size = (width*height*3)>>1;
	total_frames = (status.st_size/data_size);
	MEDIA_LOG("No Frames: %llu, Frame Size: %u", total_frames, data_size);

	return 1;
}

int Yuv_file_src::run()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	Media::state state = Media::stop;
	while (is_running)
	{
		state = get_state();
		switch (state)
		{
			case Media::init:				
				MEDIA_LOG("%s, State: %s", object_name(), "INIT");
				cv.wait();
				break;

			case Media::stop:				
				MEDIA_WARNING("%s, State: %s", object_name(), "STOP");
				cv.wait();
				break;

			case Media::pause:
				MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
				cv.wait();
				break;

			case Media::play:
				MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
				process_yuv_file();
				break;

			default:
				MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
				break;
		}
	}
	MEDIA_WARNING("Exiting Thread: %s", object_name());
	return 0;
}

int Yuv_file_src::process_yuv_file()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	Buffer* buffer = Buffer::request(data_size, Media::YUV420_PLANAR, sizeof(I420_param));
	fread(buffer->data(), data_size, 1, file);
	buffer->set_pts(frame_count++);
	I420_param* param = (I420_param *) buffer->parameter();
	param->width = width;
	param->height = height;
	if (frame_count == 1)
	{
		buffer->set_flags(FIRST_PKT);
	}
	else if (frame_count == total_frames)
	{
		buffer->set_flags(LAST_PKT);
		fclose(file);
	}
	else
	{
		buffer->set_flags(0);
	}
	push_data(0, buffer);

	if (total_frames == frame_count)
	{
		set_state(Media::stop);
	}

	return Media::ok;
}

Media::status Yuv_file_src::on_start(int start_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::play);
	cv.signal();
	return Media::ok;
}

Media::status Yuv_file_src::on_stop(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::stop);
	cv.signal();
	return Media::ok;
}

Media::status Yuv_file_src::on_pause(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::pause);
	cv.signal();
	return Media::ok;
}

Media::status Yuv_file_src::on_connect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
	is_running = 1;
	thread.start(this);
	return Media::ok;
}

Media::status Yuv_file_src::on_disconnect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Port: %d", object_name(), port);
	is_running = 0;
	cv.signal();
	thread.join();
	MEDIA_LOG("Thread stopped: %s", object_name());
	return Media::ok;
}

