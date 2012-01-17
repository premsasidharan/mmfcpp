/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <yuv_window.h>
#include <xv_video_frame.h>

Yuv_window::Yuv_window(int x, int y, int width, int height)
    :QWidget(0)
    , video_width(0)
    , video_height(0)
    , format(0)
    , yuv_data(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_PaintOnScreen);
    resize(width, height);
    port = get_xv_port();
    connect(this, SIGNAL(update_frame()), this, SLOT(repaint()));
}

Yuv_window::~Yuv_window()
{
    XvUngrabPort(QX11Info::display(), port, QX11Info::appUserTime());
}

void Yuv_window::show_frame(unsigned char* _yuv, int fmt, int _width, int _height)
{
    mutex.lock();
    yuv_data = _yuv;
    format = fmt;
    video_width = _width;
    video_height = _height;
    mutex.unlock();
    emit update_frame();
}

void Yuv_window::paintEvent(QPaintEvent *)
{
    GC gc = XCreateGC(QX11Info::display(), winId(), 0, 0);
    mutex.lock();
    if (0 != yuv_data)
    {
        Xv_video_frame* video_frame = new Xv_video_frame(video_width, video_height, format, port, yuv_data);
        XvPutImage(QX11Info::display(), port, winId(), gc, video_frame->image, 0, 0,
                   video_frame->image_width(), video_frame->image_height(), 0, 0, width(), height());
        delete video_frame;
    }
    mutex.unlock();
    XFreeGC(QX11Info::display(), gc);
}

void Yuv_window::moveEvent(QMoveEvent *)
{
    repaint();
}

XvPortID Yuv_window::get_xv_port()
{
    XvPortID port = 0;
    unsigned int count = 0;
    XvAdaptorInfo* info = 0;

    int status = XvQueryAdaptors(QX11Info::display(), QX11Info::appRootWindow(), &count, &info);
    for (int i = 0; i < (int)count; i++)
    {
        printf( "\nAdapter Name : %s, Port base id: 0x%x, Num of Ports: %ld\n", info[i].name, (unsigned int)info[i].base_id, info[i].num_ports);
        for (int j = 0; j < (int)info[j].num_ports; j++)
        {
            port = j+info[i].base_id;
            status = XvGrabPort(QX11Info::display(), port, QX11Info::appUserTime());
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

void Yuv_window::closeEvent(QCloseEvent*)
{
    emit renderer_close();
}
