#include <QApplication>
#include <gl_widget.h>
#include <X11/Xlib.h>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("\nUsage\n%s [yuv420 file] [width] [height]", argv[0]);
        printf("\nedge_player Videos/flower_cif.yuv 352 288\n");
        exit(0);
    }
    int w, h;
    w = atoi(argv[2]);
    h = atoi(argv[3]);
	XInitThreads();
	QApplication app(argc, argv);
    QGLFormat format;
    format.setVersion(3,1);
    format.setProfile(QGLFormat::CoreProfile);
    format.setSampleBuffers(true);
    Gl_widget window(w, h, QString(argv[1]), format);
	window.resize(w<<1, h<<1);
	window.show();
	return app.exec();
}

