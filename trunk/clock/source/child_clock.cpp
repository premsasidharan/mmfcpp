/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <string.h>

#include <child_clock.h>
#include <master_clock.h>

Child_clock::Child_clock(const char* _name, Master_clock* clk)
    :Abstract_clock(_name)
    , master(clk)
{
}

Child_clock::~Child_clock()
{
}

Child_clock* Child_clock::create(const char* _name, Master_clock* clk)
{
    return new Child_clock(_name, clk);
}

void Child_clock::release(Child_clock* clk)
{
    delete clk;
}

int Child_clock::wait_for_sync(uint64_t pts)
{
    int64_t lag = 0;
    master->wait_and_update_start(this);
    master->get_deviation(pts, lag);        
    if (lag > 0)
    {
        cv.timed_uwait(lag);
    }
}
