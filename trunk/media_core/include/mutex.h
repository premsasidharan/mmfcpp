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
    Mutex(const Mutex& _mutex) {};
    Mutex& operator=(const Mutex& _mutex)
    {
        return *this;
    };

private:
    pthread_mutex_t _mutex;
};

class Condition_variable
{
public:
    Condition_variable(Mutex* mutex = 0)
        :_mutex(mutex)
        , is_created(mutex == 0)
    {
        if (is_created)
        {
            _mutex = new Mutex;
        }
        pthread_cond_init(&cond, 0);
    }
    ~Condition_variable()
    {
        pthread_cond_destroy(&cond);
        if (is_created)
        {
            delete _mutex;
        }
        _mutex = 0;
    }

public:
    int wait()
    {
        int ret;
        _mutex->lock();
        ret = pthread_cond_wait(&cond, &(_mutex->_mutex));
        _mutex->unlock();
        return ret;
    }

    int timed_wait(int timeout_ms)
    {
        int ret = EINVAL;
        struct timespec ts;
        _mutex->lock();
        if (0 == clock_gettime(CLOCK_REALTIME, &ts))
        {
            ts.tv_sec += (timeout_ms/1000);
            ts.tv_nsec += ((timeout_ms%1000)*1000000);
            ret = pthread_cond_timedwait(&cond, &(_mutex->_mutex), &ts);
        }
        _mutex->unlock();
        return ret;
    }

    int signal()
    {
        int ret;
        _mutex->lock();
        ret = pthread_cond_signal(&cond);
        _mutex->unlock();
        return ret;
    }

    int broadcast()
    {
        int ret;
        _mutex->lock();
        ret = pthread_cond_broadcast(&cond);
        _mutex->unlock();
        return ret;
    }

private:
    Condition_variable(const Condition_variable& _cv) {};
    Condition_variable& operator=(const Condition_variable& _cv)
    {
        return *this;
    };

private:
    Mutex* _mutex;
    int is_created;
    pthread_cond_t cond;
};

#endif
