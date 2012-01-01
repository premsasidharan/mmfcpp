#ifndef _MEDIA_TRACER_H
#define _MEDIA_TRACER_H

#include <typeinfo>
#include <sys/time.h>

#include <media_logger.h>

#ifndef MAX_ARG_STRING_LENGTH
#define MAX_ARG_STRING_LENGTH 1024
#endif

class Media_tracer
{
public:
	Media_tracer(const char* _func_name, const char* _class_name);
    Media_tracer(const char* _func_name, const char* _class_name, const char *fmt, ...);
	~Media_tracer();

private:
	double elapsed_time();

private:
	const char* func_name;
	const char* class_name;

	struct timeval end;
	struct timeval start;
    
	char arg_string[MAX_ARG_STRING_LENGTH];
};

#if defined(MEDIA_TRACE_CONSOLE) || defined(MEDIA_TRACE_FILE)

#define MEDIA_TRACE() Media_tracer _trace_obj(__func__, 0);
#define MEDIA_TRACE_OBJ() Media_tracer _trace_obj(__func__, (typeid(this).name()));
#define MEDIA_TRACE_PARAM(fmt, ...) Media_tracer _trace_obj(__func__, 0, fmt, __VA_ARGS__);
#define MEDIA_TRACE_OBJ_PARAM(fmt, ...) Media_tracer _trace_obj(__func__, (typeid(this).name()), fmt, __VA_ARGS__);
#else
#define MEDIA_TRACE() ((void)0)
#define MEDIA_TRACE_OBJ() ((void)0)
#define MEDIA_TRACE_PARAM(fmt, ...) ((void)0)
#define MEDIA_TRACE_OBJ_PARAM(fmt, ...) ((void)0)
#endif

#endif

