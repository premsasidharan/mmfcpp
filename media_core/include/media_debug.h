#ifndef _MEDIA_DEBUG_H
#define _MEDIA_DEBUG_H

#include <media_tracer.h>

#define MEDIA_ERROR(fmt, ...) Media_logger::instance().log(Media_logger::console, fmt, __VA_ARGS__)
#define MEDIA_WARNING(fmt, ...) Media_logger::instance().log(Media_logger::console, fmt, __VA_ARGS__)

#if 0
#include <stdio.h>
#include <typeinfo>

#define OBJ_ERROR(fmt, ...) fprintf(stderr, "\n\tError: %s::%s() " fmt, typeid(this).name(), __func__, __VA_ARGS__)
#define OBJ_WARNING(fmt, ...) fprintf(stderr, "\n\tWarning: %s::%s() " fmt, typeid(this).name(), __func__, __VA_ARGS__)
#define _ERROR(fmt, ...) fprintf(stderr, "\n\t%s() " fmt, __func__, __VA_ARGS__)

#ifdef DEBUG_TRACE
#define TRACE(fmt, ...) fprintf(stderr, "%s() in %s, line %i: " fmt "\n", __func__, __FILE__, __LINE__, __VA_ARGS__)
#define OBJ_TRACE(fmt, ...) fprintf(stderr, "%s::%s() line #: %i " fmt "\n", typeid(this).name(), __func__, __LINE__, __VA_ARGS__)
#else
#define TRACE(fmt, ...) ((void)0)
#define OBJ_TRACE(fmt, ...) ((void)0)
#endif

#ifdef OBJECT_DEBUG
#define OBJ_DBG(fmt, ...) fprintf(stderr, "%s() in %s, line %i: " fmt "\n", __func__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define OBJ_DBG(fmt, ...) ((void)0)
#endif
#endif

#endif

