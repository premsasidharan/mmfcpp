/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <audio_player.h>
#include <player_window.h>

#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>

Player_window::Player_window(Audio_player& _player)
    :QMainWindow(0)
    , player(_player)
{
    setupUi(this);
    initialize();
}

Player_window::~Player_window()
{
}

void Player_window::initialize()
{
    connect(file_open, SIGNAL(triggered()), this, SLOT(on_file_open()));
    connect(play_pause_btn, SIGNAL(clicked()), this, SLOT(on_play_pause()));
    connect(playback_slider, SIGNAL(sliderPressed()), this, SLOT(slider_pressed()));
    connect(playback_slider, SIGNAL(sliderReleased()), this, SLOT(slider_released()));
}

void Player_window::on_file_open()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Wave File (*.wav)"));
    if (player.set_file_path(path.toAscii()))
    {
        pcm_widget->set_channels(player.channels());
        playback_slider->setRange(0, player.duration());
        playback_slider->setValue(0);
    }
}

void Player_window::on_play_pause()
{
    if (play_pause_btn->text() == tr("Play"))
    {
        player.start(0);
        play_pause_btn->setText(tr("Pause"));
    }
    else
    {
        int time = 0;
        player.stop(time);
        play_pause_btn->setText(tr("Play"));
    }
}

void Player_window::set_current_position(int pos)
{
    playback_slider->setValue(pos);
    //qDebug() << "Here " << pos << ", " << playback_slider->maximum();
}

void Player_window::set_track_data(int channel, int32_t* data, int size, int max_value)
{
    pcm_widget->set_track_data(channel, data, size, max_value);
}

void Player_window::slider_pressed()
{
    int time = 0;
    player.stop(time);
    qDebug() << "slider_pressed";
}

void Player_window::slider_released()
{
    int time = playback_slider->value();
    player.start(time);
    qDebug() << "slider_released " << time;
}
