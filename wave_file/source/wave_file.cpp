/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <string.h>
#include <wave_file.h>

Wave_file::Wave_file()
    :file(0)
{
    memset(&hdr, 0, sizeof(Wave_hdr));
}

Wave_file::~Wave_file()
{
}

void Wave_file::close_file()
{
    if (0 != file)
    {
        fclose(file); file = 0;
        memset(&hdr, 0, sizeof(Wave_hdr));
    }
}

int Wave_file::is_eof()
{
    if (0 == file)
    {
        return 0;
    }
    return (0 != feof(file));
}

int Wave_file::frames_count() const
{
    int size = frame_size();
    if (0 == size)
    {
        return 0;
    }
    return (hdr.wave.data_size/size); 
}
