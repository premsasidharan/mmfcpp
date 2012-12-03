#include <QApplication>

//#include "yuv_window.h"
#include "gl_widget.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
    QGLFormat format;
    format.setVersion(3,1);  // get expected output with (3,1) and below, else blank window
    format.setProfile(QGLFormat::CoreProfile);
    format.setSampleBuffers(true);
    Gl_widget window(atoi(argv[2]), atoi(argv[3]), QString(argv[1]), format);
	window.resize(800, 600);
	window.show();
	return app.exec();
}

