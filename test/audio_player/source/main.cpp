/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <QApplication>
#include <audio_player.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Audio_player player;
    player.show();
    return app.exec();
}
