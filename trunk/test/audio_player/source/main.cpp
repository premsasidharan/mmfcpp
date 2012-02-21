/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <player_window.h>

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Player_window window;
    window.show();

    return app.exec();
}
