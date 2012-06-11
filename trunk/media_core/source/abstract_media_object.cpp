/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <string.h>

#include <private.h>
#include <media_debug.h>
#include <abstract_media_object.h>

Abstract_media_object::Abstract_media_object(const char* _name)
    :input_count(0)
    , output_count(0)
    , state(Media::init)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
    obj_name = (char *)malloc(1+strlen(_name));
    strcpy(obj_name, _name);
    for (int i = 0; i < MAX_PORTS; i++)
    {
        input[i] = 0;
        output[i] = 0;
    }
}

Abstract_media_object::~Abstract_media_object()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    free(obj_name);
    obj_name = 0;

    for (int i = 0; i < MAX_PORTS; i++)
    {
        if (0 != input[i])
        {
            delete input[i];
            input[i] = 0;
        }
        if (0 != output[i])
        {
            delete output[i];
            output[i] = 0;
        }
    }
}

int Abstract_media_object::input_port_type(int port)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (port < input_count)
    {
        return input[port]->port.type;
    }
    MEDIA_ERROR("Invalid Input Port: %s, %d", object_name(), port);
    return Media::invalid_port;
}

int Abstract_media_object::output_port_type(int port)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (port < output_count)
    {
        return output[port]->port.type;
    }
    MEDIA_ERROR("Invalid Output Port: %s, %d", object_name(), port);
    return Media::invalid_port;
}

Media::status Abstract_media_object::create_input_port(const Port* port)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::status status = Media::max_ports_reached;
    if (input_count < MAX_PORTS)
    {
        Media_params params;
        input[input_count] = new _Port;
        memcpy(&(input[input_count]->port), port, sizeof(Port));
        input[input_count]->object = 0;
        params.port = &(input[input_count]->port);
        input_count++;
        notify(Media::input_port_created, params);
        status = Media::ok;
    }
    return status;
}

Media::status Abstract_media_object::create_output_port(const Port* port)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::status status = Media::max_ports_reached;
    if (output_count < MAX_PORTS)
    {
        Media_params params;
        output[output_count] = new _Port;
        memcpy(&(output[output_count]->port), port, sizeof(Port));
        output[output_count]->object = 0;
        params.port = &(output[output_count]->port);
        output_count++;
        notify(Media::output_port_created, params);
        status = Media::ok;
    }
    return status;
}

Media::status Abstract_media_object::create_input_ports(const Port* ports, int port_count)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::status status = Media::max_ports_reached;
    if (port_count < MAX_PORTS)
    {
        for (int i = 0; i < port_count; i++)
        {
            input[i] = new _Port;
            memcpy(&(input[i]->port), &ports[i], sizeof(Port));
            input[i]->object = 0;
        }
        input_count = port_count;
        status = Media::ok;
    }
    return status;
}

Media::status Abstract_media_object::create_output_ports(const Port* ports, int port_count)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::status status = Media::max_ports_reached;
    if (port_count < MAX_PORTS)
    {
        for (int i = 0; i < port_count; i++)
        {
            output[i] = new _Port;
            memcpy(&(output[i]->port), &ports[i], sizeof(Port));
            output[i]->object = 0;
        }
        output_count = port_count;
        status = Media::ok;
    }
    return status;
}

Media::state Abstract_media_object::get_state()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::state _state;
    state_lock.lock();
    _state = state;
    state_lock.unlock();
    return _state;
}

void Abstract_media_object::set_state(Media::state _state)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    state_lock.lock();
    state = _state;
    state_lock.unlock();
}

Media::status Abstract_media_object::push_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s Port: %d", object_name(), port);
    if (0 == buffer)
    {
        MEDIA_ERROR("Invalid Buffer: %s, Port: %d, Buffer: 0x%llx", object_name(), port, (unsigned long long)buffer);
        return Media::invalid_buffer;
    }
    if (port >= output_count)
    {
        MEDIA_ERROR("Invalid Port: %s, Port: %d, Buffer: 0x%llx, pts: %llu", object_name(), port, (unsigned long long)buffer, buffer->pts());
        return Media::invalid_port;
    }
    if (output[port]->object == 0)
    {
        MEDIA_ERROR("Port Not Conected: %s, Port: %d, Buffer: 0x%llx, pts: %llu", object_name(), port, (unsigned long long)buffer, buffer->pts());
        return Media::port_not_connected;
    }
    if (buffer->type() != (output[port]->port.type & buffer->type()))
    {
        MEDIA_ERROR("Type Mismatch: %s, Port: %d, Buffer: 0x%llx, pts: %llu", object_name(), port, (unsigned long long)buffer, buffer->pts());
        return Media::invalid_type;
    }

    Media::status ret = Media::ok;
    Abstract_media_object* object = output[port]->object;
    if (output[port]->port_index >= object->input_port_count())
    {
        MEDIA_ERROR("Invalid connection: object: %s, port: %d connectiong to object: %s, port: %d",
                    object_name(), port, object->object_name(), output[port]->port_index);
        return Media::invalid_type;
    }

    ret = object->input_data(output[port]->port_index, buffer);
    return ret;
}

