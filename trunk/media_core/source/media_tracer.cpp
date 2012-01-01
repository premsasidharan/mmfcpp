/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * published by the Free Software Foundation.
*/

#include <media_tracer.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

Media_tracer::Media_tracer(const char* _func_name, const char* _class_name)
		:func_name(_func_name)
		, class_name(_class_name) 
{
	sprintf(arg_string, "%s", "");
    
	MEDIA_LOG("Entering %s%s%s", (class_name==0)?"":class_name, (class_name == 0)?"":"::", func_name);
	
	memset(&end, 0, sizeof(struct timeval));
	memset(&start, 0, sizeof(struct timeval));

	gettimeofday(&start, 0);
}

Media_tracer::Media_tracer(const char* _func_name, const char* _class_name, const char *fmt = 0, ...)
		:func_name(_func_name)
		, class_name(_class_name)
{	
	va_list argp;
	va_start(argp, fmt);
	vsnprintf(arg_string, MAX_ARG_STRING_LENGTH, fmt, argp);

	MEDIA_LOG("Entering %s%s%s : %s", (class_name==0)?"":class_name, (class_name==0)?"":"::", func_name, arg_string);

	memset(&end, 0, sizeof(struct timeval));
	memset(&start, 0, sizeof(struct timeval));

	gettimeofday(&start, 0);
}

Media_tracer::~Media_tracer()
{
    double duration = 0.0;
	gettimeofday(&end, 0);
	duration = elapsed_time();
	if (0 == strcmp(arg_string, ""))
	{
		MEDIA_LOG("Leaving %s%s%s, Time spent: %lf milli seconds", (class_name == 0)?"":class_name, (class_name == 0)?"":"::", func_name, 1000.0*duration);
	}
	else
	{
		MEDIA_LOG("Leaving %s%s%s : %s, Time spent: %lf milli seconds", (class_name == 0)?"":class_name, (class_name == 0)?"":"::", func_name, arg_string, 1000.0*duration);
	}
}

double Media_tracer::elapsed_time()
{
	double end_time = ((double)end.tv_sec)+((double)end.tv_usec/1000000.0);
	double start_time = ((double)start.tv_sec)+((double)start.tv_usec/1000000.0);
	return (end_time-start_time);
}
