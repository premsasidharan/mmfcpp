#ifndef _MEDIA_PARAMS
#define _MEDIA_PARAMS

#include <abstract_media_object.h>

struct Port;

union Media_params
{
    Port* port;
};

#endif
