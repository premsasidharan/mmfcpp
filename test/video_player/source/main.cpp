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

#include <video_player.h>

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
    Video_player player;

    if (1 != player.set_parameters(width, height, (0 == atoi(argv[4]))?Media::YUY2:Media::I420, 24.0, argv[1]))
    {
        printf("\n\tInvalid Yuv File Path\n");
		return 0;
    }
    
	player.show();
    player.start(0, player.duration());
    int ret = app.exec();
    player.stop(time);

    return ret;
}
