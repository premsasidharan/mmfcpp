/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _BUFFER_H
#define _BUFFER_H

#include <mutex.h>

class Buffer_pool;
struct Buffer_node;

class Buffer
{
    friend class Buffer_pool;

private:
    Buffer(unsigned int _size, unsigned int _type, unsigned int _param_size);
    Buffer(unsigned int _size, unsigned int _type, unsigned int _param_size, void* _buffer, Buffer_pool* _pool);
    ~Buffer();

    //Prevent copy
    Buffer(const Buffer& buf) {}; 
    Buffer& operator=(const Buffer& buf) { return *this; };

public:
    unsigned int get_buffer_size() const { return size; };
    unsigned int get_parameter_size() const { return param_size; };
    unsigned int get_data_size() const { return data_size; };
    void set_data_size(unsigned int size) { data_size = size; };

    unsigned int type() const { return data_type; };
    void set_type(unsigned int _type) { data_type = _type; };

    unsigned int flags() const { return data_flag; };
    void set_flags(unsigned int _flag) { data_flag = _flag; };

    unsigned long long int pts() const { return data_pts; };
    void set_pts(unsigned long long int _pts) { data_pts = _pts; };

    void* data() { return (void*)((unsigned long long int)buffer+param_size); };
    void* parameter() { return buffer; };

    static Buffer* request(unsigned int size, unsigned int type, unsigned int _param_size); 
    static void release(Buffer* buffer);

private:
    unsigned int size;
    unsigned int data_size;
    unsigned int data_type;
    unsigned int data_flag;
    unsigned int param_size;
    unsigned long long int data_pts;

    void* buffer;

    int index;
    int is_used;
    Buffer* prev;
    Buffer* next;
    Buffer_pool* pool;
};

class Buffer_pool
{
    friend class Buffer;
public:
    Buffer_pool(unsigned int _pool_size, unsigned int buffer_size, unsigned int param_size, unsigned int type);
    ~Buffer_pool();

public:
    Buffer* request_buffer();

private:
    int wait_for_free_buffer();
    void release_buffer(Buffer* buffer);

public:
    void print_status();

private:
    void* memory;
    unsigned int pool_size;

    Buffer** buffer;
    Buffer* rear_free;
    Buffer* front_free;
    Buffer* rear_used;
    Buffer* front_used;

    Mutex mutex;
    Condition_variable cv;
};

#endif

