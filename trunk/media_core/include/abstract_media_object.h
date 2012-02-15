/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _ABSTRACT_MEDIA_OBJECT_H
#define _ABSTRACT_MEDIA_OBJECT_H

#include <media.h>
#include <mutex.h>
#include <buffer.h>
#include <observer.h>

#define MAX_PORTS 4
#define OBJ_NAME_MAX_CHARS 25

struct _Port;

struct Port
{
    unsigned int type;
    //unsigned int port_num;
    char port_name[OBJ_NAME_MAX_CHARS];
};

class Abstract_media_object
{
public:
    Abstract_media_object(const char* _name);
    virtual ~Abstract_media_object();

public:
    const char* object_name() const { return obj_name; };
    int input_port_count() const { return input_count; };
    int output_port_count() const { return output_count; };

    int input_port_type(int port);
    int output_port_type(int port);

    virtual Media::status attach(Media::events event, Observer* obs);
    virtual Media::status detach(Media::events event, Observer* obs);

    friend Media::status start(Abstract_media_object* src, int start_time);
    friend Media::status start(Abstract_media_object& src, int start_time);
    friend Media::status stop(Abstract_media_object* src, int& end_time);
    friend Media::status stop(Abstract_media_object& src, int& end_time);
    friend Media::status pause(Abstract_media_object* src, int& end_time);
    friend Media::status pause(Abstract_media_object& src, int& end_time);

    friend Media::status connect(Abstract_media_object* src, Abstract_media_object* dest);
    friend Media::status connect(Abstract_media_object& src, Abstract_media_object& dest);
    friend Media::status disconnect(Abstract_media_object* src, Abstract_media_object* dest);
    friend Media::status disconnect(Abstract_media_object& src, Abstract_media_object& dest);

protected:
    virtual Media::status on_start(int start_time) = 0;
    virtual Media::status on_stop(int end_time) = 0;
    virtual Media::status on_pause(int end_time) = 0;

    virtual Media::status on_connect(int port, Abstract_media_object* pobj) = 0;
    virtual Media::status on_disconnect(int port, Abstract_media_object* pobj) = 0;

    virtual Media::status input_data(int port, Buffer* buffer) { (void)port; (void)buffer; return Media::not_implemented; };

    virtual Media::status notify(Media::events event/*, Media_params& params*/);

protected:
    Media::state get_state();
    void set_state(Media::state _state);

    Media::status push_data(int port, Buffer* buffer);

    Media::status create_input_port(const Port* port);
    Media::status create_output_port(const Port* port);

    Media::status create_input_ports(const Port* ports, int port_count);
    Media::status create_output_ports(const Port* ports, int port_count);

private:
    Abstract_media_object(const Abstract_media_object& obj) { (void)obj; };
    Abstract_media_object& operator=(const Abstract_media_object& obj) { (void)obj; return *this; };

    Media::status private_start(int start_time);
    Media::status private_stop(int& end_time);
    Media::status private_pause(int& end_time);

private:
    char *obj_name;
    int input_count;
    int output_count;
    Mutex state_lock;
    Media::state state;
    Mutex obs_hash_lock;
    _Port* input[MAX_PORTS];
    _Port* output[MAX_PORTS];
    Observer_node* obs_hash[Media::last_event];
};

Media::status start(Abstract_media_object* src, int start_time);
Media::status start(Abstract_media_object& src, int start_time);
Media::status stop(Abstract_media_object* src, int& end_time);
Media::status stop(Abstract_media_object& src, int& end_time);
Media::status pause(Abstract_media_object* src, int& end_time);
Media::status pause(Abstract_media_object& src, int& end_time);
Media::status connect(Abstract_media_object* src, Abstract_media_object* dest);
Media::status connect(Abstract_media_object& src, Abstract_media_object& dest);
Media::status disconnect(Abstract_media_object* src, Abstract_media_object* dest);
Media::status disconnect(Abstract_media_object& src, Abstract_media_object& dest);

#endif
