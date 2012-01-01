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

