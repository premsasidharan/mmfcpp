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
    , param_size(_param_size)
    , buffer(0)
    , index(0)
    , pool(0)
{
    buffer = malloc(size+param_size);
}

Buffer::Buffer(unsigned int _size, unsigned int _type, unsigned int _param_size, void* _buffer, Buffer_pool* _pool)
    :size(_size)
    , data_size(0)
    , data_type(_type)
    , param_size(_param_size)
    , buffer(_buffer)
    , pool(_pool)
{
}

Buffer::~Buffer()
{
    if (0 == pool)
    {
        free(buffer);
        buffer = 0;
    }
}

Buffer* Buffer::request(unsigned int _size, unsigned int _type, unsigned int _param_size)
{
    Buffer* buffer = new Buffer(_size, _type, _param_size);
    return buffer;
}

void Buffer::release(Buffer* buffer)
{
    Buffer_pool* pool = buffer->pool;
    if (0 == pool)
    {
        delete buffer;
    }
    else
    {
        pool->release_buffer(buffer);
    }
}

Buffer_pool::Buffer_pool(unsigned int _pool_size, unsigned int buffer_size, unsigned int param_size, unsigned int type)
    :memory(0)
    , pool_size(_pool_size)
    , buffer(0)
    , rear_free(0)
    , front_free(0)
    , rear_used(0)
    , front_used(0)
{
    buffer = new Buffer*[pool_size];
    memory = (void *) malloc(pool_size*(buffer_size+param_size));
    for (int i = 0; i < (int)pool_size; i++)
    {
        buffer[i] = new Buffer(buffer_size, type, param_size, (void*)((unsigned long long)memory+(i*(buffer_size+param_size))), this);
        buffer[i]->index = i;
        buffer[i]->is_used = 0;
        if (i >= 1)
        {
            buffer[i-1]->next = buffer[i];
            buffer[i]->prev = buffer[i-1];
        }
    }
    front_free = buffer[0];
    rear_free = buffer[pool_size-1];
    front_used = rear_used = 0;
}

Buffer_pool::~Buffer_pool()
{
    free(memory);
    memory = 0;

    for (int i = 0; i < (int)pool_size; i++)
    {
        delete buffer[i];
        buffer[i] = 0;
    }

    delete [] buffer;
    buffer = 0;
}

Buffer* Buffer_pool::request_buffer()
{
    if (1 == wait_for_free_buffer())
    {
        mutex.lock();
        Buffer* node = 0;
        if (front_free == rear_free)
        {
            node = front_free;
            front_free = rear_free = 0;
        }
        else
        {
            node = front_free;
            front_free = front_free->next;
            node->next = 0;
            front_free->prev = 0;
        }
        if (rear_used == 0)
        {
            rear_used = front_used = node;
        }
        else
        {
            rear_used->next = node;
            node->prev = rear_used;
            rear_used = node;
        }
        mutex.unlock();
        node->is_used = 1;
        return node;
    }
    return 0;
}

int Buffer_pool::wait_for_free_buffer()
{
    int status = 0;
    do
    {
        mutex.lock();
        status = (front_free == 0);
        mutex.unlock();
        if (status)
        {
            cv.wait();
        }
        else
        {
            break;
        }
    }
    while (1);

    return 1;
}

void Buffer_pool::release_buffer(Buffer* _buffer)
{
    int flag = 0;
    mutex.lock();
    if (buffer[_buffer->index] == _buffer)
    {
        if (front_used == _buffer)
        {
            front_used = _buffer->next;
            if (0 == front_used)
            {
                rear_used = 0;
            }
            else
            {
                front_used->prev = 0;
                _buffer->next = 0;
            }
        }
        else if (rear_used == _buffer)
        {
            rear_used = _buffer->prev;
            _buffer->prev = 0;
        }
        else
        {
            (_buffer->prev)->next = _buffer->next;
            (_buffer->next)->prev = _buffer->prev;
            _buffer->next = _buffer->prev = 0;
        }
        _buffer->is_used = 0;
        if (rear_free == 0)
        {
            rear_free = front_free = _buffer;
            flag = 1;
        }
        else
        {
            rear_free->next = _buffer;
            _buffer->prev = rear_free;
            rear_free = _buffer;
        }
    }
    mutex.unlock();
    if (flag)
    {
        cv.signal();
    }
}

void Buffer_pool::print_status()
{
    printf("\n\tPool: memory: %lx, buffer_size: %d, pool_size: %d, buffer: %lx, rear_free: %lx, front_free: %lx, rear_used: %lx, front_used: %lx",
           (unsigned long)memory, buffer[0]->size, pool_size, (unsigned long)buffer, (unsigned long)rear_free,
           (unsigned long)front_free, (unsigned long)rear_used, (unsigned long)front_used);
    for (int i = 0; i < (int)pool_size; i++)
    {
        printf("\n\tIndex: %d, is_used: %d, data_size: %d, buffer: %lx, current: %lx, prev: %lx, next: %lx, pool: %lx",
               buffer[i]->index, buffer[i]->is_used, buffer[i]->data_size, (unsigned long)buffer[i]->buffer, (unsigned long)buffer[i],
               (unsigned long)buffer[i]->prev, (unsigned long)buffer[i]->next, (unsigned long)buffer[i]->pool);
    }
    printf("\n");
}
