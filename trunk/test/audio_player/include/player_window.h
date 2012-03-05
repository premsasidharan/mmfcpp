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

class Audio_player;

class Player_window:public QMainWindow, public Ui::Player_window_ui
{
    Q_OBJECT
public:
    Player_window(Audio_player& _player);
    ~Player_window();
    
public:
    void set_current_position(int pos);
    void set_track_data(int channel, int32_t* data, int size, int max_value);

protected:
    void initialize();

protected slots:
    void on_file_open();
    void on_play_pause();

private:
    Audio_player& player;
};

#endif
