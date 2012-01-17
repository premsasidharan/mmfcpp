/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _PRIORITY_QUEUE_H
#define _PRIORITY_QUEUE_H

#include <stdio.h>

#include <mutex.h>
#include <media_debug.h>

template <typename Priority, typename Type>
class Priority_queue
{
    struct Node
    {
        Type data;
        Priority priority;
        struct Node* prev;
        struct Node* next;
    };

public:
    explicit Priority_queue(int _size)
        :size(_size)
        , queue(0)
        , free_count(0)
        , used_count(0)
        , rear_free(0)
        , front_free(0)
        , rear_used(0)
        , front_used(0)
    {
        queue = new Node[size];
        initialize();
    }
    ~Priority_queue()
    {
        delete [] queue;
        queue = 0;
        rear_free = front_free = 0;
        rear_used = front_used = 0;
    }

public:
    int push(Priority priority, Type data, int timeout_ms)
    {
        Node* node = 0;
        MEDIA_TRACE_OBJ_PARAM("Priority: %d, Type: %d, timeout: %d", priority, data, timeout_ms);
        free_mutex.lock();
        used_mutex.lock();
        while (1)
        {
            node = front_free;
            if (node == 0)
            {
                //printf("\npush (%d), No Free Buffers, Priority: %d, Data: 0x%x", size, priority, data);
                free_mutex.unlock();
                used_mutex.unlock();
                if (ETIMEDOUT == cv_free.timed_wait(timeout_ms))
                {
                    return 0;
                }
                free_mutex.lock();
                used_mutex.lock();
            }
            else
            {
                break;
            }
        }
        front_free = node->next;
        if (front_free == 0)
        {
            rear_free = 0;
        }

        node->data = data;
        node->priority = priority;
        node->prev = node->next = 0;

        if (front_used == 0 || rear_used == 0)
        {
            front_used = rear_used = node;
        }
        else
        {
            rear_used->next = node;
            node->prev = rear_used;
            rear_used = node;
        }
        ++used_count;
        --free_count;

        used_mutex.unlock();
        free_mutex.unlock();
        cv_used.signal();
        return 1;
    }
    Type pop(int timeout_ms)
    {
        Node* node = 0;
        MEDIA_TRACE_OBJ();
        used_mutex.lock();
        free_mutex.lock();
        while (1)
        {
            node = front_used;
            if (node == 0)
            {
                used_mutex.unlock();
                free_mutex.unlock();
                if (ETIMEDOUT == cv_used.timed_wait(timeout_ms))
                {
                    return 0;
                }
                used_mutex.lock();
                free_mutex.lock();
            }
            else
            {
                break;
            }
        }

        Type data = node->data;
        front_used = node->next;

        node->data = 0;
        node->priority = 0;
        node->prev = node->next = 0;

        if (front_used == 0)
        {
            rear_used = 0;
        }
        else
        {
            front_used->prev = 0;
        }

        if (rear_free == 0)
        {
            front_free = rear_free = node;
        }
        else
        {
            rear_free->next = node;
            node->prev = rear_free;
            rear_free = node;
        }

        ++free_count;
        --used_count;
        free_mutex.unlock();
        used_mutex.unlock();
        cv_free.signal();
        return data;
    }

protected:
    void print_queue()
    {
        free_mutex.lock();
        used_mutex.lock();
        Node* node = front_free;
        printf("\nFree List max (%d)", size);
        if (0 == node)
        {
            printf("Empty");
        }
        else
        {
            while (node != rear_free)
            {
                printf("0x%x (%d), ", node->data, node->priority);
                node = node->next;
            }
            printf("0x%x (%d)", node->data, node->priority);
        }
        node = front_used;
        printf("\nUsed List max (%d)", size);
        if (0 == node)
        {
            printf("Empty");
        }
        else
        {
            while (node != rear_used)
            {
                printf("0x%x (%d), ", node->data, node->priority);
                node = node->next;
            }
            printf("0x%x (%d)", node->data, node->priority);
        }
        used_mutex.unlock();
        free_mutex.unlock();
    }

private:
    Priority_queue(const Priority_queue& queue) {}
    Priority_queue operator=(const Priority_queue& queue)
    {
        return *this;
    }
    void initialize()
    {
        for (int i = 1; i < size; i++)
        {
            queue[i-1].data = 0;
            queue[i-1].priority = 0;
            queue[i-1].next = &queue[i];
            queue[i].prev = &queue[i-1];
        }

        queue[size-1].data = 0;
        queue[size-1].priority = 0;

        queue[0].prev = queue[size-1].next = 0;
        front_free = &queue[0];
        rear_free = &queue[size-1];
    }

private:
    int size;
    Node* queue;
    int free_count;
    int used_count;
    Node* rear_free;
    Node* front_free;
    Node* rear_used;
    Node* front_used;

    Mutex free_mutex;
    Mutex used_mutex;
    Condition_variable cv_free;
    Condition_variable cv_used;
};

#endif
