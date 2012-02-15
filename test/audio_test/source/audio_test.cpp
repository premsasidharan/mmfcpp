/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <audio_player.h>

int main(int argc, char** argv)
{
    int time = 0;
    MEDIA_TRACE();

    if (argc < 2)
    {
        printf("\n\tInsufficien arguments\n");
        exit(0);
    }
    
    Audio_player player;
    if (1 == player.set_file_path(argv[1]))
    {
        player.start();
        player.wait();
        player.stop();
    }
    else
    {
        printf("\nInvalid Wave File Path");
    }
    return 0;
}
