/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <math.h>
#include <QString>
#include <yuv_player.h>

#include <QMessageBox>

Yuv_player::Yuv_player()
    :QMainWindow()
	, dlg(this)
    , timer(this)
    , master("master")
    , source("yuv")
    , sink("opengl", master.create_child("child"))
    , text_mode(Yuv_player::time)
    , text_helper(this)
	, mode_grp(0)
	, text_grp(0)
{
    init();
}

Yuv_player::~Yuv_player()
{
    ::disconnect(source, sink);
}

void Yuv_player::init()
{
    setupUi(this);
	init_actions();

	init_player();
    connect_signals_slots();
}

void Yuv_player::init_player()
{
    timer.setInterval(400);
	sink.set_render_widget(centralwidget);
    sink.register_text_helper(&text_helper);
    sink.attach(Media::last_pkt_rendered, this);
    ::connect(source, sink);
}

void Yuv_player::init_actions()
{
	addAction(r_action);
	addAction(g_action);
	addAction(b_action);

	addAction(pbc_action);
	addAction(screen_action);

	addAction(rgb_action);
	addAction(grid_nyuv_action);
	addAction(grid_nrgb_action);

	mode_grp = new QActionGroup(this);
	text_grp = new QActionGroup(this);

	mode_grp->addAction(y_action);
	mode_grp->addAction(u_action);
	mode_grp->addAction(v_action);
	mode_grp->addAction(r_action);
	mode_grp->addAction(g_action);
	mode_grp->addAction(b_action);
	mode_grp->addAction(rgb_action);
	mode_grp->addAction(grid_nyuv_action);
	mode_grp->addAction(grid_nrgb_action);
	rgb_action->setChecked(true);

	text_grp->addAction(none_action);
	text_grp->addAction(fc_action);
	text_grp->addAction(tc_action);
	tc_action->setChecked(true);

	y_action->setData(QVariant(Video_widget::Y));
	u_action->setData(QVariant(Video_widget::U));
	v_action->setData(QVariant(Video_widget::V));
	r_action->setData(QVariant(Video_widget::R));
	g_action->setData(QVariant(Video_widget::G));
	b_action->setData(QVariant(Video_widget::B));
	rgb_action->setData(QVariant(Video_widget::RGB));
	grid_nyuv_action->setData(QVariant(Video_widget::GRID_NYUV));
	grid_nrgb_action->setData(QVariant(Video_widget::GRID_NRGB));

	none_action->setData(QVariant(Yuv_player::none));
	tc_action->setData(QVariant(Yuv_player::time));
	fc_action->setData(QVariant(Yuv_player::frames));
}

void Yuv_player::connect_signals_slots()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
	connect(abt_action, SIGNAL(triggered()), this, SLOT(help_about()));
	connect(open_action, SIGNAL(triggered()), this, SLOT(file_open()));
	connect(screen_action, SIGNAL(triggered()), this, SLOT(change_screen_size()));
	connect(pbc_action, SIGNAL(triggered()), this, SLOT(show_playback_controls()));
	connect(centralwidget, SIGNAL(pb_control(int)), this, SLOT(playback_control(int)));
	connect(mode_grp, SIGNAL(triggered(QAction*)), this, SLOT(change_disp_mode(QAction*)));
	connect(text_grp, SIGNAL(triggered(QAction*)), this, SLOT(change_text_mode(QAction*)));
	connect(centralwidget, SIGNAL(seek(uint64_t, uint64_t)), this, SLOT(slider_seek(uint64_t, uint64_t)));
}

void Yuv_player::change_screen_size()
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

void Yuv_player::change_disp_mode(QAction* action)
{
	Video_widget::Mode mode = (Video_widget::Mode) action->data().toInt();
	centralwidget->set_display_mode(mode);
}

void Yuv_player::change_text_mode(QAction* action)
{
	text_mode = (Yuv_player::Text_mode) action->data().toInt();
	sink.update_pts_text();
}

void Yuv_player::show_playback_controls()
{
	centralwidget->show_playback_controls(!centralwidget->is_controls_visible());
}

void Yuv_player::closeEvent(QCloseEvent* event)
{
	(void)event;
	::disconnect(source, sink);
}

void Yuv_player::file_open()
{
    int ret = dlg.exec();
    if (ret)
    {
		ret = set_parameters(dlg.video_width(), dlg.video_height(), 
				dlg.video_format(), dlg.frame_rate(), 
				dlg.video_file_path().toAscii().data());
		if (ret)
		{
			start(0, 0);
			centralwidget->set_playback_control_state(Video_widget::Play);
		}
    }
}
void Yuv_player::help_about()
{
	QMessageBox::about(this, tr("About"), tr("yuv Player"));
}

void Yuv_player::slider_seek(uint64_t _start, uint64_t _end)
{
	qDebug() << "slider_seek " << _start << ", " << _end;
	start(_start, _end);
}

int Yuv_player::start(int start, int end)
{
    int ret = 0;
    timer.start();
    ret = ::start(source, start, end);
    master.start(start);
	centralwidget->show_playback_controls(true);
    MEDIA_LOG("\nStart: %d", start);
    return ret;
}

int Yuv_player::stop(int& time)
{
    int ret = 0;
    timer.stop();
    ret = ::stop(source, time);
    uint64_t tmp = 0;
    master.stop(tmp);
    return ret;
}

int Yuv_player::set_parameters(int width, int height, Media::type fmt, float fps, const char* path)
{
	QString fpath = path;
    int ret = source.set_parameters(path, fmt, fps, width, height);
    if (ret == 1)
    {
		int i = fpath.length()-fpath.lastIndexOf('/')-1;
        setWindowTitle(QString("yuv player - ")+fpath.right(i));
		centralwidget->set_slider_range(0, source.duration());
    }
    return ret;
}

void Yuv_player::time_out()
{
	centralwidget->set_slider_value(sink.current_position());
}

void Yuv_player::playback_control(int status)
{
	if (status == Video_widget::Pause)
	{
        int time = 0;
        stop(time);
		centralwidget->set_playback_control_state(Video_widget::Play);
    }
	else
    {
		qDebug() << "start " << centralwidget->slider_value() << ", " << source.duration();
        start(centralwidget->slider_value(), source.duration());
		centralwidget->set_playback_control_state(Video_widget::Pause);
    }
}

int Yuv_player::event_handler(Media::events event, Abstract_media_object* obj, Media_params& params)
{
    (void)obj;
    (void)params;
	if (Media::last_pkt_rendered == event)
	{
        timer.stop();
        time_out();
		centralwidget->update();
    }
    qDebug() << "event_handler: " << event;
    return 0;
}

Text_helper::Text_helper(Yuv_player* p)
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
        case Yuv_player::time:
            {
                int sec = time/1000000;
                int min = (sec/60);
                int hr = (min/60);
                min %= 60; sec %= 60;
                snprintf(text, length, "%02d:%02d:%02d:%06d", hr, min, sec, (int)time%1000000);
            }
            break;
        case Yuv_player::frames:
            {
                int curr_frame = 1+(int)ceil(((float)time*player->source.fps())/1000000.0);
                int frame_count = (int)ceil(((float)player->source.duration()*player->source.fps())/1000000.0);
                snprintf(text, length, "%d/%d", curr_frame, frame_count);
            }
            break;
        default:
            strcpy(text, "");
            break;
    }    
}

