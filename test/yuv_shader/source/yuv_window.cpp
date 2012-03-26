#include "yuv_window.h"

Yuv_window::Yuv_window(QWidget* parent)
	:QWidget(parent)
	, gl_widget(this)
{
}

Yuv_window::~Yuv_window()
{
}

void Yuv_window::resizeEvent(QResizeEvent* event)
{
	gl_widget.setGeometry(0, 0, width(), height());
}

