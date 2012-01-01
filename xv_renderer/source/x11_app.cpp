/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <x11_app.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int X11_app::_screen = 0;
Display* X11_app::_display = 0;
X11_app X11_app::x11_app;

Mutex X11_app::mutex;
std::map<Window, X11_window*> X11_app::win_map;	

X11_app::X11_app()
{
	XInitThreads();
	_display = XOpenDisplay(0);
	_screen = DefaultScreen(_display);
}

X11_app::~X11_app()
{
	XCloseDisplay(_display);
	_display = 0;
	_screen = 0;
}

XvPortID X11_app::get_xv_port()
{
	XvPortID port = 0;
    unsigned int count = 0;
    XvAdaptorInfo* info = 0;

	int status = XvQueryAdaptors(_display, XRootWindow(_display, _screen), &count, &info);
	for (int i = 0; i < (int)count; i++)
	{
		printf( "\n\tAdapter Name : %s, Port base id: 0x%x, Num of Ports: %ld\n", info[i].name, (unsigned int)info[i].base_id, info[i].num_ports);
		for (int j = 0; j < (int)info[j].num_ports; j++)
		{
			port = j+info[i].base_id;
			status = XvGrabPort(_display, port, CurrentTime);
			if (status == Success)
			{
	    		goto exit;
			}
		}
	}
exit:
	XvFreeAdaptorInfo(info);
	return port;
}

void X11_app::free_xv_port(XvPortID port)
{
	XvUngrabPort(_display, port, CurrentTime);
}

int X11_app::exec()
{
	XEvent event;
	do
	{ 
		XNextEvent(_display, &event); 
		XLockDisplay(_display);	
		if (event.type == Expose)
		{
			X11_window* x11_window = win_map[event.xany.window];
			if (0 != x11_window)
			{
				x11_window->on_paint();
			}
		}
		XUnlockDisplay(_display);
	} while (event.type != KeyPress); 

	return 1;
}

int X11_app::add_window(Window window, X11_window* x11_window)
{
	mutex.lock();
	win_map[window] = x11_window;
	mutex.unlock();
	return 1;
}

int X11_app::del_window(Window window)
{
	mutex.lock();
	win_map.erase(window);
	mutex.unlock();
	return 1;
}

