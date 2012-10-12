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

#include <yuv_player.h>

#include <QtGui/QApplication>

void print_usage();
int find_args_index(char** argv, int size, const char* str);
int parse_args(char** argv, int size, const char* str, int& result);
int parse_args(char** argv, int size, const char* str, char*& result);
int parse_args(char** argv, int size, const char* str, float& result);
int parse_args(char** argv, int size, const char* str, Media::type& result);

int main(int argc, char** argv)
{
    MEDIA_TRACE();
    QApplication app(argc, argv);

    float fps = 0.0;
    Yuv_player player;
    QString file_path = "";
    Media::type format = Media::I420;
    int ret = 0, width = 0, height = 0;

    if (argc < 11)
    {
        printf("\nInsufficient arguments");
        print_usage();
        ret = 1;
    }
    else
    {   
        char* path = 0;
        ret = parse_args(argv, argc-1, "-p", path);
        ret = ret && parse_args(argv, argc-1, "-w", width);
        ret = ret && parse_args(argv, argc-1, "-h", height);
        ret = ret && parse_args(argv, argc-1, "-fmt", format);
        ret = ret && parse_args(argv, argc-1, "-fps", fps);
        
        if (0 == ret)
        {
            printf("\nFailed to parse parameters !!!");
            print_usage();
        }
        else
        {
            file_path = path;
        }
        ret = player.set_parameters(width, height, format, fps, file_path.toAscii().data());
        if (ret == 1)
        {
            player.start(0, 0);
        }
    }

    player.show();
    ret = app.exec();

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

int parse_args(char** argv, int size, const char* str, float& result)
{
    int ret = find_args_index(argv, size, str);
    if (ret >= 0)
    {
        result = atof(argv[ret+1]);
    }
    return (ret >= 0);
}

int parse_args(char** argv, int size, const char* str, Media::type& result)
{
    char fmt_str[10];
    int ret = find_args_index(argv, size, str);
    if (ret < 0)
    {
        return 0;
    }

    strncpy(fmt_str, argv[ret+1], 10);
    for (int i = 0; i < (int)strlen(fmt_str); i++)
    {
        fmt_str[i] = toupper(fmt_str[i]);
    }

    if (0 == strcmp(fmt_str, "I420") || 0 == strcmp(fmt_str, "YUV420"))
    {
        result = Media::I420;
        return 1;
    }
    if (0 == strcmp(fmt_str, "I422") || 0 == strcmp(fmt_str, "YUV422"))
    {
        result = Media::I422;
        return 1;
    }
    if (0 == strcmp(fmt_str, "I444") || 0 == strcmp(fmt_str, "YUV444"))
    {
        result = Media::I444;
        return 1;
    }
    if (0 == strcmp(fmt_str, "YV12"))
    {
        result = Media::YV12;
        return 1;
    }
    if (0 == strcmp(fmt_str, "UYVY"))
    {
        result = Media::UYVY;
        return 1;
    }
    if (0 == strcmp(fmt_str, "YUY2"))
    {
        result = Media::YUY2;
        return 1;
    }

    return 0;
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

void print_usage()
{
    printf("\nUsage yuv_player -p path -w width -h height -fmt format -fps frame_rate");
    printf("\nFormat - YUV420(I420)|YUV422(I422)|YUV444(I444)|YV12|YUY2|UYVY");
    printf("\nEg:- ./yuv_player -p test.yuv -w 320 -h 240 -fmt I420 -fps 24\n");   
}

