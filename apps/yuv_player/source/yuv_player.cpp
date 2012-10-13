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
    , one_shot(this)
    , view_count(1)
    , master("master")
    , source1("source 1")
    , source2("source 2")
    , sink("opengl", master.create_child("child"))
    , text_mode(Yuv_player::time)
    , text_helper(this)
    , tool_bar(this)
    , mode_grp(this)
    , text_grp(this)
    , stereo_grp(this)
{
    init();
}

Yuv_player::~Yuv_player()
{
    ::disconnect(source1, sink);
    ::disconnect(source2, sink);
}

void Yuv_player::init()
{
    setupUi(this);
    init_actions();

    init_player();
    connect_signals_slots();
    update_mixer_actions();
    
    source[0] = &source1;
    source[1] = &source2;
}

void Yuv_player::init_player()
{
    timer.setInterval(400);
    one_shot.setInterval(300);
    one_shot.setSingleShot(true);
    sink.set_render_widget(video);
    sink.register_text_helper(&text_helper);
    sink.attach(Media::last_pkt_rendered, this);
    ::connect(source1, "yuv", sink, "left");
}

void Yuv_player::add_action_group(QActionGroup* act_grp, QAction** const action, int* data, int count)
{
    for (int i = 0; i < count; i++)
    {
        addAction(action[i]);
        action[i]->setData(QVariant(data[i]));
        act_grp->addAction(action[i]);
        tool_bar.addAction(action[i]);
    }
}

void Yuv_player::init_actions()
{
    static int mix_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 26, 27, 48, 49};
    static int txt_data[] = {Yuv_player::none, Yuv_player::frames, Yuv_player::time};
    static int mode_data[] = {Video_widget::Y, Video_widget::U, Video_widget::V, 
                                Video_widget::R, Video_widget::G, Video_widget::B, 
                                Video_widget::RGB, Video_widget::GRID_NYUV, Video_widget::GRID_NRGB};
    QAction* txt_action[] = {none_action, fc_action, tc_action};
    QAction* mode_action[] = {y_action, u_action, v_action, r_action, g_action, b_action, 
                                rgb_action, grid_nyuv_action, grid_nrgb_action};
    QAction* mix_action[] = {left_action, right_action, sub_action, add_action, intleave_action, 
                                nvsleft_action, nvsright_action, nhsleft_action, nhsright_action, 
                                bvsleft_action, bvsright_action, bhsleft_action, bhsright_action};

    addToolBar(Qt::TopToolBarArea, &tool_bar);

    tool_bar.addAction(open_action);
    tool_bar.addAction(stereo_action);
    tool_bar.addSeparator();
    add_action_group(&mode_grp, mode_action, mode_data, sizeof(mode_data)/sizeof(int));
    tool_bar.addSeparator();
    add_action_group(&stereo_grp, mix_action, mix_data, sizeof(mix_data)/sizeof(int));
    tool_bar.addSeparator();
    add_action_group(&text_grp, txt_action, txt_data, sizeof(txt_data)/sizeof(int));

    addAction(pbc_action);
    addAction(screen_action);
    pbc_action->setEnabled(false);
    actsize_action->setEnabled(false);

    mode_grp.setEnabled(false);
    text_grp.setEnabled(false);

    tc_action->setChecked(true);
    rgb_action->setChecked(true);
    pbc_action->setChecked(true);
    intleave_action->setChecked(true);
}

