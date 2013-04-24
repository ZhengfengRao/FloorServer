/*
 * File: TaskQueue.h
 * Author: chu
 *
 * Created on 2013年3月9日, 下午4:53
 */

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "common.h"

//class TaskQueue
//{
//private:
// int m_maxfds;
// std::queue<int> m_fds;
// pthread_mutex_t m_mutex;
// pthread_cond_t m_condition;
//public:
//
// TaskQueue(int nMaxfds = -1) :
// m_maxfds(nMaxfds),
// m_mutex(PTHREAD_MUTEX_INITIALIZER),
// m_condition(PTHREAD_COND_INITIALIZER) { }
//
// void Push(int fd)
// {
// pthread_mutex_lock(&m_mutex);
// if (m_maxfds > 0 && m_fds.size() >= m_maxfds)//connection is full. Reject it!
// {
// LOG(LOG_LEVEL_WARN, "************ TaskQueue full! ************ m_maxfds = " << m_maxfds << ", size = " << m_fds.size() << ", fd = " << fd);
// close(fd); //*********************
//
// pthread_mutex_unlock(&m_mutex);
// return;
// }
//
// m_fds.push(fd);
// pthread_cond_signal(&m_condition);
// pthread_mutex_unlock(&m_mutex);
// }
//
// //will block if no elements
// int Pop()
// {
// int fd = -1;
// pthread_mutex_lock(&m_mutex);
//
// //no elements, need to wait.
// if (m_fds.size() <= 0)
// {
// //shall be called with mutex locked by the calling thread or undefined behavior results.
// //thread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)函数传入的参数mutex用于保护条件，
// //因为我们在调用pthread_cond_wait时，如果条件不成立我们就进入阻塞，但是进入阻塞这个期间，如果条件变量改变了的话，
// //那我们就漏掉了这个条件。因为这个线程还没有放到等待队列上，所以调用pthread_cond_wait前要先锁互斥量，
// //即调用pthread_mutex_lock(),pthread_cond_wait在把线程放进阻塞队列后，自动对mutex进行解锁，
// //使得其它线程可以获得加锁的权利。这样其它线程才能对临界资源进行访问并在适当的时候唤醒这个阻塞的进程。
// //当pthread_cond_wait返回的时候又自动给mutex加锁,所以最后我们要手动解锁。
// pthread_cond_wait(&m_condition, &m_mutex);
// pthread_mutex_unlock(&m_mutex);
//
// //got singal, try to pop again.
// return Pop();
// }
//
// fd = m_fds.front();
// m_fds.pop();
// pthread_mutex_unlock(&m_mutex);
//
// return fd;
// }
//};

class TaskQueue
{
private:
    int m_maxfds;
    std::queue<int> m_fds;
    boost::mutex m_mutex;
    boost::condition m_condition;

    //pthread_mutex_t m_mutex;
    //pthread_cond_t m_condition;
public:

    TaskQueue(int nMaxfds = -1) :
    m_maxfds(nMaxfds) { }

    void Push(int fd)
    {
        LOG(LOG_LEVEL_DEBUG, "Push. fd:" << fd);
        LOCK _l(m_mutex);
        if (m_maxfds > 0 && m_fds.size() >= m_maxfds)//connection is full. Reject it!
        {
            LOG(LOG_LEVEL_WARN, "************ TaskQueue full! ************ m_maxfds = " << m_maxfds << ", size = " << m_fds.size() << ", fd = " << fd);
            close(fd); //*********************

            return;
        }

        m_fds.push(fd);
        m_condition.notify_one();
        LOG(LOG_LEVEL_DEBUG, "Push. finished");
    }

    //will block if no elements

    int Pop()
    {
        LOG(LOG_LEVEL_DEBUG, "Pop()");
        int fd = -1;
        LOCK _l(m_mutex);

        //no elements, need to wait.
        if (m_fds.size() <= 0)
        {
            //LOG(LOG_LEVEL_DEBUG, "Pop. start wait....");
            m_condition.wait(_l);
            _l.unlock();
            //LOG(LOG_LEVEL_DEBUG, "Pop. wait comes.");
            return Pop();
        }
        else
        {
            fd = m_fds.front();
            m_fds.pop();
            //LOG(LOG_LEVEL_DEBUG, "PopXXXXXXXXXXXXXX2");
        }

        //LOG(LOG_LEVEL_DEBUG, "Pop(). fd:"<<fd);
        return fd;
    }

    //timed block if no element

    int Pop_Timed()
    {
        int fd = -1;
        LOCK _l(m_mutex);

        //no elements, need to wait.
        if (m_fds.size() <= 0)
        {
            m_condition.timed_wait(_l, boost::get_system_time() + boost::posix_time::seconds(3));
            _l.unlock();
        }
        else
        {
            fd = m_fds.front();
            m_fds.pop();
        }
        return fd;
    }
};

#endif /* TASKQUEUE_H */

