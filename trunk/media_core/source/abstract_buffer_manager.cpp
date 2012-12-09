/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>

#include <abstract_buffer_manager.h>

Buffer* Abstract_buffer_manager::create_buffer(unsigned int size, unsigned int type, unsigned int param_size, void* data)
{
    return new Buffer(size, type, param_size, data, this);
}

void Abstract_buffer_manager::delete_buffer(Buffer* buffer)
{
    delete buffer;
    buffer = 0;
}
