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

Yuv_dlg::Yuv_dlg(QWidget* parent)
    :QDialog(parent)
    , fps(24.0)
    , width(352)
    , height(288)
    , path("")
    , format(Media::I420)
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
    connect(start_btn, SIGNAL(clicked()), this, SLOT(start_playback()));
    connect(file_btn, SIGNAL(clicked()), this, SLOT(set_yuv_file_path()));

    connect(res_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_change(int)));
    connect(fmt_cbox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(format_change(const QString&)));

    connect(fps_edit, SIGNAL(textChanged(const QString&)), this, SLOT(fps_changed(const QString&)));
    connect(xres_edit, SIGNAL(textChanged(const QString&)), this, SLOT(width_changed(const QString&)));
    connect(yres_edit, SIGNAL(textChanged(const QString&)), this, SLOT(height_changed(const QString&)));
}

void Yuv_dlg::initialize_controls()
{
    fps_edit->setText("24.00");
    start_btn->setEnabled(false);
    path_edit->setEnabled(false);
    res_cbox->setCurrentIndex(Yuv_dlg::CIF);
}

void Yuv_dlg::start_playback()
{
    QDialog::accept();
}

void Yuv_dlg::set_yuv_file_path()
{
    path = QFileDialog::getOpenFileName(this, tr("Select Yuv File"),
                             path, tr("Yuv File (*.yuv)"));
    path_edit->setText(path);
    validate_parameters();
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

void Yuv_dlg::format_change(const QString& text)
{
    if (text == "YUV420")
    {
        format = Media::I420;
    }
    else if (text == "YUV422")
    {
        format = Media::I422;
    }
    else if (text == "YUV444")
    {
        format = Media::I444;
    }
    else if (text == "YUY2")
    {
        format = Media::YUY2;
    }
    else if (text == "UYVY")
    {
        format = Media::UYVY;
    }
    else if (text == "YV12")
    {
        format = Media::YV12;
    }
}

void Yuv_dlg::fps_changed(const QString& text)
{
    fps = text.toFloat();
    validate_parameters();
}

void Yuv_dlg::width_changed(const QString& text)
{
    width = text.toInt();
    validate_parameters();
}

void Yuv_dlg::height_changed(const QString& text)
{
    height = text.toInt();
    validate_parameters();
}

void Yuv_dlg::validate_parameters()
{
    bool ok = (width >= 176) && (width <= 1920);
    ok = ok && ((height >= 144) && (height <= 1080));
    ok = ok && (fps > 0.0f && fps <= 60.0f);
    ok = ok && QFile::exists(path);
    start_btn->setEnabled(ok);
}

