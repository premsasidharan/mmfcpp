/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <math.h>
#include <QString>
#include <video_player.h>

#include <QMessageBox>

Video_player::Video_player()
    :QMainWindow()
    , trick_mode(0)
    , timer(this)
    , state(Media::stop)
    , master(0)
    , source(0)
    , sink(0)
    , text_mode(Video_player::time_code)
    , text_helper(this)
{
    setupUi(this);
    initialize();
}

Video_player::~Video_player()
{
    ::disconnect(source, sink);
	delete sink; sink = 0;
	delete source; source = 0;
	delete master; master = 0;
}

void Video_player::initialize()
{
    master = new Master_clock("master");
    source = new Yuv_file_src("yuv");
    sink = new Video_renderer("opengl", master->create_child("child"), centralwidget);
    connect_signals_slots();
    
    timer.setInterval(400);
    sink->attach(Media::last_pkt_rendered, this);

    sink->register_text_helper(&text_helper);
}

void Video_player::connect_signals_slots()
{
    ::connect(source, sink);

	addAction(screen);
	addAction(view_progress);

	//TODO:Need to cleanup the whole thing
	QActionGroup* yuv_act_grp = new QActionGroup(this);
	yuv_act_grp->addAction(luma_y);
	yuv_act_grp->addAction(chroma_u);
	yuv_act_grp->addAction(chroma_v);
	yuv_act_grp->addAction(red);
	yuv_act_grp->addAction(green);
	yuv_act_grp->addAction(blue);
	yuv_act_grp->addAction(norm);
	yuv_act_grp->addAction(nyuv_combo);
	yuv_act_grp->addAction(nrgb_combo);

	norm->setChecked(true);
    
	QActionGroup* text_act_grp = new QActionGroup(this);
	text_act_grp->addAction(text_none);
	text_act_grp->addAction(text_fc);
	text_act_grp->addAction(text_tc);
	text_tc->setChecked(true);

	addAction(red);
	addAction(green);
	addAction(blue);

	addAction(norm);
	addAction(nyuv_combo);
	addAction(nrgb_combo);

    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));

	connect(screen, SIGNAL(triggered()), this, SLOT(change_screen_size()));
	connect(view_progress, SIGNAL(triggered()), this, SLOT(show_hide_progress_bar()));

	connect(luma_y, SIGNAL(triggered()), this, SLOT(mode_luma()));
	connect(chroma_u, SIGNAL(triggered()), this, SLOT(mode_chromau()));
	connect(chroma_v, SIGNAL(triggered()), this, SLOT(mode_chromav()));

	connect(red, SIGNAL(triggered()), this, SLOT(mode_red()));
	connect(green, SIGNAL(triggered()), this, SLOT(mode_green()));
	connect(blue, SIGNAL(triggered()), this, SLOT(mode_blue()));

	connect(norm, SIGNAL(triggered()), this, SLOT(mode_normal()));
	connect(nyuv_combo, SIGNAL(triggered()), this, SLOT(mode_nyuv()));
	connect(nrgb_combo, SIGNAL(triggered()), this, SLOT(mode_nrgb()));

	connect(text_none, SIGNAL(triggered()), this, SLOT(text_mode_none()));
	connect(text_fc, SIGNAL(triggered()), this, SLOT(text_mode_frame_count()));
	connect(text_tc, SIGNAL(triggered()), this, SLOT(text_mode_time_code()));

	connect(about, SIGNAL(triggered()), this, SLOT(help_about()));

	connect(centralwidget, SIGNAL(pb_control(int)), this, SLOT(playback_control(int)));
	connect(centralwidget, SIGNAL(seek(uint64_t, uint64_t)), this, SLOT(slider_seek(uint64_t, uint64_t)));
}

void Video_player::change_screen_size()
{
	if (isFullScreen())
	{
		menuBar()->show();
		statusBar()->show();
		showNormal();
	}
	else
	{
		menuBar()->hide();
		statusBar()->hide();
		showFullScreen();
	}
}

