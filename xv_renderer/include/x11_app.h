/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _X11_APP_H
#define _X11_APP_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#include <map>
#include <mutex.h>
#include <x11_window.h>

class X11_window;
class Xv_video_frame;

class X11_app
{
public:
	friend class X11_window;
	friend class Xv_video_frame;

	~X11_app();
	XvPortID get_xv_port();
	void free_xv_port(XvPortID port);

	static int exec();
	static X11_app& instance() { return x11_app; };
	static int screen() { return _screen; };
	static Display* display() { return _display; };
	static Window root_window() { return XRootWindow(_display, _screen); };

private:
	X11_app();
	int add_window(Window window, X11_window* x11_window);
	int del_window(Window window);

private:
	static int _screen;
	static Display* _display;
	static X11_app x11_app;

	static Mutex mutex;
	static std::map<Window, X11_window*> win_map;	
};

#endif

