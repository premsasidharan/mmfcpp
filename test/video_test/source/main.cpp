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

#include <video_widget.h>
#include <yuv_file_src.h>
#include <video_renderer.h>

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
    MEDIA_TRACE();
    QApplication app(argc, argv);

    if (argc < 3)
    {
        printf("\n\tInsufficient arguments");
        printf("\n\tUsage yuv_test yuv_file_with_path width height");
        printf("\n\tEg:- ./yuv_test test.yuv 320 240\n");
		return 0;
    }

	int time = 0;
    int width = atoi(argv[2]);
    int height = atoi(argv[3]);
    Video_widget window;
    Yuv_file_src src("yuv");
    Video_renderer sink("opengl", &window);

    if (1 != src.set_parameters(argv[1], width, height))
    {
        printf("\n\tInvalid Yuv File Path\n");
		return 0;
    }
    
	window.show();
    connect(&src, &sink);
    start(&src, 0);
    int ret = app.exec();
    stop(&src, time);
    disconnect(&src, &sink);

    return ret;
}

