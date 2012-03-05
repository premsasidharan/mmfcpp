/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#ifndef _BUFFER_H
#define _BUFFER_H

#include <set>
#include <mutex.h>

class Buffer
{
private:
    Buffer(unsigned int _size, unsigned int _type, unsigned int _param_size);
    Buffer(unsigned int _start, unsigned int _size, unsigned int _type, unsigned int _param_size, Buffer* buff);
    ~Buffer();

    //Prevent copy
    Buffer(const Buffer& buf) { (void)buf; }; 
    Buffer& operator=(const Buffer& buf) { (void)buf; return *this; };
    
    int reference_count() const;
    static void release(Buffer*parent, Buffer* child);

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

    void* data() { return buffer; };
    void* parameter() { return param; };
    
    void print(int level);
    
    Buffer* split(unsigned int start, unsigned int size, unsigned int type, unsigned int param_size);
    
    Buffer* clone();

    static Buffer* request(unsigned int size, unsigned int type, unsigned int _param_size); 
    static void release(Buffer* buffer);

private:
    unsigned int size;
    unsigned int data_size;
    unsigned int data_type;
    unsigned int data_flag;
    unsigned int param_size;
    unsigned long long int data_pts;

    void* param;
    void* buffer;
    
    int ref_count;
    mutable Mutex mutex;

    Buffer* parent;
    std::set<Buffer*> children;
};

#endif

