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
//#include "Contacts.h"
#include "Message.h"
#include "Runable.h"

class Worker : public Runable
{
private:
    TaskQueue* m_TaskQueue;
    char m_buf[SOCK_RECV_BUF];
    //ContactsManager* m_ContactsManager;
public:

    Worker(TaskQueue* task_queue) : Runable(), m_TaskQueue(task_queue)
    {
        memset(m_buf, 0, SOCK_RECV_BUF);
        //m_ContactsManager = ContactsManager::GetInstance();
    }

    void Run()
    {
        LOG(LOG_LEVEL_NODE, "Worker::Run().");

        int _fd = -1;
        while ( !m_bExit )
        {
            //_fd = m_TaskQueue->Pop(); //will block if no task
            if ((_fd = m_TaskQueue->Pop_Timed()) == -1)
            {
                continue;
            }
            LOG(LOG_LEVEL_DEBUG, "[fd: " << _fd << "] start to read.");

            //recv data.
            int _ret = -1;
            int _read_bytes = 0;
            int _decrypted_len = 0;
            std::string _str;
            size_t _sent;

            char* _p = m_buf;
            memset(m_buf, 0, SOCK_RECV_BUF);

            while ( true )
            {
                _ret = recv(_fd, _p, SOCK_RECV_BUF - _read_bytes, 0);
                if (_ret < 0)
                {
                    if (errno == EAGAIN)//no data to read on socket when nonblock. read finished.
                    {
                        break;
                    }
                    else
                    {
                        LOG(LOG_LEVEL_ERROR, "[fd:" << _fd << "] recv() error, returned :" << _ret << ". error: " << strerror(errno));
                        goto close_socket;
                    }
                }
                else if (_ret == 0)//peer point socket is closed
                {
                    LOG(LOG_LEVEL_NODE, "[fd:" << _fd << "] peer closed.");
                    goto close_socket;
                }
                else
                {
                    _read_bytes += _ret;
                    _p = m_buf + _ret;
                    if (_read_bytes >= SOCK_RECV_BUF)
                    {
                        LOG(LOG_LEVEL_WARN, "[fd:" << _fd << "] read " << _read_bytes << " bytes, reach max buffer length. discard.");
                        goto close_socket;
                    }
                }
            }
            if (_read_bytes > 0)//have read something
            {
                LOG(LOG_LEVEL_DEBUG, "[fd:" << _fd << "] read " << _read_bytes << " bytes from fd:" << _fd << ". MSG:" << m_buf);
            }

            //*********************************************************************************************
            //Below is bussiness code.
            //*********************************************************************************************
            //decode http header
		
		    /*
            if (m_ContactsManager->Process(m_buf, _str) == true)
            {
                //LOG_DEBUG("rep:" << _str);
                _sent = send(_fd, _str.c_str(), _str.length(), 0);
                if (_sent < _str.length())
                {
                    LOG(LOG_LEVEL_WARN, "[fd:" << _fd << "] send response failed, error:" << strerror(errno) << ". response:" << _str << ". _sent:" << _sent << ", size:" << _str.length());
                }
                else
                {
                    LOG(LOG_LEVEL_INFO, "[fd:" << _fd << "] send ok.");
                }
            }
            */
            //*********************************************************************************************
            //Bussiness code finished.
            //*********************************************************************************************

            //continue;
close_socket:
            close(_fd);
            LOG(LOG_LEVEL_NODE, "[fd:" << _fd << "] socket closed.");
        }

        LOG(LOG_LEVEL_NODE, "Worker::Run() exited!");
    }
};
typedef boost::shared_ptr<Worker> WorkerPtr;
typedef boost::shared_ptr<boost::thread> ThreadPtr;

class ThreadPool
{
private:

    typedef struct _thread
    {

        _thread(WorkerPtr handle, ThreadPtr object) :
        m_handle(handle), m_object(object) { }

        WorkerPtr m_handle;
        ThreadPtr m_object;
    } WorkThread;
    std::vector<WorkThread> m_WorkThreads;
public:

    ThreadPool(int thread_num, TaskQueue* task_queue)
    {
        for (int i = 0; i < thread_num; i++)
        {
            WorkerPtr _worker(new Worker(task_queue));
            ThreadPtr _threadObj(new boost::thread(boost::bind(&Worker::Run, _worker.get())));
            WorkThread _thread(_worker, _threadObj);
            m_WorkThreads.push_back(_thread);
        }
    }

    ~ThreadPool() { }

    void StopAll()
    {
        LOG(LOG_LEVEL_NODE, "ThreadPool::StopAll()");

        BOOST_FOREACH(std::vector<WorkThread>::value_type& i, m_WorkThreads)
        {
            i.m_handle->Stop();
        }

        BOOST_FOREACH(std::vector<WorkThread>::value_type& i, m_WorkThreads)
        {
            i.m_object->join();
        }
        LOG(LOG_LEVEL_NODE, "ThreadPool::StopAll() ok.");
    }
};
typedef boost::shared_ptr<ThreadPool> ThreadPoolPtr;
#endif /* WORKTHREAD_H */

