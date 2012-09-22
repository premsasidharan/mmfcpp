/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <stdio.h>
#include <string.h>

#include <child_clock.h>
#include <master_clock.h>

Master_clock::Master_clock(const char* _name)
    :Abstract_clock(_name)
    , correction(0)
    , state(Media::stop)
    , start_pts(0)
{
    memset(&start_time, 0, sizeof(struct timeval));
}

Master_clock::~Master_clock()
{
    mutex.lock();
    std::set<Child_clock*>::iterator itr = children.begin();
    for (; itr != children.end(); itr++)
    {
        Child_clock* child = *itr;
        Child_clock::release(child);
    }
    mutex.unlock();
}

int Master_clock::start(uint64_t start)
{ 
    struct timezone tz;
    mutex.lock();
    start_pts = start;
    gettimeofday(&start_time, &tz);
    reset_children_start();
    mutex.unlock();
    state = Media::play;
    //printf("\ntime: %llu, %u %u", start, (unsigned int)start_time.tv_sec, (unsigned int)start_time.tv_usec);
    fflush(stdout);
    return 1;
}

int Master_clock::stop(uint64_t& stop)
{
    int64_t time = 0;
    int ret = get_deviation(0, time);
    if (ret)
    {
        stop = (time < 0)?(-time):0;
        state = Media::stop;
    }
    return ret;
}

int Master_clock::get_deviation(uint64_t pts, int64_t& lag)
{
    struct timeval tv;
    struct timezone tz;
    mutex.lock();
    gettimeofday(&tv, &tz);
    int64_t time = (tv.tv_sec*1000000+tv.tv_usec)-(start_time.tv_sec*1000000+start_time.tv_usec);
    mutex.unlock();
    lag = (time >= 0)?((int64_t)pts-(int64_t)(start_pts+time)):0;
    //if (lag < 0)
    //printf("\ntime: %u %u time: %ld, pts: %ld, lag: %ld", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec, time, pts, lag);
    return (time >= 0);
}

Child_clock* Master_clock::create_child(const char* name)
{
    mutex.lock();
    Child_clock* child = Child_clock::create(name, this);
    children.insert(child);
    mutex.unlock();
    return child;
}

void Master_clock::release(Child_clock* clk)
{
    mutex.lock();
    children.erase(clk);
    Child_clock::release(clk);
    mutex.unlock();
}

void Master_clock::wait_and_update_start(Child_clock* child)
{
    if (0 == child || child->started)
    {
        return;
    }

    mutex.lock();
    child->started = true;
    bool all_started = true;
    std::set<Child_clock *>::iterator itr = children.begin();
    for (; itr != children.end(); itr++)
    {
        all_started &= (*itr)->started;
    }
    if (all_started)
    {
        struct timezone tz;
        gettimeofday(&start_time, &tz);
        mutex.unlock();
        cv.broadcast();
    }
    else
    {
        mutex.unlock();
        cv.wait();
    }
}

void Master_clock::reset_children_start()
{
    std::set<Child_clock *>::iterator itr = children.begin();
    for (; itr != children.end(); itr++)
    {
        (*itr)->started = false;
    }
}
