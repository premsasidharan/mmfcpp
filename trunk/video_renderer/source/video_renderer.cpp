/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <buffer.h>
#include <media_debug.h>
#include <video_renderer.h>

const Port Video_renderer::input_port[] = 
    {{
        Media::YUY2 |
        Media::YV12 |
        Media::I420 |
        Media::I422 |
        Media::I444 |
        Media::UYVY, 
        "left"
    },
	{
        Media::YUY2 |
        Media::YV12 |
        Media::I420 |
        Media::I422 |
        Media::I444 |
        Media::UYVY, 
        "right"
    }};

Video_renderer::Video_renderer(const char* _name, Child_clock* clk)
    :Abstract_media_object(_name)
    , curr_pos(0)
    , is_running(0)
	, view_count(0)
    , window(0)
    , child_clk(clk)
    , queue1(5)
	, queue2(5)
	, video_end(0)
	, video_start(0)
    , text_helper(0)
{
    MEDIA_TRACE_OBJ_PARAM("%s", _name);
	
    prev[0] = prev[1] = 0;
	queue[0] = &queue1;
	queue[1] = &queue2;
	create_input_ports(input_port, 2);
}

Video_renderer::~Video_renderer()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	for (int i = 0; i < 2; i++)
	{
		if (0 != prev[i])
		{
		    Buffer::release(prev[i]);
		    prev[i] = 0;
		}
	}
}

void Video_renderer::update_pts_text()
{
	Guard g(mutex);
    if (0 != text_helper)
    {
        text_helper->read_text(disp_text, Video_renderer::MAX_DISP_TEXT_LENGTH, curr_pos);
		window->show_text(disp_text);
    }
	else
	{
		window->show_text(0);
	}
	window->update();
}

void Video_renderer::play_video()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
	bool flag = true;
    Buffer* buffer[2] = {0, 0};
	for (int i = 0; i < view_count; i++)
	{
		buffer[i] = queue[i]->pop(2000);
		flag = flag & (0 != buffer[i]);
		if (false == flag)
		{
			MEDIA_ERROR("%s: Null, view_count: %d", object_name(), view_count);
		}
	}

    if (flag)
    {
		curr_pos = buffer[0]->pts();
        Yuv_param* parameter[2] = {0, 0};
		for (int i = 0; i < view_count; i++)
		{
			parameter[i] = (Yuv_param*) buffer[i]->parameter();
		    //MEDIA_ERROR(": %s, Buffer: %llx, pts: %llu (%dx%d) State: %s", object_name(),
		    //	(unsigned long long)buffer, buffer->pts(), parameter->width, parameter->height, "PLAY");
			if ((video_start < video_end) && (buffer[i]->pts() < video_start || buffer[i]->pts() > video_end))
			{
		    	MEDIA_LOG("Video_renderer: %s, shedding buffer (pts:%lu)", object_name(), buffer[i]->pts());
				Buffer::release(buffer[i]);
				if (i < view_count-1)
				{
					continue;
				}
				else
				{
					return;
				}
			}
			window->show_frame(i, buffer[i]->type(), parameter[i]->width, parameter[i]->height, (uint8_t*)buffer[i]->data());
			if (0 !=  prev[i])
			{
			    Buffer::release(prev[i]);
			}
			prev[i] = buffer[i];
		    if (buffer[i]->flags() & LAST_PKT)
		    {
		        set_state(Media::stop);
		        Media_params params;
		        memset(&params, 0, sizeof(Media_params));
		        notify(Media::last_pkt_rendered, params);
		    }
		}
		update_pts_text();
		window->update();
		child_clk->wait_for_sync(curr_pos);
        //TODO:Free Last packet
    }
    else
    {
        MEDIA_ERROR("Video_renderer: %s, Null Buffer", object_name());
    }
}

