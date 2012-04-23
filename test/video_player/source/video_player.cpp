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
    , timer(this)
    , slider(Qt::Horizontal, this)
    , trick_mode(0)
    , button("Pause", this)
    , layout(this)
    , window(this)
    , source("yuv")
    , sink("opengl", &window)
{
    layout.addWidget(&button);
    layout.addWidget(&slider);
    setLayout(&layout);
    
    initialize();
}

Video_player::~Video_player()
{
    ::disconnect(source, sink);
}

void Video_player::initialize()
{
    ::connect(source, sink);
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    connect(&button, SIGNAL(pressed()), this, SLOT(on_play_pause()));
    connect(&slider, SIGNAL(sliderPressed()), this, SLOT(slider_pressed()));
    connect(&slider, SIGNAL(sliderReleased()), this, SLOT(slider_released()));
    
    timer.setInterval(400);
    
    sink.attach(Media::last_pkt_rendered, this);
}
    
void Video_player::show()
{
    window.show();
}

int Video_player::start(int start, int end)
{
    timer.start();
    return ::start(source, start, end);
}

int Video_player::stop(int& time)
{
    int ret = 0;
    timer.stop();
    ret = ::stop(source, time);
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
    }
    return ret;
}

void Video_player::on_timeout()
{
    if (trick_mode)
    {
        int time = slider.value();
        int end = (button.text() == QString("Pause"))?source.duration():time;
        ::start(source, time, end);
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
    if (button.text() == QString("Play"))
    {
        timer.start();
    }
}

void Video_player::slider_released()
{
    trick_mode = 0;
    qDebug() << "slider_released " << slider.value();
    int start = slider.value();
    int end = (button.text() == QString("Pause"))?source.duration():start;
    ::start(source, start, end);
    if (button.text() == QString("Play"))
    {
        timer.stop();
    }
}

void Video_player::on_play_pause()
{
    if (button.text() == QString("Pause"))
    {
        int time = 0;
        timer.stop();
        ::stop(source, time);
        button.setText("Play");
    }
    else
    {
        ::start(source, slider.value(), source.duration());
        timer.start();
        button.setText("Pause");
    }
}

int Video_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    if (!trick_mode)
    {
        timer.stop();
        on_timeout();
    }
    qDebug() << "event_handler: " << event;
    return 0;
}
