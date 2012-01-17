/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _MEDIA_PARAMS
#define _MEDIA_PARAMS

#include <abstract_media_object.h>

struct Port;

union Media_params
{
    Port* port;
};

#endif
