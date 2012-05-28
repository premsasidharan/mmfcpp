#include <mutex.h>

#include <stdio.h>

Condition_variable::Condition_variable(Mutex* mutex)
    :_mutex(mutex)
    , is_created(mutex == 0)
{
    if (is_created)
    {
        _mutex = new Mutex;
    }
    pthread_cond_init(&cond, 0);
}

Condition_variable::~Condition_variable()
{
    pthread_cond_destroy(&cond);
    if (is_created)
    {
        delete _mutex;
    }
    _mutex = 0;
}

int Condition_variable::wait()
{
    int ret;
    _mutex->lock();
    ret = pthread_cond_wait(&cond, &(_mutex->_mutex));
    _mutex->unlock();
    return ret;
}

int Condition_variable::timed_wait(int timeout_ms)
{
    return timed_nwait(timeout_ms*1000000);
}

int Condition_variable::timed_uwait(int timeout_us)
{
    return timed_nwait(timeout_us*1000);
}

int Condition_variable::timed_nwait(int timeout_ns)
{
    int ret = EINVAL;
    struct timespec ts;
    _mutex->lock();
    if (0 == clock_gettime(CLOCK_REALTIME, &ts))
    {
        int temp = (timeout_ns+ts.tv_nsec);
        ts.tv_sec += (temp/1000000000); 
        ts.tv_nsec = (temp%1000000000);
        ret = pthread_cond_timedwait(&cond, &(_mutex->_mutex), &ts);
    }
    _mutex->unlock();
    return ret;
    
}

int Condition_variable::signal()
{
    int ret;
    _mutex->lock();
    ret = pthread_cond_signal(&cond);
    _mutex->unlock();
    return ret;
}

int Condition_variable::broadcast()
{
    int ret;
    _mutex->lock();
    ret = pthread_cond_broadcast(&cond);
    _mutex->unlock();
    return ret;
}
