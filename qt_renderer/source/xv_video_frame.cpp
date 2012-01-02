/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <QX11Info>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <xv_video_frame.h>

Xv_video_frame::Xv_video_frame(int _width, int _height, int fmt, XvPortID _port, unsigned char* _yuv_data)
	:width(_width)
	, height(_height)
	, format(fmt)
	, port(_port)
	, image(0)
    , yuv_data(_yuv_data)
{
	image = XvCreateImage(QX11Info::display(), port, fmt, (char *)yuv_data, width, height);
}

Xv_video_frame::~Xv_video_frame()
{
	XFree(image);
	image = 0;
}

unsigned int Xv_video_frame::video_fmt(const char* fmt)
{
	unsigned int id = 0;
	for (int i = strlen(fmt)-1; i >= 0 ; i--)
	{
		id = (id << 8)|fmt[i];
	}
	return id;
}

