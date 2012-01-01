/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _BUFFER_PRIVATE_H
#define _BUFFER_PRIVATE_H

#include <buffer.h>

struct Buffer_node
{
	int index;
	Buffer_node* prev;
	Buffer_node* next;
};

#endif

