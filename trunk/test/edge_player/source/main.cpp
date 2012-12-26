#include <QApplication>
#include <gl_widget.h>
#include <X11/Xlib.h>

int main(int argc, char** argv)
{
    int w, h;
	XInitThreads();
	QApplication app(argc, argv);
    QGLFormat format;
    format.setVersion(3,1);
    format.setProfile(QGLFormat::CoreProfile);
    format.setSampleBuffers(true);
    w = atoi(argv[2]);
    h = atoi(argv[3]);
    Gl_widget window(w, h, QString(argv[1]), format);
	window.resize(w<<1, h<<1);
	window.show();
	return app.exec();
}

