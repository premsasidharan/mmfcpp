#include <player_window.h>

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Player_window window;
    window.show();

    return app.exec();
}
