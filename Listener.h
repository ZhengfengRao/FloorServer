/*
 * File: EpollThread.h
 * Author: raozf
 *
 * Created on 2013年3月19日, 上午11:00
 */

#ifndef LISTENER_H
#define LISTENER_H

#include "common.h"
#include "Runable.h"

class Listener : public Runable
{
private:
    int m_fd;
    TaskQueue* m_TaskQueue;
    SocketManager* m_SocketManager;
private:

    Listener() : Runable(), m_fd(-1), m_TaskQueue(NULL), m_SocketManager(NULL) { }

    bool SetNonBlocking(int fd)
    {
        int opts;
        opts = fcntl(fd, F_GETFL);
        if (opts >= 0)
        {
            opts = opts | O_NONBLOCK;
            if (fcntl(fd, F_SETFL, opts) >= 0)
            {
                return true;
            }
            else
            {
                LOG(LOG_LEVEL_ERROR, "fcntl(fd, F_SETFL, opts) failed. opts = " << opts << ", fd = " << fd);
            }
        }
        else
        {
            LOG(LOG_LEVEL_ERROR, "fcntl(fd, F_GETFL) returned:" << opts << ", fd = " << fd);
        }

        return false;
    }

public:

    static Listener* GetInstance()
    {
        static Listener Instance;
        return &Instance;
    }

    bool Init(int port, TaskQueue* task_queue, SocketManager* socket_manager)
    {
        m_TaskQueue = task_queue;
        m_SocketManager = socket_manager;
        if ((task_queue == NULL) || (m_SocketManager == NULL))
        {
            LOG(LOG_LEVEL_FATAL, "Invalid paramers. m_TaskQueue:" << m_TaskQueue << ", m_SocketManager:" << m_SocketManager);
            return false;
        }

        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (SetNonBlocking(m_fd) == true)
        {
            struct sockaddr_in server_addr;
            bzero(&server_addr, sizeof (server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_family = INADDR_ANY;
            if (bind(m_fd, (sockaddr*) & server_addr, sizeof (server_addr)) == 0)
            {
                if (listen(m_fd, LISTENQ) == 0)
                {
                    return true;
                }
                else
                {
                    close(m_fd);
                    LOG(LOG_LEVEL_FATAL, "listen() failed. " << strerror(errno));
                }
            }
            else
            {
                close(m_fd);
                LOG(LOG_LEVEL_FATAL, "bind() failed. " << strerror(errno));
            }
        }

        return false;
    }

    virtual void Run()
    {
        LOG(LOG_LEVEL_NODE, "Listener::Run().");

        /* The size is not the maximum size of the backing store but just a hint to the kernel about how to dimension internal structures.
         * (Nowadays, size is ignored. Since Linux 2.6.8, the size argument is unused:The kernel dynamically sizes the required data structures
         * without needing this initial hint.)*/
        int ep_fd = epoll_create(256);
        struct epoll_event ev, events[MAX_EVENTS];
        ev.data.fd = m_fd;
        ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(ep_fd, EPOLL_CTL_ADD, m_fd, &ev);

        int nfds = 0;
        int conn_fd = -1;
        int sock_fd = -1;
        struct sockaddr_in client_addr;
        socklen_t clilen = sizeof (client_addr);
        memset(&client_addr, 0, sizeof (struct sockaddr_in));

        while ( m_bExit == false )
        {
            nfds = epoll_wait(ep_fd, events, MAX_EVENTS, 3000);
            for (int i = 0; i < nfds; i++)
            {
                if (events[i].data.fd == m_fd)
                {
                    conn_fd = accept(m_fd, (sockaddr*) & client_addr, &clilen);
                    if (conn_fd < 0)
                    {
                        LOG(LOG_LEVEL_ERROR, "accept() returned fd:" << conn_fd << ". error msg:" << strerror(errno));
                        continue;
                    }
                    if (SetNonBlocking(conn_fd) == false)
                    {
                        LOG(LOG_LEVEL_WARN, "[fd:" << conn_fd << "] SetNonBlocking() failed. close socket.");
                        close(conn_fd);
                        continue;
                    }
                    m_SocketManager->UpdateActiveTime(conn_fd);

                    //The string is returned in a statically allocated buffer, which subsequent calls will overwrite.
                    //which means:we don't need to free the string returnd by inet_ntoa(), but it's non-re-entry,be careful in multithreads.
                    char *str = inet_ntoa(client_addr.sin_addr);
                    LOG(LOG_LEVEL_NODE, "[fd:" << conn_fd << "] connect from:" << str);

                    ev.data.fd = conn_fd;
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(ep_fd, EPOLL_CTL_ADD, conn_fd, &ev); //add the new connection to epoll
                }
                else if (events[i].events & EPOLLIN)
                {
                    if ((sock_fd = events[i].data.fd) < 0)
                    {
                        LOG(LOG_LEVEL_WARN, "[fd:" << sock_fd << "] Invalid socket fd.");
                        continue;
                    }

                    LOG(LOG_LEVEL_DEBUG, "[fd:" << sock_fd << "] EPOLLIN. events:" << events[i].events);
                    m_TaskQueue->Push(sock_fd);
                    m_SocketManager->UpdateActiveTime(sock_fd);
                }
            }
        }

        LOG(LOG_LEVEL_NODE, "Listener::Run() exited.");
    }
};
#endif /* LISTENER_H */

