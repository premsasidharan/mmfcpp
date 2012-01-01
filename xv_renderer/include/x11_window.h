/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _X11_WINDOW_H
#define _X11_WINDOW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <x11_app.h>

#include <mutex.h>
#include <xv_video_frame.h>

class X11_window
{
public:
	friend class X11_app;

	X11_window(int x, int y, int width, int height);
	~X11_window();

public:
	void show();
	void show_frame(unsigned char* _yuv, int width, int fmt, int height);

private:
	void on_paint();

private:
	XvPortID port;
	Window window;
	int video_width;
	int video_height;
	unsigned int format;
	unsigned char* yuv_data;
};

#endif

