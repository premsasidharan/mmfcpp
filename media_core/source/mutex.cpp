#include <mutex.h>

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
