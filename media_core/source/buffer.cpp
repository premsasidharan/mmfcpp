/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdlib.h>

#include <buffer.h>
#include <buffer_private.h>

#include <media_debug.h>

Buffer::Buffer(unsigned int _size, unsigned int _type, unsigned int _param_size)
    :size(_size)
    , data_size(0)
    , data_type(_type)
    , data_flag(0)
    , param_size(_param_size)
    , param(0)
    , buffer(0)
    , ref_count(1)
    , parent(0)
{
    buffer = malloc(size);
    param = malloc(param_size);
}

Buffer::Buffer(unsigned int start, unsigned int _size, unsigned int _type, unsigned int _param_size, Buffer* buff)
    :size(_size)
    , data_size(0)
    , data_type(_type)
    , data_flag(0)
    , param_size(_param_size)
    , param(0)
    , buffer(0)
    , ref_count(1)
    , parent(buff)
{
    param = malloc(param_size);
    buffer = buff->buffer+start;
}

Buffer::~Buffer()
{
    if (0 == parent)
    {
        free(buffer);
        buffer = 0;
    }
    free(param);
    param = 0;
}

int Buffer::reference_count() const
{
    int count;
    mutex.lock();
    count = ref_count+children.size();
    mutex.unlock();
    return count;
}

Buffer* Buffer::split(unsigned int _start, unsigned int _size, unsigned int _type, unsigned int _param_size)
{
    Buffer* buff = 0;
    if ((_start+_size) > size)
    {
        return 0;
    }
    mutex.lock();
    if ((ref_count+children.size()) == 0)
    {
        mutex.unlock();
        return 0;
    }
    buff = new Buffer(_start, _size, _type, _param_size, this);
    children.insert(buff);
    mutex.unlock();
    return buff;
}

Buffer* Buffer::request(unsigned int _size, unsigned int _type, unsigned int _param_size)
{
    Buffer* buffer = new Buffer(_size, _type, _param_size);
    return buffer;
}

void Buffer::release(Buffer* buffer)
{
    buffer->mutex.lock();
    if (buffer->ref_count < 1)
    {
        MEDIA_ERROR("Error Buffer::release. Invalid reference count: %d\n", buffer->ref_count);
        buffer->mutex.unlock();
        return;
    }
    --buffer->ref_count;
    if (0 == (buffer->ref_count+buffer->children.size()))
    {
        if (0 != buffer->parent)
        {
            Buffer::release(buffer->parent, buffer);
        }
        buffer->mutex.unlock();
        delete buffer;
        buffer = 0;
    }
    else
    {
        buffer->mutex.unlock();
    }
}

void Buffer::release(Buffer*parent, Buffer* child)
{
    parent->mutex.lock();
    parent->children.erase(child);
    if (0 == (parent->ref_count+parent->children.size()))
    {
        if (0 != parent->parent)
        {
            Buffer::release(parent->parent, parent);
        }
        parent->mutex.unlock();
        delete parent;
        parent = 0;
    }
    else
    {
        parent->mutex.unlock();
    }
}

void Buffer::print(int level)
{
    printf("\nBuffer, level: %d, data: 0x%llx, param: 0x%llx, ref_count: %d"
           ", children: %d, this: 0x%llx, parent: 0x%llx", 
        level, (unsigned long long)buffer, (unsigned long long)param, 
        reference_count(), (int)children.size(), (unsigned long long)this, 
        (unsigned long long)parent);
    for (std::set<Buffer*>::iterator itr = children.begin();
        itr != children.end(); itr++)
    {
        (*itr)->print(level+1);
    }
}
