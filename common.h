/*
 * File: common.h
 * Author: chu
 *
 * Created on 2013年3月9日, 下午6:36
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered/unordered_map.hpp>

#define NOW boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time())
#define THREAD_ID boost::this_thread::get_id()
#define LOG_DEBUG(msg) std::cout<<"["<<NOW<<"][DEBUG]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_INFO(msg) std::cout<<"["<<NOW<<"][INFO]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_WARN(msg) std::cout<<"["<<NOW<<"][WARN]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_ERROR(msg) std::cout<<"["<<NOW<<"][ERROR]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_NODE(msg) std::cout<<"["<<NOW<<"][NODE]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_FATAL(msg) std::cout<<"["<<NOW<<"][FATAL]"<<"[Thread "<<THREAD_ID<<"] "<<msg<<std::endl;
#define LOG_RAW(msg) std::cout<<msg

//#define LOG_SIMPLE
#ifdef LOG_SIMPLE
#define LOG_DEBUG(msg) LOG_RAW(msg)
#define LOG_INFO(msg) LOG_RAW(msg)
#define LOG_WARN(msg) LOG_RAW(msg)
#define LOG_ERROR(msg) LOG_RAW(msg)
#define LOG_NODE(msg) LOG_RAW(msg)
#define LOG_FATAL(msg) LOG_RAW(msg)
#endif

#define LOCK boost::mutex::scoped_lock

#define SERVER_PORT 7932 //default listen port
#define LISTENQ 20
#define MAX_EVENTS 20
#define WORK_THREADS 10 //number of work threads
#define SOCK_RECV_BUF 1024*10 //connections recv buf size
#define SOCK_INACTIVE_TIMEOUT 20 //close inactive connections timeout
#define RELOAD_CFG_INTERVAL 3 //reload configure file interval time

//log_path,log_level,log_save_days
//grace_exit
//deny_ip
#endif /* COMMON_H */