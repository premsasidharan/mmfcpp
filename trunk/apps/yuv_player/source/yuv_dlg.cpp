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
{
    setupUi(this);
    initialize();
}

Yuv_dlg::~Yuv_dlg()
{
}

void Yuv_dlg::initialize()
{
    path[0] = path[1] = "";
    width[0] = width[1] = 352;
    height[0] = height[1] = 288;
    format[0] = format[1] = Media::I420;

    set_validators();
    connect_signals();
    initialize_controls();
}

void Yuv_dlg::set_validators()
{
    xres_edit->setValidator(new QIntValidator(176, 1920, this));
    xres_edit1->setValidator(new QIntValidator(176, 1920, this));
    yres_edit->setValidator(new QIntValidator(144, 1080, this));
    yres_edit1->setValidator(new QIntValidator(144, 1080, this));
    fps_edit->setValidator(new QDoubleValidator(1.0, 60.0, 2, this));
}

void Yuv_dlg::connect_signals()
{
    connect(start_btn, SIGNAL(clicked()), this, SLOT(start_playback()));
    connect(file_btn, SIGNAL(clicked()), this, SLOT(set_yuv_file_path()));
    connect(file_btn1, SIGNAL(clicked()), this, SLOT(set_yuv_file_path_1()));

    connect(res_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_change(int)));
    connect(res_cbox1, SIGNAL(currentIndexChanged(int)), this, SLOT(resolution_change_1(int)));
    connect(fmt_cbox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(format_change(const QString&)));
    connect(fmt_cbox1, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(format_change_1(const QString&)));

    connect(fps_edit, SIGNAL(textChanged(const QString&)), this, SLOT(fps_changed(const QString&)));
    connect(xres_edit, SIGNAL(textChanged(const QString&)), this, SLOT(width_changed(const QString&)));
    connect(xres_edit1, SIGNAL(textChanged(const QString&)), this, SLOT(width_changed_1(const QString&)));
    connect(yres_edit, SIGNAL(textChanged(const QString&)), this, SLOT(height_changed(const QString&)));
    connect(yres_edit1, SIGNAL(textChanged(const QString&)), this, SLOT(height_changed_1(const QString&)));
}

void Yuv_dlg::initialize_controls()
{
    fps_edit->setText("24.00");
    start_btn->setEnabled(false);
    path_edit->setEnabled(false);
    path_edit1->setEnabled(false);
    res_cbox->setCurrentIndex(Yuv_dlg::CIF);
    res_cbox1->setCurrentIndex(Yuv_dlg::CIF);
}

void Yuv_dlg::start_playback()
{
    QDialog::accept();
}

void Yuv_dlg::set_parameters(int view, Media::type fmt, float f, int w, int h, const char* p)
{
	fps = f;
	format[view] = fmt;
	width[view] = w;
	height[view] = h;
	path[view] = p;

	fps_edit->setText(QString("%1").arg(fps, 0, 'f', 2));
	if (0 == view)
	{
    	res_cbox->setCurrentIndex(Yuv_dlg::CUSTOM);
		update_res_text(xres_edit, yres_edit, Yuv_dlg::CUSTOM);
		xres_edit->setText(QString("%1").arg(w));
		yres_edit->setText(QString("%1").arg(h));
		path_edit->setText(path[0]);
	}
	else
	{
    	res_cbox1->setCurrentIndex(Yuv_dlg::CUSTOM);
		update_res_text(xres_edit1, yres_edit1, Yuv_dlg::CUSTOM);
		xres_edit1->setText(QString("%1").arg(w));
		yres_edit1->setText(QString("%1").arg(h));
		path_edit1->setText(path[0]);
	}
}

void Yuv_dlg::set_yuv_file_path()
{
    path[0] = QFileDialog::getOpenFileName(this, tr("Select Yuv File"),
                             path[0], tr("Yuv File (*.yuv)"));
    path_edit->setText(path[0]);
    validate_parameters();
}

void Yuv_dlg::set_yuv_file_path_1()
{
    path[1] = QFileDialog::getOpenFileName(this, tr("Select Yuv File"),
                             path[1], tr("Yuv File (*.yuv)"));
    path_edit1->setText(path[1]);
    validate_parameters();
}

void Yuv_dlg::update_res_text(QLineEdit* edit1, QLineEdit* edit2, int index)
{
    edit1->setEnabled(index == Yuv_dlg::CUSTOM);
    edit2->setEnabled(index == Yuv_dlg::CUSTOM);

    switch (index)
    {
        case Yuv_dlg::QCIF:
            edit1->setText("176");
            edit2->setText("144");
            break;
        case Yuv_dlg::CIF:
            edit1->setText("352");
            edit2->setText("288");
            break;
        case Yuv_dlg::SD:
            edit1->setText("640");
            edit2->setText("480");
            break;
        case Yuv_dlg::HD:
            edit1->setText("1280");
            edit2->setText("720");
            break;
        case Yuv_dlg::FHD:
            edit1->setText("1920");
            edit2->setText("1080");
            break;
        case Yuv_dlg::CUSTOM:
            edit1->setText("");
            edit2->setText("");
            break;
    }
}

void Yuv_dlg::resolution_change(int index)
{
	update_res_text(xres_edit, yres_edit, index);
}

void Yuv_dlg::resolution_change_1(int index)
{
	update_res_text(xres_edit1, yres_edit1, index);
}

void Yuv_dlg::update_format_change(int index, const QString& text)
{
    if (text == "YUV420")
    {
        format[index] = Media::I420;
    }
    else if (text == "YUV422")
    {
        format[index] = Media::I422;
    }
    else if (text == "YUV444")
    {
        format[index] = Media::I444;
    }
    else if (text == "YUY2")
    {
        format[index] = Media::YUY2;
    }
    else if (text == "UYVY")
    {
        format[index] = Media::UYVY;
    }
    else if (text == "YV12")
    {
        format[index] = Media::YV12;
    }
}

void Yuv_dlg::format_change(const QString& text)
{
	update_format_change(0, text);
}

void Yuv_dlg::format_change_1(const QString& text)
{
	update_format_change(1, text);
}

void Yuv_dlg::fps_changed(const QString& text)
{
    fps = text.toFloat();
    validate_parameters();
}

void Yuv_dlg::width_changed(const QString& text)
{
    width[0] = text.toInt();
    validate_parameters();
}

void Yuv_dlg::width_changed_1(const QString& text)
{
    width[1] = text.toInt();
    validate_parameters();
}

void Yuv_dlg::height_changed(const QString& text)
{
    height[0] = text.toInt();
    validate_parameters();
}

void Yuv_dlg::height_changed_1(const QString& text)
{
    height[1] = text.toInt();
    validate_parameters();
}

void Yuv_dlg::validate_parameters()
{
    bool ok = (width[0] >= 176) && (width[0] <= 1920);
    ok = ok && ((height[0] >= 144) && (height[0] <= 1080));
    ok = ok && (fps > 0.0f && fps <= 60.0f);
    ok = ok && QFile::exists(path[0]);
	if (left_frame->isVisible())
	{
		ok = ok && (width[1] >= 176) && (width[1] <= 1920);
    	ok = ok && ((height[1] >= 144) && (height[1] <= 1080));
    	ok = ok && (fps > 0.0f && fps <= 60.0f);
    	ok = ok && QFile::exists(path[1]);
	}
    start_btn->setEnabled(ok);
}

void Yuv_dlg::set_stereo_mode(bool status)
{
	if (status)
	{
		left_frame->show();
	}
	else
	{
		left_frame->hide();
	}
	setMinimumSize(400, status?230:120);
	setMaximumSize(400, status?230:120);
}

