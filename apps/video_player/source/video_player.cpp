/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <QString>
#include <video_player.h>

Video_player::Video_player()
    :QWidget(0)
    , trick_mode(0)
    , timer(this)
    , slider(Qt::Horizontal, this)
    , button("Pause", this)
    , gray_chk_box("Gray", this)
    , layout(this)
    , state(Media::stop)
    , master("master")
    , window(this)
    , source("yuv")
    , sink("opengl", master.create_child("child"), &window)
{
    initialize();
}

Video_player::~Video_player()
{
    ::disconnect(source, sink);
}

void Video_player::initialize()
{
    layout.addWidget(&button);
    layout.addWidget(&slider);
    layout.addWidget(&gray_chk_box);
    
    setLayout(&layout);
    
    connect_signals_slots();
    
    timer.setInterval(400);
    sink.attach(Media::last_pkt_rendered, this);
}

void Video_player::connect_signals_slots()
{
    ::connect(source, sink);
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    connect(&button, SIGNAL(pressed()), this, SLOT(on_play_pause()));
    connect(&slider, SIGNAL(sliderPressed()), this, SLOT(slider_pressed()));
    connect(&slider, SIGNAL(sliderReleased()), this, SLOT(slider_released()));
    connect(&gray_chk_box, SIGNAL(stateChanged(int)), this, SLOT(on_gray_chk_box_state_change(int)));
}

void Video_player::show()
{
    window.show();
}

int Video_player::start(int start, int end)
{
    int ret = 0;
    timer.start();
    ret = ::start(source, start, end);
    master.start(start);
    if (0 == trick_mode)
    {
        state = Media::play;
        button.setText("Pause");
    }
    MEDIA_LOG("\nStart: %d", start);
    return ret;
}

int Video_player::stop(int& time)
{
    int ret = 0;
    timer.stop();
    ret = ::stop(source, time);
    uint64_t tmp = 0;
    master.stop(tmp);
    state = Media::stop;
    button.setText("Play");
    return ret;
}

int Video_player::duration() const
{
    return source.duration();
}

int Video_player::set_parameters(int width, int height, Media::type fmt, float fps, const char* path)
{
    int ret = source.set_parameters(path, fmt, fps, width, height);
    if (ret == 1)
    {
        slider.setRange(0, source.duration());
        window.setWindowTitle(path);
    }
    return ret;
}

void Video_player::on_timeout()
{
    if (trick_mode)
    {
        int time = slider.value();
        int end = (state == Media::play)?source.duration():time;
        start(time, end);
    }
    else
    {
        slider.setValue(sink.current_position());
    }
}

void Video_player::slider_pressed()
{
    qDebug() << "slider_pressed";
    trick_mode = 1;
    if (state == Media::stop)
    {
        timer.start();
    }
}

void Video_player::slider_released()
{
    qDebug() << "slider_released " << slider.value();
    int time = slider.value();
    int end = (state == Media::play)?source.duration():time;
    start(time, end);
    if (state == Media::stop)
    {
        timer.stop();
    }
    trick_mode = 0;
}

void Video_player::on_play_pause()
{
    if (state == Media::play)
    {
        int time = 0;
        stop(time);
    }
    else
    {
        start(slider.value(), source.duration());
    }
}

void Video_player::on_gray_chk_box_state_change(int state)
{
    qDebug() << "on_gray_chk_box_state_change " << state;
    window.set_gray_scale((Qt::Checked == state));
}

int Video_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    (void)params;
    if (!trick_mode)
    {
        timer.stop();
        on_timeout();
        state == Media::stop;
        slider.setValue(slider.maximum());
    }
    qDebug() << "event_handler: " << event;
    return 0;
}

