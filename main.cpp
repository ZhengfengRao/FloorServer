#include <boost/unordered/unordered_map.hpp>

#include "common.h"
#include "TaskQueue.h"
#include "WorkThread.h"
#include "SocketManager.h"

bool SetNonBlocking(int sock_fd)
{
    int opts;
    opts = fcntl(sock_fd, F_GETFL);
    if (opts >= 0)
    {
        opts = opts | O_NONBLOCK;
        if (fcntl(sock_fd, F_SETFL, opts) >= 0)
        {
            return true;
        }
        else
        {
            LOG_ERROR("fcntl(sock_fd, F_SETFL, opts) failed. opts = " << opts << ", fd = " << sock_fd);
        }
    }
    else
    {
        LOG_ERROR("fcntl(sock_fd, F_GETFL) returned:" << opts << ", fd = " << sock_fd);
    }

    return false;
}

int InitSocket(int server_port)
{
    int _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (SetNonBlocking(_fd) == false)
    {
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_family = INADDR_ANY;
    if (bind(_fd, (sockaddr*) & server_addr, sizeof (server_addr)) != 0)
    {
        LOG_FATAL("bind() failed. " << strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (listen(_fd, LISTENQ) != 0)
    {
        LOG_FATAL("listen() failed. " << strerror(errno));
        exit(EXIT_FAILURE);
    }

    return _fd;
}

void StartEpoll(int listen_fd, TaskQueue* task_queue, SocketManager* socket_manager)
{
    /* The size is not the maximum size of the backing store but just a hint to the kernel about how to dimension internal structures.
     * (Nowadays, size is ignored. Since Linux 2.6.8, the size argument is unused:The kernel dynamically sizes the required data structures
     * without needing this initial hint.)*/
    int ep_fd = epoll_create(256);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    int nfds = 0;
    int conn_fd = -1;
    int sock_fd = -1;
    struct sockaddr_in client_addr;
    socklen_t clilen = sizeof (client_addr);
    memset(&client_addr, 0, sizeof (struct sockaddr_in));

    for (;;)
    {
        // //TODO:remove closed socket handle.
        // //the socket handle will be re-used for later sockets. maybe it's unnessary to remove it?????
        // if()
        // {
        // epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &ev) ;
        // }

        nfds = epoll_wait(ep_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listen_fd)
            {
                conn_fd = accept(listen_fd, (sockaddr*) & client_addr, &clilen);
                if (conn_fd < 0)
                {
                    LOG_ERROR("accept() returned fd:" << conn_fd << ". error msg:" << strerror(errno));
                    continue;
                }
                SetNonBlocking(conn_fd);
                socket_manager->UpdateActiveTime(conn_fd);

                //The string is returned in a statically allocated buffer, which subsequent calls will overwrite.
                //which means:we don't need to free the string returnd by inet_ntoa(), but it's non-re-entry,be careful in multithreads.
                char *str = inet_ntoa(client_addr.sin_addr);
                LOG_INFO("connect from:" << str);

                ev.data.fd = conn_fd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(ep_fd, EPOLL_CTL_ADD, conn_fd, &ev); //add the new connection to epoll
            }
            else if (events[i].events & EPOLLIN)
            {
                if ((sock_fd = events[i].data.fd) < 0)
                {
                    LOG_INFO("Invalid socket fd:" << sock_fd);
                    continue;
                }

                LOG_INFO("EPOLLIN. socket fd:" << sock_fd << ", events:" << events[i].events);
                task_queue->Push(sock_fd);
                socket_manager->UpdateActiveTime(sock_fd);
            }
        }
    }
}

void Init()
{

}

void Pause()
{
    pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&_mutex);
    pthread_cond_wait(&_cond, &_mutex);
    pthread_mutex_unlock(&_mutex);
}

int main(int argc, char** argv)
{
    int server_port = SERVER_PORT;
    if (argc >= 2)
    {
        int port = atoi(argv[1]);
        if (port > 0 && port < 65535)
        {
            server_port = port;
        }
        else
        {
            LOG_ERROR("Invalid port:" << argv[1]);
        }
    }
    LOG_INFO("Use port: " << server_port);

    //
    Init();

    //start workthreads
    TaskQueue _taskQueue;
    ThreadPool _pool(WORK_THREADS, &_taskQueue);
    SocketManager _manager;

    int listen_fd = InitSocket(server_port);
    boost::thread _epollThread(StartEpoll, listen_fd, &_taskQueue, &_manager);
    boost::thread _checkThread(&SocketManager::Run, &_manager);
    //boost::thread _ctrlThread();

    //use signal to pause(block the thread) forever, not sleep loop.
    Pause();
    /*
    while(1)
    {
    sleep(10);

   // ***************************** NOTICE ********************************
   // sleep the main thread may cause crash(when epoll got soemthing from the socket).
   // the crash is happened in main thread, all other threads works quite well.
   // i got no backtrace info in gdb:
   // Program received signal SIGSEGV, Segmentation fault.
   // 0x0000000000000000 in ?? ()
   // #0 0x0000000000000000 in ?? ()
   // #1 0x0000000000000000 in ?? ()
   // (gdb) info threads
   // 13 Thread 0x7fffcbfff700 (LWP 28064) 0x00000035950ab91d in nanosleep ()
   // from /lib64/libc.so.6
   // 12 Thread 0x7fffecdfa700 (LWP 28063) 0x00000035950e7c73 in epoll_wait ()
   // from /lib64/libc.so.6
   // 11 Thread 0x7fffed7fb700 (LWP 28062) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 10 Thread 0x7fffee1fc700 (LWP 28061) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 9 Thread 0x7fffeebfd700 (LWP 28060) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 8 Thread 0x7fffef5fe700 (LWP 28059) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 7 Thread 0x7ffff53ba700 (LWP 28058) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 6 Thread 0x7ffff5dbb700 (LWP 28057) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 5 Thread 0x7fffeffff700 (LWP 28056) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 4 Thread 0x7ffff67bc700 (LWP 28055) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 3 Thread 0x7ffff71bd700 (LWP 28054) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // 2 Thread 0x7ffff7bbe700 (LWP 28053) 0x000000359540b43c in pthread_cond_wait@@GLIBC_2.3.2 () from /lib64/libpthread.so.0
   // * 1 Thread 0x7ffff7bc0720 (LWP 27892) 0x0000000000000000 in ?? ()
   //
   // i guess this is maybe the main thread disturbe epoll's working?.......
   // need to be resolved.
    }
     */
}