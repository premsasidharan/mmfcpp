/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/
#include <QtGui/QApplication>
#include "yuv_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Yuv_window w(0, 0, 640, 480);
    w.show();

    return app.exec();
}
