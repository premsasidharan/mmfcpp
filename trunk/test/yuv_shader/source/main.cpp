#include <QApplication>

#include "yuv_window.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	Yuv_window window(atoi(argv[2]), atoi(argv[3]), QString(argv[1]));
	window.resize(600, 600);
	window.show();
	return app.exec();
}

