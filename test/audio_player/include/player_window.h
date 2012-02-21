/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _PLAYER_WINDOW_H_
#define _PLAYER_WINDOW_H_

#include <ui_player_window.h>

#include <QTimer>
#include <QMainWindow>

#include <audio_player.h>

class Player_window:public QMainWindow, public Ui::Player_window_ui
{
    Q_OBJECT
public:
    Player_window();
    ~Player_window();

protected:
    void initialize();

protected slots:
    void on_file_open();
    void on_play_pause();
    void on_timer_elapsed();

private:
    QTimer timer;
    Audio_player player;
};

#endif