void Video_player::show_hide_progress_bar()
{
	centralwidget->enable_progress_bar(!centralwidget->is_progress_bar_enabled());
}

void Video_player::mode_luma()
{
	centralwidget->set_mode(0);
}

void Video_player::mode_chromau()
{
	centralwidget->set_mode(1);
}

void Video_player::mode_chromav()
{
	centralwidget->set_mode(2);
}

void Video_player::mode_red()
{
	centralwidget->set_mode(3);
}

void Video_player::mode_green()
{
	centralwidget->set_mode(4);
}

void Video_player::mode_blue()
{
	centralwidget->set_mode(5);
}

void Video_player::mode_normal()
{
	centralwidget->set_mode(6);
}

void Video_player::mode_nyuv()
{
	centralwidget->set_mode(7);
}

void Video_player::mode_nrgb()
{
	centralwidget->set_mode(8);
}

void Video_player::text_mode_none()
{
	text_mode = Video_player::no_text;
}

void Video_player::text_mode_time_code()
{
	text_mode = Video_player::time_code;
}

void Video_player::text_mode_frame_count()
{
	text_mode = Video_player::frame_count;
}

void Video_player::help_about()
{
	QMessageBox::information(this, tr("About"), tr("Yuv Player"));
}

void Video_player::slider_seek(uint64_t _start, uint64_t _end)
{
	qDebug() << "slider_seek " << _start << ", " << _end;
	start(_start, _end);
}

int Video_player::start(int start, int end)
{
    int ret = 0;
    timer.start();
    ret = ::start(source, start, end);
    master->start(start);
    if (0 == trick_mode)
    {
        state = Media::play;
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
    master->stop(tmp);
    state = Media::stop;
    return ret;
}

float Video_player::fps() const
{
    return source->fps();
}

int Video_player::duration() const
{
    return source->duration();
}

int Video_player::set_parameters(int width, int height, Media::type fmt, float fps, const char* path)
{
    int ret = source->set_parameters(path, fmt, fps, width, height);
    if (ret == 1)
    {
        setWindowTitle(path);
		centralwidget->set_slider_range(0, source->duration());
    }
    return ret;
}

void Video_player::time_out()
{
	centralwidget->set_value(sink->current_position());
}

void Video_player::playback_control(int status)
{
	if (status == 0)
	{
        int time = 0;
        stop(time);
		centralwidget->set_pb_control_status(1);
    }
	else
    {
		qDebug() << "start " << centralwidget->current_pos() << ", " << source->duration();
        start(centralwidget->current_pos(), source->duration());
		centralwidget->set_pb_control_status(0);
    }
}

int Video_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    (void)event;
    (void)obj;
    (void)params;
    if (!trick_mode)
    {
        timer.stop();
        time_out();
        state == Media::stop;
    }
    qDebug() << "event_handler: " << event;
    return 0;
}

void Video_player::set_text_mode(Video_player::Text_mode mode)
{
    text_mode = mode;
}

Text_helper::Text_helper(Video_player* p)
    :player(p)
{
}

Text_helper::~Text_helper()
{
}

void Text_helper::read_text(char* text, int length, uint64_t time)
{
    switch (player->text_mode)
    {
        case Video_player::time_code:
            {
                int sec = time/1000000;
                int min = (sec/60);
                int hr = (min/60);
                min %= 60; sec %= 60;
                snprintf(text, length, "%02d:%02d:%02d:%06d", hr, min, sec, time%1000000);
            }
            break;
        case Video_player::frame_count:
            {
                int curr_frame = 1+(int)ceil(((float)time*player->fps())/1000000.0);
                int frame_count = (int)ceil(((float)player->duration()*player->fps())/1000000.0);
                snprintf(text, length, "%d/%d", curr_frame, frame_count);
            }
            break;
        default:
            snprintf(text, length, "");
            break;
    }    
}