void Yuv_player::connect_signals_slots()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(time_out()));
    connect(abt_action, SIGNAL(triggered()), this, SLOT(help_about()));
    connect(open_action, SIGNAL(triggered()), this, SLOT(file_open()));
    connect(actsize_action, SIGNAL(triggered()), this, SLOT(actual_size()));
    connect(stereo_action, SIGNAL(triggered()), this, SLOT(file_stereo_open()));
    connect(&one_shot, SIGNAL(timeout()), this, SLOT(one_shot_timeout()));
    connect(screen_action, SIGNAL(triggered()), this, SLOT(change_screen_size()));
    connect(pbc_action, SIGNAL(triggered()), this, SLOT(show_playback_controls()));
    connect(video, SIGNAL(pb_control(int)), this, SLOT(playback_control(int)));
    connect(&mode_grp, SIGNAL(triggered(QAction*)), this, SLOT(change_disp_mode(QAction*)));
    connect(&text_grp, SIGNAL(triggered(QAction*)), this, SLOT(change_text_mode(QAction*)));
    connect(&stereo_grp, SIGNAL(triggered(QAction*)), this, SLOT(change_stereo_mode(QAction*)));
    connect(video, SIGNAL(seek(uint64_t, uint64_t)), this, SLOT(slider_seek(uint64_t, uint64_t)));
}

void Yuv_player::update_mixer_actions()
{
    stereo_grp.setEnabled(view_count == 2);
}

void Yuv_player::change_screen_size()
{
    if (isFullScreen())
    {
        menuBar()->show();
        statusBar()->show();
        tool_bar.show();
        showNormal();
    }
    else
    {
        menuBar()->hide();
        statusBar()->hide();
        tool_bar.hide();
        showFullScreen();
    }
}

void Yuv_player::change_disp_mode(QAction* action)
{
    Video_widget::Mode mode = (Video_widget::Mode) action->data().toInt();
    video->set_display_mode(mode);
}

void Yuv_player::change_text_mode(QAction* action)
{
    text_mode = (Yuv_player::Text_mode) action->data().toInt();
    sink.update_pts_text();
}

void Yuv_player::change_stereo_mode(QAction* action)
{
    int mode = action->data().toInt();
    video->set_stereo_mode(mode);
}

void Yuv_player::show_playback_controls()
{
    video->show_playback_controls(!video->is_controls_visible());
}

void Yuv_player::closeEvent(QCloseEvent* event)
{
    (void)event;
    ::disconnect(source1, sink);
    ::disconnect(source2, sink);
}

void Yuv_player::file_open()
{
    dlg.set_stereo_mode(false);
    int ret = dlg.exec();
    if (ret)
    {
        ::disconnect(source1, sink);
        ::disconnect(source2, sink);

        ::connect(source1, "yuv", sink, "left");

        view_count = 1;
        ret = set_source_parameters();
        if (ret)
        {
            start(0, 0);
        }
    }
}

void Yuv_player::file_stereo_open()
{
    dlg.set_stereo_mode(true);
    int ret = dlg.exec();
    if (ret)
    {
        ::disconnect(source1, sink);
        ::disconnect(source2, sink);

        ::connect(source1, "yuv", sink, "left");
        ::connect(source2, "yuv", sink, "right");

        view_count = 2;
        ret = set_source_parameters();
        if (ret == 1)
        {	
            start(0, 0);
        }
    }
}

void Yuv_player::help_about()
{
    QMessageBox::information(this, tr("About"), 
        tr("yuv playback and comparison\n"
           "utility based on mmfcpp framework\n"
           "http://code.google.com/p/mmfcpp/"),
        QMessageBox::NoButton);
}

void Yuv_player::slider_seek(uint64_t _start, uint64_t _end)
{
    mutex.lock();
    pb_stack.push(QPair<uint64_t, uint64_t>(_start, _end));
    if (!one_shot.isActive())
    {
        one_shot.start();
    }
    mutex.unlock();
}

int Yuv_player::start(int start, int end)
{
    int ret = 1;
    timer.start();
    for (int i = 0; i < view_count; i++)
    {
        ret = /*ret &&*/ ::start(source[i], start, end);
    }
    master.start(start);
    if (start == end && start < video_duration())
    {
        video->set_playback_control_state(Video_widget::Play);
    }
    else
    {
        enable_file_actions(false);
        video->set_playback_control_state(Video_widget::Pause);
    }
    MEDIA_LOG("\nStart: %d", start);
    return ret;
}

