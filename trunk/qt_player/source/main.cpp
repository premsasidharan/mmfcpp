/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/
#include <QString>
#include <QtGui/QApplication>

#include <player.h>

int main(int argc, char** argv)
{
    MEDIA_TRACE();   
    QApplication app(argc, argv);

	if (argc < 2)
	{
		printf("\n\tInsufficien arguments\n");
		exit(0);
	}
	const QString str(argv[1]);
	Player player(str);
	player.start();
	return app.exec();
}

