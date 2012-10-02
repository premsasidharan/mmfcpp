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
    int video_width(int view) const { return width[view]; };
    int video_height(int view) const { return height[view]; };
    Media::type video_format(int view) const { return format[view]; };
    const QString video_file_path(int view) const { return path[view]; };

	void set_stereo_mode(bool status);
	void set_parameters(int view, Media::type fmt, float f, int width, int height, const char* path);
    
protected:
    void initialize();
    void set_validators();
    void connect_signals();
    void initialize_controls();
    void validate_parameters();

	void update_format_change(int index, const QString& text);
	void update_res_text(QLineEdit* edit1, QLineEdit* edit2, int index);

protected slots:
    void start_playback();

    void fps_changed(const QString& text);

    void set_yuv_file_path();
    void resolution_change(int index);
    void format_change(const QString& text); 
    void width_changed(const QString& text);
    void height_changed(const QString& text);

    void set_yuv_file_path_1();
    void resolution_change_1(int index);
    void format_change_1(const QString& text); 
    void width_changed_1(const QString& text);
    void height_changed_1(const QString& text);

private:
    float fps;
    int width[2];
    int height[2];
    QString path[2];
    Media::type format[2];
};

#endif

