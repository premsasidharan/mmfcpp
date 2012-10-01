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
    Yuv_file_src src1("yuv 1");
    Yuv_file_src src2("yuv 2");
    Master_clock master("master");
    Video_renderer sink("opengl", master.create_child("child"));
	sink.set_render_widget(&window);

    if (1 != src1.set_parameters(argv[1], Media::I420, 24.0, width, height))
    {
        printf("\n\tInvalid Yuv File Path\n");
		return 0;
    }
    
    if (1 != src2.set_parameters(argv[4], Media::I420, 24.0, atoi(argv[5]), atoi(argv[6])))
    {
        printf("\n\tInvalid Yuv File Path\n");
		return 0;
    }

	window.show();
    connect(&src1, "yuv", &sink, "left");
    connect(&src2, "yuv", &sink, "right");
    start(&src1, 0, src1.duration());
    start(&src2, 0, src2.duration());
    int ret = app.exec();
    stop(&src1, time);
    stop(&src2, time);
    disconnect(&src1, &sink);
    disconnect(&src2, &sink);

    return ret;
}

