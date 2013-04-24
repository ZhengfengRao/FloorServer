/*
 * File: Server.h
 * Author: raozf
 *
 * Created on 2013年3月27日, 下午6:28
 */

#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "TaskQueue.h"
#include "SocketManager.h"
#include "Listener.h"
#include "WorkThread.h"

class Server
{
private:
    int m_nServerPort;
    TaskQueue m_taskQueue;
    SocketManager m_manager;
    Listener* m_pListener;
    ThreadPoolPtr m_pWorkers;
    ThreadPtr m_listenThreadObj;
    ThreadPtr m_checkThreadObj;

private:

    Server(int server_port) :
    m_nServerPort(server_port), m_pListener(NULL) { }

    ~Server() { }

public:

    static Server* NewServer(int server_port)
    {
        Server* _p = new Server(server_port);
        return _p;
    }

    static void DestoryServer(Server* pServer)
    {
        LOG(LOG_LEVEL_NODE, "Server::DestoryServer() pServer = " << pServer);
        if (pServer != NULL)
        {
            pServer->Stop();
            delete pServer;
            pServer = NULL;

            LOG(LOG_LEVEL_NODE, "Server destoryed.");
        }
    }

    void Run()
    {
        m_pWorkers = ThreadPoolPtr(new ThreadPool(WORK_THREADS, &m_taskQueue));
        m_pListener = Listener::GetInstance();
        if (m_pListener->Init(m_nServerPort, &m_taskQueue, &m_manager) == false)
        {
            LOG(LOG_LEVEL_FATAL, "Listener Init() failed. programm exit!");
            exit(EXIT_FAILURE);
        }
        m_listenThreadObj = ThreadPtr(new boost::thread(&Listener::Run, m_pListener));
        m_checkThreadObj = ThreadPtr(new boost::thread(&SocketManager::Run, &m_manager));
    }

    void Stop()
    {
        LOG(LOG_LEVEL_NODE, "Server stopping...");
        m_pListener->Stop();
        m_listenThreadObj->join();

        m_pWorkers->StopAll();
        m_manager.Stop();
        m_listenThreadObj->join();
        LOG(LOG_LEVEL_NODE, "Server stopped.");
    }
};

#endif /* SERVER_H */