int Video_renderer::run()
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    Media::state state = Media::stop;
    while (is_running)
    {
        state = get_state();
        switch (state)
        {
            case Media::init:
                MEDIA_LOG("%s, State: %s", object_name(), "INIT");
                cv.wait();
                break;

            case Media::stop:
                MEDIA_LOG("%s, State: %s", object_name(), "STOP");
                stop_cv.signal();
                cv.wait();
                break;

            case Media::play:
                MEDIA_LOG("%s, State: %s", object_name(), "PLAY");
                play_video();
                break;

            default:
                MEDIA_ERROR("%s, State: %s", object_name(), "Invalid");
                break;
        }
    }
    MEDIA_WARNING("Exiting Thread: %s", object_name());
    return 0;
}

Media::status Video_renderer::on_start(int start, int end)
{
    MEDIA_TRACE_OBJ_PARAM("%s, start: %d end: %d", object_name(), start, end);
	flush();
	video_end = end;
	video_start = start;
    set_state(Media::play);
	window->set_views(view_count);
    cv.signal();
    return Media::ok;
}

Media::status Video_renderer::on_stop(int end)
{
    MEDIA_TRACE_OBJ_PARAM("%s", object_name());
    set_state(Media::stop);
    stop_cv.wait();
	end = curr_pos;
    MEDIA_LOG("Stop state: %s", object_name());
    return Media::ok;
}

Media::status Video_renderer::on_connect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
	if (view_count < 2)
	{
		++view_count;
	}
	if (0 == is_running)
	{
    	is_running = 1;
    	thread.start(this);
	}
    return Media::ok;
}

Media::status Video_renderer::on_disconnect(int port, Abstract_media_object* pobj)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
	if (is_running)
	{
    	is_running = 0;
    	cv.signal();
    	thread.join();
    	MEDIA_LOG("Thread stopped: %s", object_name());
	}
	if (view_count > 0)
	{
		--view_count;
	}
    return Media::ok;
}

Media::status Video_renderer::input_data(int port, Buffer* buffer)
{
    MEDIA_TRACE_OBJ_PARAM("%s, port: %d", object_name(), port);
    int status = 0;
    if (Media::play == get_state())
    {
        status = queue[port]->push(buffer->pts(), buffer, 500);
        MEDIA_LOG("Video_renderer: %s, port: %d, Buffer: 0x%llx, pts: %llu, Status: %d", object_name(), port, 
			(unsigned long long)buffer, buffer->pts(), status);
    }
    else
    {
        MEDIA_ERROR("Packet received under non-play state: %s, Buffer: 0x%llx, port: %d, pts: %llu", object_name(), 
			(unsigned long long)buffer, port, buffer->pts());
        Buffer::release(buffer);
        //sleep(1);
        return Media::non_play_state;
    }
    return Media::ok;
}

int Video_renderer::current_position() const
{
	int time = 0;
	mutex.lock();
	time = curr_pos;
	mutex.unlock();
	return time;
}

void Video_renderer::set_render_widget(Video_widget* _window)
{
	mutex.lock();
	window = _window;
	mutex.unlock();
}

void Video_renderer::register_text_helper(Abstract_text_helper* helper)
{
    mutex.lock();
    text_helper = helper;
    mutex.unlock();
}

void Video_renderer::unregister_text_helper()
{
    mutex.lock();
    text_helper = 0;
    mutex.unlock();
}

void Video_renderer::flush()
{
	Guard g(mutex);
	for (int i = 0; i < 2; i++)
	{
		while (queue[i]->size() > 0)
		{
			Buffer* buff = queue[i]->pop(1);
			Buffer::release(buff);
		}
		if (0 != prev[i])
		{			
			Yuv_param* param = (Yuv_param*) prev[i]->parameter();
			window->show_frame(i, prev[i]->type(), param->width, param->height, 0);
			if (0 !=  prev[i])
			{
		    	Buffer::release(prev[i]);
				prev[i] = 0;
			}
		}
	}
}

