/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _MUTEX_H
#define _MUTEX_H

#include <time.h>
#include <errno.h>
#include <pthread.h>

class Condition_variable;

class Mutex
{
public:
    friend class Condition_variable;

    Mutex()
    {
        pthread_mutex_init(&_mutex, 0);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

public:
    int lock()
    {
        return pthread_mutex_lock(&_mutex);
    }
    int unlock()
    {
        return pthread_mutex_unlock(&_mutex);
    }
    int trylock()
    {
        return pthread_mutex_trylock(&_mutex);
    }

private:
    Mutex(const Mutex& _mutex) { (void)_mutex; };
    Mutex& operator=(const Mutex& _mutex) { (void)_mutex; return *this; };

private:
    pthread_mutex_t _mutex;
};

class Guard
{
public:
    Guard(Mutex& m):mutex(m) { mutex.lock(); };
    ~Guard() { mutex.unlock(); };
    
private:
    Mutex& mutex;
};

class Condition_variable
{
public:
    Condition_variable(Mutex* mutex = 0);
    ~Condition_variable();

public:
    int wait();
    int timed_wait(int msec);
    int timed_uwait(int timeout_us);

    int signal();
    int broadcast();

private:
    Condition_variable(const Condition_variable& _cv) { (void)_cv; };
    Condition_variable& operator=(const Condition_variable& _cv) { (void)_cv; return *this; };
    
    int timed_nwait(int timeout_ns);

private:
    Mutex* _mutex;
    int is_created;
    pthread_cond_t cond;
};

#endif
