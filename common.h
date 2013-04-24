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
#include <fstream>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <queue>
#include <limits>
#include <sys/wait.h>
#include <sys/types.h>
#include <execinfo.h>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

//*********************************************** COMMON ***************************************************
#define SERVER_PORT 7932 //default listen port
#define LISTENQ 20
#define MAX_EVENTS 20
#define WORK_THREADS 10 //number of work threads
#define SOCK_RECV_BUF 1024*1024 //connections recv buf size(about 60bytes per contact item)
#define SOCK_INACTIVE_TIMEOUT 20 //close inactive connections timeout
#define RELOAD_CFG_INTERVAL 3 //reload configure file interval time

#define LOCK boost::mutex::scoped_lock

#define NOW boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time())
#define NOW_STR NOW.c_str()
#define DATE boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day())
#define THREAD_ID boost::this_thread::get_id()

//*********************************************** LOG MODULE ***************************************************
#define LOG_DIR "./"
#define LOG_CONFIG "config.log"
#define GRACE_EXIT "grace.exit"

enum LogLevel
{
    LOG_LEVEL_DEBUG = 0, //debug information: running details
    LOG_LEVEL_INFO, //normal information:
    LOG_LEVEL_WARN, //warning information:bussiness works incorrectly, but can continue to run,might got unexcepted result
    LOG_LEVEL_ERROR, //error information: bussiness error, but won't affect other bussiness
    LOG_LEVEL_NODE, //node information: program runing to keypoint
    LOG_LEVEL_FATAL, //fatal information: bussiness error, and program can't run anymore, must exit
    LOG_LEVEL_NONE, //don't log anything.
};

const char g_strLogLevel[][6] = {"DEBUG", "INFO", "WARN", "ERROR", "NODE", "FATAL", "NONE"};

LogLevel g_logLevel = LOG_LEVEL_DEBUG;
bool g_logRaw = false;
bool g_log2File = false;
std::string g_logFileDate;
std::ofstream g_oflog;
std::streambuf* g_pCoutBuf = std::cout.rdbuf();

#define SET_LOG()\
{\
   if(g_log2File == true)\
   {\
        g_logFileDate = DATE;\
        std::string _filename = LOG_DIR;\
        _filename += "/";\
        _filename += g_logFileDate;\
        _filename += ".log";\
        g_oflog.open(_filename.c_str(), std::ios_base::app);\
        std::streambuf* _p = g_oflog.rdbuf();\
        std::cout.rdbuf(_p);\
    }\
    else\
    {\
        std::cout.rdbuf(g_pCoutBuf);\
        if(g_oflog.is_open())\
        {\
            g_oflog.flush();\
            g_oflog.close();\
        }\
        g_logFileDate = "";\
    }\
}

#define REOPEN_LOG() \
{\
    if(g_log2File == true)\
    {\
        try\
        {\
            std::string _filename = LOG_DIR;\
            _filename += "/";\
            _filename += DATE;\
            _filename += ".log";\
            std::cout.flush();\
            g_oflog.close();\
            g_oflog.open(_filename.c_str(), std::ios_base::app);\
            std::cout.rdbuf(g_oflog.rdbuf());\
            g_logFileDate = DATE;\
        }\
        catch(...)\
        {\
            printf("ERROR in writing log...\n");\
        }\
    }\
}

#define LOG(level, msg) \
    if(level >= g_logLevel)\
    {\
        if(!g_logRaw)\
        {\
            std::cout<<"["<<NOW<<"]["<<g_strLogLevel[level]<<"][Thread "<<THREAD_ID<<"]"<<msg<<std::endl;\
        }\
        else\
        {\
            std::cout<<msg<<std::endl;\
        }\
    }



//*********************************************** JSON MODULE ***************************************************
//json mseeage data filed name.
#define JSON_MSG_TYPE "msg"
#define JSON_UID "uid"
#define JSON_DATA "data"
#define JSON_CONTACT_NUM "num"
#define JSON_CONTACT_NAME "name"
#define JSON_CONTACT_STATUS "status"

#define ROOT_DIR "./data" //data file's base directory
#define KV_CONTACT_MAP "ContactsMap" //kv file name

#define JSON_PARSE(reader, str, value, expectedType) if ((reader.parse(str, value) == false))\
    {\
        LOG(LOG_LEVEL_ERROR, "parse json failed." << reader.getFormatedErrorMessages() << " str:" << str);\
        return false;\
    }\
    if (value.isNull() == true)\
    {\
        LOG(LOG_LEVEL_ERROR, "value is NULL. str:" << str);\
        return false;\
    }\
    if(value.type() !=expectedType)\
    {\
        LOG(LOG_LEVEL_ERROR, "expected type:" << expectedType<<", json type:"<<value.type());\
        return false;\
    }



//*********************************************** CRYPT MODULE ***************************************************
//aes crypt
typedef unsigned int u32;
typedef unsigned char u8;
#define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ ((u32)(pt)[2] << 8) ^ ((u32)(pt)[3]))
#define PUTU32(ct, st) { (ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); (ct)[2] = (u8)((st) >> 8); (ct)[3] = (u8)(st); }
#define AES_MAX_BUFFER_LEN SOCK_RECV_BUF-8


#endif /* COMMON_H */
