/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _MASTER_CLOCK_H_
#define _MASTER_CLOCK_H_

#include <set>
#include <stdint.h>
#include <sys/time.h>

#include <media.h>
#include <mutex.h>
#include <abstract_clock.h>

class Child_clock;

class Master_clock:public Abstract_clock
{
public:
    friend class Child_clock;
    
    Master_clock(const char* _name);
    ~Master_clock();

public:
    int start(uint64_t start);
    int stop(uint64_t& stop);
    
    int get_deviation(uint64_t pts, int64_t& lag);
    
    Child_clock* create_child(const char* name);
    
    void release(Child_clock* child);
    
protected:
    void reset_children_start();
    void wait_and_update_start(Child_clock* child);
    
private:
    int correction;
    Media::state state;
    uint64_t start_pts;
    struct timeval start_time;
    
    Mutex mutex;
    Condition_variable cv;
    std::set<Child_clock*> children;
};

#endif
