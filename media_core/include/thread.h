#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

template<class T>
class Thread
{
public:
	Thread()
		:_thread(0)
	{
	}

	~Thread()
	{
		join();
	}

public:
	int start(T* obj)
	{
		return pthread_create(&_thread, 0, Thread::thread_func, obj);
	}

	void join()
	{
		if (_thread)
		{
			pthread_join(_thread, 0);
			_thread = 0;
		}
	}

protected:
	static void* thread_func(void* ptr)
	{
		T* obj = reinterpret_cast<T *>(ptr);
		return (void*) obj->run();
	};

private:
	pthread_t _thread;
};

#endif

