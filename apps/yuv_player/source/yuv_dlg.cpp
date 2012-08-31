/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <yuv_dlg.h>

#include <QDebug>
#include <QFileDialog>

Yuv_dlg::Yuv_dlg(Video_player* _player)
    :QDialog(0)
    , fps(24.0)
    , width(352)
    , height(288)
    , path("")
    , format(Media::I420)
    , player(_player)
{
    setupUi(this);

    initialize();
}

Yuv_dlg::~Yuv_dlg()
{
}

void Yuv_dlg::initialize()
{
    set_validators();
    connect_signals();
    initialize_controls();
}

void Yuv_dlg::set_validators()
{
    xres_edit->setValidator(new QIntValidator(176, 1920, this));
    yres_edit->setValidator(new QIntValidator(144, 1080, this));
    fps_edit->setValidator(new QDoubleValidator(1.0, 60.0, 2, this));
}

void Yuv_dlg::connect_signals()
{
    connect(file_btn, SIGNAL(clicked()), this, SLOT(set_yuv_file_path()));
    connect(start_btn, SIGNAL(clicked()), this, SLOT(start_playback()));
    connect(res_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_change(int)));
}

void Yuv_dlg::initialize_controls()
{
    fps_edit->setText("24.00");
    res_cbox->setCurrentIndex(Yuv_dlg::CIF);
}

void Yuv_dlg::start_playback()
{
    QDialog::accept();
    player->set_parameters(xres_edit->text().toInt(), 
        yres_edit->text().toInt(), Media::I420, 
        fps_edit->text().toFloat(), path.toAscii().data());
}

void Yuv_dlg::set_yuv_file_path()
{
    path = QFileDialog::getOpenFileName(this, tr("Select Yuv File"),
                             path, tr("Yuv File (*.yuv)"));
    path_edit->setText(path);
}

void Yuv_dlg::resolution_change(int index)
{
    xres_edit->setEnabled(index == Yuv_dlg::CUSTOM);
    yres_edit->setEnabled(index == Yuv_dlg::CUSTOM);

    switch (index)
    {
        case Yuv_dlg::QCIF:
            xres_edit->setText("176");
            yres_edit->setText("144");
            break;
        case Yuv_dlg::CIF:
            xres_edit->setText("352");
            yres_edit->setText("288");
            break;
        case Yuv_dlg::SD:
            xres_edit->setText("640");
            yres_edit->setText("480");
            break;
        case Yuv_dlg::HD:
            xres_edit->setText("1280");
            yres_edit->setText("720");
            break;
        case Yuv_dlg::FHD:
            xres_edit->setText("1920");
            yres_edit->setText("1080");
            break;
        case Yuv_dlg::CUSTOM:
            xres_edit->setText("");
            yres_edit->setText("");
            break;
    }
}

