#include <tone_dlg.h>
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Tone_dlg dlg;
    dlg.exec();

    return 0;
}

