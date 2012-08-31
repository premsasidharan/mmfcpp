/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _YUV_DLG_H_
#define _YUV_DLG_H_

#include <QDialog>

#include <media.h>
#include <video_player.h>

#include <ui_yuv_dlg.h>

class Yuv_dlg:public QDialog, public Ui_yuv_dlg
{
    Q_OBJECT
public:
    enum VIDEO_RES { QCIF, CIF, SD, HD, FHD, CUSTOM };

    Yuv_dlg(Video_player* _player);
    ~Yuv_dlg();

protected:
    void initialize();
    void set_validators();
    void connect_signals();
    void initialize_controls();

protected slots:
    void start_playback();
    void set_yuv_file_path();
    void resolution_change(int index);

private:
    float fps;
    int width;
    int height;
    QString path;
    Media::type format;
    Video_player* player;
};

#endif

