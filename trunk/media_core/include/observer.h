#ifndef _OBSERVER_H
#define _OBSERVER_H

#include <media.h>
#include <media_params.h>

class Observer;

struct Observer_node
{
    Observer* observer;
    struct Observer_node* link;
};

class Abstract_media_object;

class Observer
{
public:
	virtual int event_handler(Media::events event, Abstract_media_object* obj, Media_params& params) = 0;
};

#endif
 
