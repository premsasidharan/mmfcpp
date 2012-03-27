#include "yuv_window.h"

Yuv_window::Yuv_window(int width, int height, const QString& path, QWidget* parent)
	:QWidget(parent)
	, gl_widget(width, height, path, this)
{
}

Yuv_window::~Yuv_window()
{
}

void Yuv_window::resizeEvent(QResizeEvent* event)
{
	gl_widget.setGeometry(0, 0, width(), height());
}

