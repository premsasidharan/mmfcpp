/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

#include <wave_file_src.h>
#include <audio_renderer.h>

int main(int argc, char** argv)
{
	int time = 0;
    MEDIA_TRACE();
	Wave_file_src src("wave");
	Audio_renderer sink("alsa", "default");

	if (argc < 2)
	{
		printf("\n\tInsufficien arguments\n");
		exit(0);
	}

	if (1 == src.set_file_path(argv[1]))
	{
		connect(&src, &sink);
		start(&src, 0);
		sleep(10);
		stop(&src, time);
		disconnect(&src, &sink);
	}
	else
	{
		printf("\nInvalid Wave File Path");
	}
	return 0;
}

