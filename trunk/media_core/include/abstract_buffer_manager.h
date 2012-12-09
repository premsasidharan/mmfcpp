/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _ABSTRACT_BUFFER_MANAGER_H_
#define _ABSTRACT_BUFFER_MANAGER_H_

#include <set>
#include <list>

#include <mutex.h>
#include <buffer.h>

class Abstract_buffer_manager
{
    friend class Buffer;
public:
    Abstract_buffer_manager() {} ;
    virtual ~Abstract_buffer_manager() {};
    
    virtual Buffer* request(unsigned int size, unsigned int type, unsigned int param_size) = 0;

protected:   
    virtual void free(Buffer* buff) = 0;

    virtual void map(Buffer* buffer) {};
    virtual void* data(Buffer* buffer) = 0;
    virtual void unmap(Buffer* buffer) {};
    
    Buffer* create_buffer(unsigned int size, unsigned int type, unsigned int param_size, void* data);
    void delete_buffer(Buffer* buffer);
};

#endif
