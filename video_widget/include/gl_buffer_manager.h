/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _GL_BUFFER_MANAGER_H_
#define _GL_BUFFER_MANAGER_H_

#include <media.h>
#include <buffer.h>
#include <abstract_buffer_manager.h>

#include <QQueue>
#include <QMutex>
#include <QGLWidget>
#include <QWaitCondition>

class Video_widget;

class Gl_buffer_manager:public QGLWidget, public Abstract_buffer_manager
{
public:
    Gl_buffer_manager(Video_widget* video, int count);
    ~Gl_buffer_manager();
    
    void map(Buffer* buffer);
    void* data(Buffer* buffer);
    void unmap(Buffer* buffer);
    
    Buffer* request(unsigned int size, unsigned int type, unsigned int param_size);
    
protected:
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);
    
    void free(Buffer* buffer);
    
protected:
    bool flag;
    int pool_size;
    GLuint* buffers;
    
    QMutex mutex;
    QWaitCondition wc;
    QQueue<GLuint> free_queue;
    QMap<GLuint, unsigned int> buff_size;
    
    Video_widget* widget; 
};

#endif
