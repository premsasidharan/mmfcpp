/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _MEDIA_DEBUG_H
#define _MEDIA_DEBUG_H

#include <media_tracer.h>

#define MEDIA_ERROR(fmt, ...) Media_logger::instance().log(Media_logger::console, fmt, __VA_ARGS__)
#define MEDIA_WARNING(fmt, ...) Media_logger::instance().log(Media_logger::console, fmt, __VA_ARGS__)

#endif

