/*
 * File: TaskQueue.h
 * Author: chu
 *
 * Created on 2013年3月9日, 下午4:53
 */

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "common.h"

class TaskQueue
{
private:
    int m_maxfds;
    std::queue<int> m_fds;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
public:

    TaskQueue(int nMaxfds = -1) :
    m_maxfds(nMaxfds),
    m_mutex(PTHREAD_MUTEX_INITIALIZER),
    m_condition(PTHREAD_COND_INITIALIZER) { }

    void
    Push(int fd)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_maxfds > 0 && m_fds.size() >= m_maxfds)
        {
            //connection is full. Reject it!
            LOG_WARN("************ TaskQueue full! ************ m_maxfds = " << m_maxfds << ", size = " << m_fds.size() << ", fd = " << fd);
            close(fd); //*********************

            pthread_mutex_unlock(&m_mutex);
            return;
        }

        m_fds.push(fd);
        pthread_cond_signal(&m_condition);
        pthread_mutex_unlock(&m_mutex);
    }

    //will block if no elements

    int
    Pop()
    {
        int fd = -1;
        pthread_mutex_lock(&m_mutex);

        //no elements, need to wait.
        if (m_fds.size() <= 0)
        {
            pthread_mutex_unlock(&m_mutex);
            pthread_cond_wait(&m_condition, &m_mutex);
            pthread_mutex_unlock(&m_mutex);

            return Pop();
        }

        fd = m_fds.front();
        m_fds.pop();
        pthread_mutex_unlock(&m_mutex);
        return fd;
    }
};

#endif /* TASKQUEUE_H */