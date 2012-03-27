#ifndef _YUV_WINDOW_H
#define _YUV_WINDOW_H

#include <QWidget>

#include "gl_widget.h"

class Yuv_window: public QWidget
{
	Q_OBJECT

public:
	Yuv_window(int width, int height, const QString& path, QWidget* parent = 0);
	~Yuv_window();

protected:
	void resizeEvent(QResizeEvent* event);

private:
	Gl_widget gl_widget;
};

#endif

