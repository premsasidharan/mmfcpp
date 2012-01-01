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

#include <x11_app.h>
#include <x11_window.h>
#include <xv_renderer.h>
#include <yuv_file_src.h>

int main(int argc, char** argv)
{
	MEDIA_TRACE();
	int time = 0;
	if (argc >= 3)
	{
		int width = atoi(argv[2]);
		int height = atoi(argv[3]);
		X11_window window(0, 0, width, height);	
		Yuv_file_src src("yuv");
		Xv_renderer sink("xv", &window);
		if (1 == src.set_parameters(argv[1], width, height))
		{
			window.show();
			connect(&src, &sink);
			start(&src, 0);
			X11_app::exec();
			stop(&src, time);
			disconnect(&src, &sink);
		}
		else
		{
			printf("\n\tInvalid Yuv File Path\n");
		}
	}
	else
	{
		printf("\n\tInsufficient arguments");
		printf("\n\tUsage yuv_test yuv_file_with_path width height");
		printf("\n\tEg:- ./yuv_test test.yuv 320 240\n");
	}
	return 0;
}

