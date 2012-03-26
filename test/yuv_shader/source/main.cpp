#include <QApplication>

#include "yuv_window.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	Yuv_window window;
	window.resize(600, 600);
	window.show();
	return app.exec();
}

