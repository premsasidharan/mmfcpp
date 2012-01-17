/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _X11_WINDOW_H
#define _X11_WINDOW_H

#include <QtGui>
#include <QtDebug>
#include <QX11Info>
#include <QMutex>

#include <xv_video_frame.h>

class Yuv_window:public QWidget
{
    Q_OBJECT
public:
    Yuv_window(int x, int y, int width, int height);
    ~Yuv_window();

public:
    void show_frame(unsigned char* _yuv, int width, int fmt, int height);

signals:
    void update_frame();
    void renderer_close();

protected:
    void moveEvent(QMoveEvent*);
    void paintEvent(QPaintEvent*);
    void closeEvent(QCloseEvent*);

private:
    XvPortID get_xv_port();

private:
    XvPortID port;
    int video_width;
    int video_height;
    unsigned int format;
    unsigned char* yuv_data;

    QMutex mutex;
};

#endif
