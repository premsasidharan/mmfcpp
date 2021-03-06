/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _YUV_PLAYER_H_
#define _YUV_PLAYER_H_

#include <QTimer>
#include <QStack>
#include <QSlider>
#include <QToolBar>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QMainWindow>

#include <yuv_dlg.h>

#include <master_clock.h>

#include <video_widget.h>
#include <yuv_file_src.h>
#include <video_renderer.h>

#include <ui_main_window.h>

class Yuv_player;

class Text_helper: public Abstract_text_helper
{
public:
    Text_helper(Yuv_player* p);
    ~Text_helper();

public:
    void read_text(char* text, int length, uint64_t time);

private:
    Yuv_player* player;
};

class Yuv_player:public QMainWindow, private Ui_yuv_player, public Observer
{
    Q_OBJECT
public:
    friend class Text_helper;

    enum Text_mode {none, time, frames};

    Yuv_player();
    ~Yuv_player();

public:
    int stop(int& time);
    int start(int start, int end);

    int set_parameters(int width, int height, Media::type fmt, float fps, const char* path);

protected:
    void init();
    void init_player();
    void init_actions();
    int set_source_parameters();
    void connect_signals_slots();
    void closeEvent(QCloseEvent* event);
    int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params);

    int video_duration();
    void update_mixer_actions();
    void enable_file_actions(bool status);
    void add_action_group(QActionGroup* act_grp, QAction** const action, int* data, int count);

protected slots:
    void file_open();
    void help_about();
    void actual_size();
    void file_stereo_open();
    void one_shot_timeout();
    void change_screen_size();
    void show_playback_controls();
    void playback_control(int status);
    void change_disp_mode(QAction* action);
    void change_text_mode(QAction* action);
    void change_stereo_mode(QAction* action);
    void slider_seek(uint64_t _start, uint64_t _end);

private:
    Yuv_dlg dlg;

    QTimer one_shot;

    int view_count;

    Master_clock master;

    Yuv_file_src source1;
    Yuv_file_src source2;
    Yuv_file_src* source[2];

    Video_renderer sink;

    Text_mode text_mode;
    Text_helper text_helper;

    QToolBar tool_bar;
    QActionGroup mode_grp;
    QActionGroup text_grp;
    QActionGroup stereo_grp;

    QMutex mutex;
    QStack<QPair<uint64_t, uint64_t> > pb_stack;
};

#endif
