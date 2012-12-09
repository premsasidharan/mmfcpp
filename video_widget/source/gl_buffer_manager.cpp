/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <stdio.h>
#include <GL/glew.h>
#include <QGLWidget>
#include <QtDebug>

#include <media_debug.h>
#include <video_widget.h>
#include <yuv_parameters.h>
#include <gl_buffer_manager.h>

Gl_buffer_manager::Gl_buffer_manager(Video_widget* video, int count)
    :QGLWidget(video, video)
    , flag(false)
    , pool_size(count)
    , buffers(0)
    , widget(video)
{
    resize(0, 0);
    buffers = new GLuint[count];
}

Gl_buffer_manager::~Gl_buffer_manager()
{ 
    mutex.lock();
    glDeleteBuffers(pool_size, buffers);
    delete [] buffers; buffers = 0;
    doneCurrent();
    mutex.unlock();
}

void Gl_buffer_manager::initializeGL()
{
    glewInit();
    mutex.lock();
    glGenBuffers(pool_size, buffers);
    for (int i = 0; i < pool_size; i++)
    {
        free_queue.enqueue(buffers[i]);
        buff_size[buffers[i]] = 0;
    }
    mutex.unlock();
}

void Gl_buffer_manager::paintGL()
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    glVertex2f(-1.0, -1.0);
    glVertex2f(1.0, -1.0); 
    glVertex2f(1.0, 1.0); 
    glVertex2f(-1.0, 1.0); 
    glEnd();
}

void Gl_buffer_manager::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
}

Buffer* Gl_buffer_manager::request(unsigned int size, unsigned int type, unsigned int param_size)
{
    Buffer* buffer = 0;
    Yuv_param* param = 0;
    
    if (!flag)
    {
        makeCurrent();
        flag = true;
    }
    
    while (1)
    {
        mutex.lock();
        if (free_queue.isEmpty())
        {
            wc.wait(&mutex);
            mutex.unlock();
        }
        else
        {
            break;
        }
    }
    
    buffer = create_buffer(size, type, param_size, 0);
    param = (Yuv_param *)buffer->parameter();
    param->gl_buffer = free_queue.dequeue();
    if (size != buff_size[param->gl_buffer])
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, param->gl_buffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        buff_size[param->gl_buffer] = size;
    }
    mutex.unlock();
    return buffer;
}

void Gl_buffer_manager::free(Buffer* buffer)
{
    Yuv_param* param = 0;
    mutex.lock();
    param = (Yuv_param *)buffer->parameter();
    free_queue.enqueue(param->gl_buffer);
    delete_buffer(buffer);
    mutex.unlock();
    wc.wakeOne();
}

void* Gl_buffer_manager::data(Buffer* buffer)
{
    (void)buffer;
    void *ret = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    return ret;
}

void Gl_buffer_manager::map(Buffer* buffer)
{
    Yuv_param *param = (Yuv_param *)buffer->parameter();
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, param->gl_buffer);
}

void Gl_buffer_manager::unmap(Buffer* buffer)
{
    (void)buffer;
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
