/*
 * File: WorkThread.h
 * Author: chu
 *
 * Created on 2013年3月9日, 下午4:56
 */

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "common.h"
#include "TaskQueue.h"

class WorkThread
{
private:
    TaskQueue* m_TaskQueue;
    bool m_bExit;
    char m_buf[SOCK_RECV_BUF ];
    boost::condition m_condition;

public:

    WorkThread(TaskQueue* task_queue) : m_TaskQueue(task_queue), m_bExit(false)
    {
        memset(m_buf, 0, SOCK_RECV_BUF);
    }

    void
    Run()
    {
        LOG_NODE("Run().");

        int _fd = -1;
        while ( !m_bExit )
        {
            _fd = m_TaskQueue->Pop(); //will block if no task
            LOG_DEBUG("start to read. fd = " << _fd);

            //recv data.
            //TODO:need to set timeout
            int _ret = -1;
            int _read_bytes = 0;

            char* _p = m_buf;
            memset(m_buf, 0, SOCK_RECV_BUF);
            while ( true )
            {
                _ret = recv(_fd, _p, SOCK_RECV_BUF - _read_bytes, 0);
                if (_ret < 0)
                {
                    if (errno == EAGAIN)//no data to read on socket when nonblock:read finished.
                    {
                        break;
                    }
                    else
                    {
                        LOG_ERROR("recv() error, returned :" << _ret << ". " << strerror(errno) << "fd = " << _fd);
                        goto close_socket;
                    }
                }
                else if (_ret == 0)//peer point socket is closed
                {
                    //LOG_DEBUG("peer closed." << " fd = " << _fd);
                    goto close_socket;
                }
                else
                {
                    _read_bytes += _ret;
                    _p = m_buf + _ret;
                    if (_read_bytes >= SOCK_RECV_BUF)
                    {
                        LOG_WARN("socket " << _fd << " read " << _read_bytes << " bytes, reach max buffer length. discard.");
                        goto close_socket;
                    }
                }
            }

            //have read something
            if (_read_bytes > 0)
            {
                LOG_DEBUG("read " << _read_bytes << " bytes from fd:" << _fd << ". MSG:" << m_buf);
            }


            //process massge

            //continue;
close_socket:
            close(_fd);
        }

        LOG_NODE("exited!");
    }

    void
    Stop()
    {
        m_bExit = true;
    }
};

class ThreadPool
{
private:
    boost::unordered_map<WorkThread*, boost::thread*> m_threads;
    typedef boost::unordered_map<WorkThread*, boost::thread*>::iterator iterator;
public:

    ThreadPool(int thread_num, TaskQueue* task_queue)
    {
        for (int i = 0; i < thread_num; i++)
        {
            //?????????????????????????
            //?????????????????????????
            WorkThread _thread(task_queue);
            boost::thread _thrd(boost::bind(&WorkThread::Run, &_thread));
            m_threads.insert(std::pair<WorkThread*, boost::thread*>(&_thread, &_thrd));
        }
    }

    void
    StopAll()
    {

        BOOST_FOREACH(iterator::value_type& i, m_threads)
        {
            i.first->Stop();
        }

        BOOST_FOREACH(iterator::value_type& i, m_threads)
        {
            i.second->join();
        }
    }
};
#endif /* WORKTHREAD_H */