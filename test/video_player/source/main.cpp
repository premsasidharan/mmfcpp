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

void print_usage();
Media::type media_format(char* str);
int find_args_index(char** argv, int size, const char* str);
int parse_args(char** argv, int size, const char* str, int& result);
int parse_args(char** argv, int size, const char* str, char*& result);

int main(int argc, char** argv)
{
    MEDIA_TRACE();
    QApplication app(argc, argv);

    if (argc < 11)
    {
        printf("\nInsufficient arguments");
        print_usage();
		return 0;
    }

	int time = 0;
    char* path = 0;
    char* format = 0;
    int ret, width = 0, height = 0, fps = 0;
    
    ret = parse_args(argv, argc-1, "-p", path);
    ret = ret && parse_args(argv, argc-1, "-w", width);
    ret = ret && parse_args(argv, argc-1, "-h", height);
    ret = ret && parse_args(argv, argc-1, "-fmt", format);
    ret = ret && parse_args(argv, argc-1, "-fps", fps);
    
    if (0 == ret)
    {
        printf("\nFailed to parse parameters !!!");
        print_usage();
        return 0;
    }
    
    Video_player player;

    if (1 != player.set_parameters(width, height, media_format(format), (float)fps, path))
    {
        printf("\n\tInvalid Yuv File Path\n");
		return 0;
    }
    
	player.show();
    player.start(0, player.duration());
    ret = app.exec();
    player.stop(time);

    return ret;
}

int find_args_index(char** argv, int size, const char* str)
{
    for (int i = 0; i < size; i++)
    {
        if (0 == strcmp(argv[i], str))
        {
            return i;
        }
    }
    return -1;
}

int parse_args(char** argv, int size, const char* str, int& result)
{
    int ret = find_args_index(argv, size, str);
    if (ret >= 0)
    {
        result = atoi(argv[ret+1]);
    }
    return (ret >= 0);
}

int parse_args(char** argv, int size, const char* str, char*& result)
{
    int ret = find_args_index(argv, size, str);
    if (ret >= 0)
    {
        result = argv[ret+1];
    }
    return (ret >= 0);
}

Media::type media_format(char* str)
{
    Media::type type = Media::I420;
    if (0 == strcmp(str, "I420"))
    {
        type = Media::I420;
    }
    else if (0 == strcmp(str, "YUY2"))
    {
        type = Media::YUY2;
    }
    else if (0 == strcmp(str, "YV12"))
    {
        type = Media::YV12;
    }
    else if (0 == strcmp(str, "UYVY"))
    {
        type = Media::UYVY;
    }
    return type;
}

void print_usage()
{
    printf("\nUsage video_player -p path -w width -h height -fmt format -fps frame_rate");
    printf("\nEg:- ./video_player -p test.yuv -w 320 -h 240 -fmt I420 -fps 24\n");   
}