Media::status Abstract_media_object::private_start(int start_time, int end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s, Start time: %d, End time: %d", object_name(), start_time, end_time);
    /*if (Media::play == get_state())
    {
        MEDIA_WARNING("Already in Play State: %s, Start time: %d, End time: %d", object_name(), start_time, end_time);
        return Media::ok;
    }*/

    Media::status status = Media::ok;
    set_state(Media::play);
    for (int i = 0; i < output_count; i++)
    {
        if (0 != output[i]->object)
        {
            status = (status == Media::ok)?output[i]->object->private_start(start_time, end_time):status;
        }
    }
    on_start(start_time, end_time);

    return status;
}

Media::status Abstract_media_object::private_stop(int& end_time)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    if (Media::stop == get_state())
    {
        MEDIA_WARNING("Already in Stop State: %s", object_name());
        return Media::ok;
    }

    Media::status status = Media::ok; //set_state(Abstract_media_object::STOP);
    for (int i = 0; i < output_count; i++)
    {
        if (0 != output[i]->object)
        {
            status = (status == Media::ok)?output[i]->object->private_stop(end_time):status;
        }
    }
    on_stop(end_time);

    MEDIA_LOG(": %s, End time: %d", object_name(), end_time);
    return status;
}

Media::status start(Abstract_media_object* src, int start_time, int end_time)
{
    if (src == 0)
    {
        MEDIA_ERROR("Source object NULL, start time: %d, end time: %d", start_time, end_time);
        return Media::invalid_object;
    }
    MEDIA_TRACE_PARAM("%s, start time: %d, end time: %d", src->object_name(), start_time, end_time);

    if (src->input_count != 0)
    {
        MEDIA_ERROR("Not a valid source, start time: %d, end time: %d", start_time, end_time);
        return Media::invalid_object;
    }

    return src->private_start(start_time, end_time);
}

Media::status start(Abstract_media_object& src, int start_time, int end_time)
{
    return start(&src, start_time, end_time);
}

Media::status stop(Abstract_media_object* src, int& end_time)
{
    if (src == 0)
    {
        MEDIA_ERROR("Source object %s", "NULL");
        return Media::invalid_object;
    }
    MEDIA_TRACE_PARAM("%s", src->object_name());

    if (src->input_count != 0)
    {
        MEDIA_ERROR("Not a valid source, Input ports exist: %d", src->input_count);
        return Media::invalid_object;
    }

    Media::status ret = src->private_stop(end_time);
    MEDIA_LOG(": %s, end time: %d", src->object_name(), end_time);
    return ret;
}

Media::status stop(Abstract_media_object& src, int& end_time)
{
    return stop(&src, end_time);
}

Media::status connect(Abstract_media_object* src, Abstract_media_object* dest)
{
    if (src == 0 || dest == 0)
    {
        MEDIA_ERROR("Invalid Source or Destination, (Source: 0x%llx, Dest: 0x%llx)", (unsigned long long)src, (unsigned long long)dest);
        return Media::invalid_object;
    }
    MEDIA_TRACE_PARAM("%s - %s", src->object_name(), dest->object_name());

    unsigned int src_type;
    int i, src_port, dest_port;
    for (i = 0; (i < src->output_count) && (0 != src->output[i]->object); i++);
    if (i >= src->output_count)
    {
        MEDIA_ERROR("No Free port in source object: 0x%llx", (unsigned long long)src);
        return Media::no_free_port;
    }
    src_port = i;

    src_type = src->output[src_port]->port.type;
    for (i = 0; i < dest->input_count; i++)
    {
        if ((0 == dest->input[i]->object) && (src_type == dest->input[i]->port.type))
        {
            break;
        }
    }
    if (i >= dest->input_count)
    {
        MEDIA_ERROR("No Free port in destination object: 0x%llx, with type: %d", (unsigned long long)src, src_port);
        return Media::no_free_port;
    }
    dest_port = i;

    src->output[src_port]->object = dest;
    src->output[src_port]->port_index = dest_port;

    dest->input[dest_port]->object = src;
    dest->input[dest_port]->port_index = src_port;

    src->on_connect(dest_port, dest);
    dest->on_connect(src_port, src);

    return Media::ok;
}

