#include <QApplication>

#include <gl_widget.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
    int w = atoi(argv[2]);
    int h = atoi(argv[3]);
	Gl_widget widget(w, h, QString(argv[1]));
	widget.resize(w, h);
	widget.show();
	return app.exec();
}

