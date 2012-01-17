/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <x11_window.h>
#include <xv_video_frame.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

X11_window::X11_window(int x, int y, int width, int height)
    :video_width(0)
    , video_height(0)
    , format(0)
    , yuv_data(0)
{
    XSetWindowAttributes attributes;

    attributes.override_redirect = 0;
    attributes.border_pixel = XBlackPixel(X11_app::display(), X11_app::screen());
    attributes.background_pixel = XWhitePixel(X11_app::display(), X11_app::screen());

    window = XCreateWindow(X11_app::display(), XRootWindow(X11_app::display(), X11_app::screen()), 0, 0, width, height, 1,
                           DefaultDepth(X11_app::display(), X11_app::screen()), InputOutput, DefaultVisual(X11_app::display(), X11_app::screen()),
                           CWBackPixel|CWBorderPixel|CWOverrideRedirect, &attributes);
    X11_app::instance().add_window(window, this);

    XSelectInput(X11_app::display(), window, ExposureMask|KeyPressMask|ButtonPressMask|StructureNotifyMask);
    port = X11_app::instance().get_xv_port();
}

X11_window::~X11_window()
{
    X11_app::instance().free_xv_port(port);
    X11_app::instance().del_window(window);
    XUnmapWindow(X11_app::display(), window);
    XDestroyWindow(X11_app::display(), window);
}

void X11_window::show()
{
    XMapWindow(X11_app::display(), window);
    XFlush(X11_app::display());
}

void X11_window::show_frame(unsigned char* _yuv, int fmt, int width, int height)
{
    XExposeEvent event;
    memset(&event, 0, sizeof(event));

    XLockDisplay(X11_app::display());
    event.type = Expose;
    event.display = X11_app::display();
    event.window = window;
    yuv_data = _yuv;
    format = fmt;
    video_width = width;
    video_height = height;
    XSendEvent(X11_app::display(), window, True, ExposureMask, (XEvent *)&event);
    XFlush(X11_app::display());
    XUnlockDisplay(X11_app::display());
}

void X11_window::on_paint()
{
    GC gc = XCreateGC(X11_app::display(), window, 0, 0);
    if (0 != yuv_data)
    {
        XWindowAttributes attr;
        XGetWindowAttributes(X11_app::display(), X11_app::root_window(), &attr);
        Xv_video_frame* video_frame = new Xv_video_frame(video_width, video_height, format, port, yuv_data);
        XvPutImage(X11_app::display(), video_frame->port, window, gc, video_frame->image, 0, 0,
                   video_frame->image_width(), video_frame->image_height(), 0, 0, attr.width, attr.height);
        delete video_frame;
    }
    XFreeGC(X11_app::display(), gc);
}
