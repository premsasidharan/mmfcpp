#ifndef _WAVE_FILE_SRC_H
#define _WAVE_FILE_SRC_H

#include <stdio.h>

#include <media.h>
#include <thread.h>
#include <wave_file.h>
#include <pcm_parameters.h>
#include <abstract_media_object.h>

class Wave_file_src:public Abstract_media_object
{
public:
	friend class Thread<Wave_file_src>;

	Wave_file_src(const char* _name);
	~Wave_file_src();

public:
	int set_file_path(const char* path);

protected:
	Media::status on_start(int start_time);
	Media::status on_stop(int end_time);
	Media::status on_pause(int end_time);

	Media::status on_connect(int port, Abstract_media_object* pobj);
	Media::status on_disconnect(int port, Abstract_media_object* pobj);

protected:
	int run();
	void process_wave_file();

private:
	int is_running;
	Wave_file file;
	int packet_size;
	int packet_count;
	Condition_variable cv;
	Thread<Wave_file_src> thread;

	const static Port output_port[];
};

#endif