Media::status connect(Abstract_media_object* src, char* src_port, Abstract_media_object* dest, char* dest_port)
{
    if (src == 0 || dest == 0)
    {
        MEDIA_ERROR("Invalid Source or Destination, (Source: 0x%llx, Dest: 0x%llx)", (unsigned long long)src, (unsigned long long)dest);
        return Media::invalid_object;
    }
    MEDIA_TRACE_PARAM("(%s, %s) - (%s, %s)", src->object_name(), src_port, dest->object_name(), dest_port);
    
    unsigned int dest_type;
    int i, src_i, dest_i;
    for (i = 0; i < dest->input_count; i++)
    {
        if (dest->input[i]->object != 0)
        {
            continue;
        }
        if (strcmp(dest_port, dest->input[i]->port.port_name) == 0)
        {
            break;
        }
    }
    if (i >= dest->input_count)
    {
        MEDIA_ERROR("No Free port in dest object: 0x%llx", (unsigned long long)dest);
        return Media::no_free_port;
    }
    dest_i = i;
    dest_type = dest->input[i]->port.type;
    
    for (i = 0; i < src->output_count; i++)
    {
        if (src->output[i]->object != 0)
        {
            continue;
        }
        if (strcmp(src_port, src->output[i]->port.port_name) == 0
            && src->output[i]->port.type == dest_type)
        {
            break;
        }
    }
    if (i >= src->output_count)
    {
        MEDIA_ERROR("No Free port in src object: 0x%llx", (unsigned long long)src);
        return Media::no_free_port;
    }
    src_i = i;

    src->output[src_i]->object = dest;
    src->output[src_i]->port_index = dest_i;

    dest->input[dest_i]->object = src;
    dest->input[dest_i]->port_index = src_i;

    src->on_connect(dest_i, dest);
    dest->on_connect(src_i, src);
    
    return Media::ok;
}

Media::status connect(Abstract_media_object& src, char* src_port, Abstract_media_object& dest, char* dest_port)
{
    return connect(&src, src_port, &dest, dest_port);
}

Media::status connect(Abstract_media_object& src, Abstract_media_object& dest)
{
    return connect(&src, &dest);
}

Media::status disconnect(Abstract_media_object* src, Abstract_media_object* dest)
{
    if (src == 0 || dest == 0)
    {
        MEDIA_ERROR("Invalid Source or Destination, (Source: 0x%llx, Dest: 0x%llx)", (unsigned long long)src, (unsigned long long)dest);
        return Media::invalid_object;
    }
    MEDIA_TRACE_PARAM("%s - %s", src->object_name(), dest->object_name());

    int i, src_port, dest_port;
    for (i = 0; i < src->output_count; i++)
    {
        if (src->output[i]->object == dest)
        {
            break;
        }
    }
    if (i >= src->output_count)
    {
        MEDIA_ERROR("Not connected src: 0x%llx, dest:0x%llx", (unsigned long long)src, (unsigned long long)dest);
        return Media::not_connected;
    }
    src_port = i;
    for (i = 0; i < dest->input_count; i++)
    {
        if (dest->input[i]->object == src)
        {
            break;
        }
    }
    if (i >= dest->input_count)
    {
        MEDIA_ERROR("Not connected src: 0x%llx, dest:0x%llx", (unsigned long long)src, (unsigned long long)dest);
        return Media::not_connected;
    }
    dest_port = i;

    src->on_disconnect(dest_port, dest);
    dest->on_disconnect(src_port, src);

    src->output[src_port]->object = 0;
    src->output[src_port]->port_index = 0;

    dest->input[dest_port]->object = 0;
    dest->input[dest_port]->port_index = 0;

    return Media::ok;
}

Media::status disconnect(Abstract_media_object& src, Abstract_media_object& dest)
{
    return disconnect(&src, &dest);
}

Media::status Abstract_media_object::attach(Media::events event, Observer* obs)
{
    int found = 0;
    Media::status status = Media::ok;
    std::multimap<Media::events, Observer*>::iterator itr;
    std::pair<std::multimap<Media::events, Observer*>::iterator, std::multimap<Media::events, Observer*>::iterator> ret;
    if (0 == obs)
    {
        return Media::null_object;
    }
    obs_map_lock.lock();
    ret = obs_map.equal_range(event);
    for (itr = ret.first; itr != ret.second; ++itr)
    {
        if (obs == itr->second)
        {
            found = 1;
            break;
        }
    }
    if (0 == found)
    {
        obs_map.insert(std::pair<Media::events, Observer*>(event, obs));
    }
    obs_map_lock.unlock();
    return status;
}

Media::status Abstract_media_object::detach(Media::events event, Observer* obs)
{
    int found = 0;
    Media::status status = Media::ok;
    std::multimap<Media::events, Observer*>::iterator itr;
    std::pair<std::multimap<Media::events, Observer*>::iterator, std::multimap<Media::events, Observer*>::iterator> ret;
    obs_map_lock.lock();
    ret = obs_map.equal_range(event);
    for (itr = ret.first; itr != ret.second; ++itr)
    {
        if (obs == itr->second)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        obs_map.erase(itr);
    }
    else
    {
        status = Media::invalid_object;
    }
    obs_map_lock.unlock();
    return status;
}

Media::status Abstract_media_object::notify(Media::events event, Media_params& params)
{
    Observer* obs = 0;
    std::multimap<Media::events, Observer*>::iterator itr;
    std::pair<std::multimap<Media::events, Observer*>::iterator, std::multimap<Media::events, Observer*>::iterator> ret;
    obs_map_lock.lock();
    ret = obs_map.equal_range(event);
    for (itr = ret.first; itr != ret.second; ++itr)
    {
        obs = itr->second;
        if (0 != obs)
        {
            obs->event_handler(event, this, params);
        }
    }
    obs_map_lock.unlock();
    return Media::ok;
}
