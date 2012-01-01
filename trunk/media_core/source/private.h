/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <abstract_media_object.h>

struct _Port
{
	Port port;
	int port_index;
	Abstract_media_object* object;
};

