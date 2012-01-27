/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

template<class T>
class Thread
{
public:
    Thread();
    ~Thread();

public:
    int start(T* obj);
    void join();

protected:
    static void* thread_func(void* ptr);
    
private:
    Thread(const Thread& thread) {};
    Thread& operator=(const Thread& thread) { return *this; };

private:
    pthread_t _thread;
};

template<class T>
Thread<T>::Thread()
    :_thread(0)
{
}

template<class T>
Thread<T>::~Thread()
{
    join();
}

template<class T>
int Thread<T>::start(T* obj)
{
    return pthread_create(&_thread, 0, Thread::thread_func, obj);
}

template<class T>
void Thread<T>::join()
{
    if (_thread)
    {
        pthread_join(_thread, 0);
        _thread = 0;
    }
}

template<class T>
void* Thread<T>::thread_func(void* ptr)
{
    T* obj = reinterpret_cast<T *>(ptr);
    return (void*) obj->run();
}
    
#endif
