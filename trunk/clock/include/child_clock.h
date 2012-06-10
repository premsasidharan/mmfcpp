/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _CHILD_CLOCK_H_
#define _CHILD_CLOCK_H_

#include <stdint.h>
#include <master_clock.h>
#include <abstract_clock.h>

class Child_clock:public Abstract_clock
{
public:
    friend class Master_clock;

    int wait_for_sync(uint64_t pts);
    
protected:
    static Child_clock* create(const char* _name, Master_clock* clk);
    static void release(Child_clock* clk);
    
private:
    Child_clock(const char* _name, Master_clock* clk);
    ~Child_clock();
    
private:
    bool started;
    Master_clock* master;
    Condition_variable cv;
};

#endif
