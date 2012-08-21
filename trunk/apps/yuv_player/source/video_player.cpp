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
    , show_chk_box("More", this)
    , luma_radio("Luma (Y)", this)
    , chru_radio("Chroma.U", this)
    , chrv_radio("Chroma,V", this)
    , red_radio("Red", this)
    , blue_radio("Blue", this)
    , green_radio("Green", this)
    , norm_radio("Normal", this)
    , nyuv_radio("Normal.Y.U.V", this)
    , nrgb_radio("Normal.R.G.B", this)
    , vert_layout(this)
    , horz_layout(0)
    , grid_layout(0)
    , state(Media::stop)
    , master("master")
    , window(this)
    , source("yuv")
    , sink("opengl", master.create_child("child"), &window)
{
    resize(100, 50);
    initialize();
}

Video_player::~Video_player()
{
    ::disconnect(source, sink);
}

void Video_player::initialize()
{
    horz_layout.addWidget(&button);
    horz_layout.addWidget(&slider);
    horz_layout.addWidget(&show_chk_box);

    grid_layout.addWidget(&luma_radio, 0, 0); 
    grid_layout.addWidget(&red_radio, 0, 1); 
    grid_layout.addWidget(&norm_radio, 0, 2); 

    grid_layout.addWidget(&chru_radio, 1, 0);
    grid_layout.addWidget(&green_radio, 1, 1);
    grid_layout.addWidget(&nyuv_radio, 1, 2);

    grid_layout.addWidget(&chrv_radio, 2, 0);
    grid_layout.addWidget(&blue_radio, 2, 1);
    grid_layout.addWidget(&nrgb_radio, 2, 2);
    
    vert_layout.addLayout(&grid_layout);
    vert_layout.addLayout(&horz_layout);
    setLayout(&vert_layout);
    
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
    connect(&show_chk_box, SIGNAL(stateChanged(int)), this, SLOT(on_show(int)));
   
    connect(&luma_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&chru_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&chrv_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&red_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&green_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&blue_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&norm_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&nyuv_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));
    connect(&nrgb_radio, SIGNAL(toggled(bool)), this, SLOT(on_mode_change(bool)));

    norm_radio.setChecked(true);
    show_radio_controls(false);
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

void Video_player::on_mode_change(bool status)
{
    int mode = -1;

    if (false == status)
    {
        return;
    }

    if (luma_radio.isChecked())
    {
        mode = 0;
    }
    else if (chru_radio.isChecked())
    {
        mode = 1;
    }
    else if (chrv_radio.isChecked())
    {
        mode = 2;
    }
    else if (red_radio.isChecked())
    {
        mode = 3;
    }
    else if (green_radio.isChecked())
    {
        mode = 4;
    }
    else if (blue_radio.isChecked())
    {
        mode = 5;
    }
    else if (norm_radio.isChecked())
    {
        mode = 6;
    }
    else if (nyuv_radio.isChecked())
    {
        mode = 7;
    }
    else if (nrgb_radio.isChecked())
    {
        mode = 8;
    }

    if (mode >= 0)
    {
        window.set_mode(mode);
    }
    qDebug() << "on_mode_change " << mode;
}

void Video_player::on_show(int state)
{
    qDebug() << "on_show " << (state == Qt::Checked);
    show_radio_controls((state == Qt::Checked));
    if (state == Qt::Checked)
    {
        resize(width(), 120);
        move(x(), window.height()-130);
    }
    else
    {
        resize(width(), 50);
        move(x(), window.height()-60);
    }    
}

void Video_player::show_radio_controls(bool ok)
{
    if (ok)
    {
        luma_radio.show();
        chru_radio.show();
        chrv_radio.show();

        red_radio.show();
        green_radio.show();
        blue_radio.show();

        norm_radio.show();
        nyuv_radio.show();
        nrgb_radio.show();
    }
    else
    {
        luma_radio.hide();
        chru_radio.hide();
        chrv_radio.hide();

        red_radio.hide();
        green_radio.hide();
        blue_radio.hide();

        norm_radio.hide();
        nyuv_radio.hide();
        nrgb_radio.hide(); 
    }
}

void Video_player::resizeEvent(QResizeEvent* event)
{
}

int Video_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    (void)params;
    if (!trick_mode)
    {
        timer.stop();
        on_timeout();
        state == Media::stop;
    }
    qDebug() << "event_handler: " << event;
    return 0;
}