int Yuv_player::stop(int& time)
{
    int ret = 1;
    uint64_t tmp = 0;
    timer.stop();
    for (int i = 0; i < view_count; i++)
    {
        ret = /*ret &&*/ ::stop(source[i], time);
    }
    video->set_playback_control_state(Video_widget::Play);
    master.stop(tmp);
    time_out();
    enable_file_actions(true);
    return ret;
}

int Yuv_player::video_duration()
{
    int ret = source[0]->duration();
    for (int i = 1; i < view_count; i++)
    {
        int tmp = source[i]->duration();
        ret = (tmp < ret)?tmp:ret;
    }
    return ret;
}

int Yuv_player::set_source_parameters()
{
    int ret = 1;
    QString title("yuv player -");
    for (int i = 0; i < view_count; i++)
    {
        ret = ret && source[i]->set_parameters(dlg.video_file_path(i).toAscii().data(), dlg.video_format(i), 
                    dlg.frame_rate(), dlg.video_width(i), dlg.video_height(i));
        QString fpath = dlg.video_file_path(i);
        int index = fpath.length()-fpath.lastIndexOf('/')-1;
        title = title+" "+fpath.right(index);
    }

    if (ret == 1)
    {
        setWindowTitle(title);
        video->set_slider_range(0, video_duration());

        pbc_action->setEnabled(true);
        pbc_action->setChecked(true);
        actsize_action->setEnabled(true);

        mode_grp.setEnabled(true);
        text_grp.setEnabled(true);

        video->show_playback_controls(true);
        update_mixer_actions();
    }
    return ret;
}

int Yuv_player::set_parameters(int width, int height, Media::type fmt, float fps, const char* path)
{
    dlg.set_parameters(0, fmt, fps, width, height, path);
    return set_source_parameters();
}

void Yuv_player::time_out()
{
    video->set_slider_value(sink.current_position());
}

void Yuv_player::one_shot_timeout()
{
    if (!pb_stack.isEmpty())
    {
        mutex.lock();
        QPair<uint64_t, uint64_t> time = pb_stack.pop();
        pb_stack.clear();
        mutex.unlock();
        start(time.first, time.second);
    }
}

void Yuv_player::playback_control(int status)
{
    if (status == Video_widget::Pause)
    {
        int time = 0;
        stop(time);
    }
    else
    {
        start(video->slider_value(), video_duration());
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
        video->update();
        enable_file_actions(true);
    }
    return 0;
}

void Yuv_player::enable_file_actions(bool status)
{
    open_action->setEnabled(status);
    stereo_action->setEnabled(status);
}

void Yuv_player::actual_size()
{
    int wwidth = 0, wheight = 0;
    for (int i = 0; i < view_count; i++)
    {
        if (wwidth < dlg.video_width(i))
        {
            wwidth = dlg.video_width(i);
        }
        if (wheight < dlg.video_height(i))
        {
            wheight = dlg.video_height(i);
        }
    }

    QAction* action = mode_grp.checkedAction();
    int mode = action->data().toInt();
    if (mode > Video_widget::RGB)
    {
        wwidth *= 2;
        wheight *= 2;
    }

    wheight += menuBar()->isVisible()?menuBar()->height():0;
    wheight += statusBar()->isVisible()?statusBar()->height():0;
    wheight += tool_bar.isVisible()?tool_bar.height():0;

    resize(wwidth, wheight);
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
                int curr_frame = 1+(int)ceil(((float)time*player->source1.fps())/1000000.0);
                int frame_count = (int)ceil(((float)player->video_duration()*player->source1.fps())/1000000.0);
                snprintf(text, length, "%d/%d", curr_frame, frame_count);
            }
            break;
        default:
            strcpy(text, "");
            break;
    }    
}

