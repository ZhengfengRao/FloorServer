/*
 * File: SocketManager.h
 * Author: raozf
 *
 * Created on 2013年3月11日, 下午5:32
 */

#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include "common.h"

class SocketManager
{
private:
    boost::unordered_map<int, time_t> m_sockets;
    pthread_mutex_t m_mutex;
    bool m_bExit;

    typedef boost::unordered_map<int, time_t>::iterator iterator;

public:

    SocketManager() :
    m_mutex(PTHREAD_MUTEX_INITIALIZER),
    m_bExit(false) { }

    void
    UpdateActiveTime(int sock_fd)
    {
        time_t _now = time(NULL);
        pthread_mutex_lock(&m_mutex);
        iterator it = m_sockets.find(sock_fd);
        if (it == m_sockets.end())
        {
            m_sockets.insert(std::pair<int, time_t>(sock_fd, _now));
            LOG_DEBUG("insert fd:" << sock_fd << ", time:" << _now);
        }
        else
        {
            it->second = _now;
            LOG_DEBUG("update fd:" << sock_fd << ", time:" << _now);
        }

        pthread_mutex_unlock(&m_mutex);
    }

    void
    Run()
    {
        while ( m_bExit == false )
        {
            time_t _now = time(NULL);
            pthread_mutex_lock(&m_mutex);

            BOOST_FOREACH(iterator::value_type& it, m_sockets)
            {
                if (_now - it.second >= SOCK_INACTIVE_TIMEOUT)
                {
                    close(it.first);
                    //LOG_INFO("socket "<< it.first<<" inactive timeout, force closed. update time: "<<it.second);

                    //m_sockets.erase(it.first);
                    //unnecessary to remove it.
                    //the socket handle will be used for other sockets.
                }
            }
            pthread_mutex_unlock(&m_mutex);

            sleep(2);
        }
    }
};


#endif /* SOCKETMANAGER_H */