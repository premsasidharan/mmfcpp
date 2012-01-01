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
#include <wave_file_src.h>

const Port Wave_file_src::output_port[] = {{Media::AUDIO_PCM/*, 0*/, "pcm"}};

Wave_file_src::Wave_file_src(const char* _name)
	:Abstract_media_object(_name)
	, is_running(0)
	, file(0)
	, packet_size(0)
	, packet_count(0)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	create_output_ports(output_port, 1);
}

Wave_file_src::~Wave_file_src()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
}

int Wave_file_src::set_file_path(const char* path)
{
	MEDIA_TRACE_OBJ_PARAM("%s, Path: %s", object_name(), path);
	if (1 == file.open(path))
	{
		packet_size = 1024*file.channels()*(file.bits_per_sample()/8);
		return 1;
	}
	return 0;
}

int Wave_file_src::run()
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
				MEDIA_LOG("%s, State: %s", object_name(), "STOP");
				cv.wait();
				break;

			case Media::pause:
				MEDIA_LOG("%s, State: %s", object_name(), "PAUSE");
				cv.wait();
				break;

			case Media::play:
				MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
				process_wave_file();
				break;

			default:
				MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
				break;
		}
	}
	MEDIA_WARNING("Exiting Thread: %s", object_name());
	return 0;
}

void Wave_file_src::process_wave_file()
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	int data_size = 0;
	Buffer* buffer = Buffer::request(packet_size, Media::AUDIO_PCM, sizeof(Pcm_param));
	data_size = file.read((char*)buffer->data(), packet_size);
	buffer->set_pts(packet_count++);
	buffer->set_data_size(data_size);	

	Pcm_param* param = (Pcm_param *) buffer->parameter();
	param->channel_count = file.channels();
	param->samples_per_sec = file.samples_per_second();
	param->avg_bytes_per_sec = file.average_bytes_per_second();
	param->bits_per_sample = file.bits_per_sample();

	if (packet_count == 1)
	{
		buffer->set_flags(FIRST_PKT);
	}
	if (data_size != packet_size)
	{
		buffer->set_flags(LAST_PKT);
		file.close();
	}
	else
	{
		buffer->set_flags(0);
	}
	MEDIA_WARNING("%s, channels: %d, samples_per_sec: %d, bits_per_sample: %d, Buff_size: %d", object_name(), file.channels(), file.samples_per_second(), file.bits_per_sample(), data_size);
	push_data(0, buffer);

	if (data_size != packet_size)
	{
		set_state(Media::stop);
	}
}

Media::status Wave_file_src::on_start(int start_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::play);
	cv.signal();
	return Media::ok;
}

Media::status Wave_file_src::on_stop(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::stop);
	cv.signal();
	return Media::ok;
}

Media::status Wave_file_src::on_pause(int end_time)
{
	MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	set_state(Media::pause);
	cv.signal();
	return Media::ok;
}

Media::status Wave_file_src::on_connect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
	is_running = 1;
	thread.start(this);
	return Media::ok;
}

Media::status Wave_file_src::on_disconnect(int port, Abstract_media_object* pobj)
{
	MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
	is_running = 0;
	cv.signal();
	thread.join();
	MEDIA_LOG("Thread stopped: %s", object_name());
	return Media::ok;
}

