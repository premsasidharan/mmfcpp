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

#include <ui_yuv_dlg.h>

class Yuv_dlg:public QDialog, public Ui_yuv_dlg
{
    Q_OBJECT
public:
    enum VIDEO_RES { QCIF, CIF, SD, HD, FHD, CUSTOM };

    Yuv_dlg(QWidget* parent = 0);
    ~Yuv_dlg();

public:
    float frame_rate() const { return fps; };
    int video_width() const { return width; };
    int video_height() const { return height; };
    Media::type video_format() const { return format; };
    const QString video_file_path() const { return path; };
    
protected:
    void initialize();
    void set_validators();
    void connect_signals();
    void initialize_controls();
    void validate_parameters();

protected slots:
    void start_playback();
    void set_yuv_file_path();
    void resolution_change(int index);
    void fps_changed(const QString& text);
    void width_changed(const QString& text);
    void height_changed(const QString& text);
    void format_change(const QString& text); 

private:
    float fps;
    int width;
    int height;
    QString path;
    Media::type format;
};

#endif

