/*
 * File: debug.h
 * Author: raozf
 *
 * Created on 2013年3月27日, 下午6:12
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

bool pre_exit(pid_t pid, int status)
{
    bool _bNeedRestart = false;
    if (WIFEXITED(status))//pid terminated by exit
    {
        LOG(LOG_LEVEL_NODE, "[Demaon] Child process normal termination, exit status = " << WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))//pid terminated by signal
    {
        LOG(LOG_LEVEL_NODE, "[Demaon] Child process abnormal termination, killed by signal number = " << WTERMSIG(status) <<
#ifdef WCOREDUMP
                (WCOREDUMP(status) ? "(core file generated)" : "(No core file generated)"));
#else
                "WCOREDUMP not defined.");
#endif

        //need restart?
        //if(WTERMSIG(status) != SIGUSR1)//not killed by me,then restart it.
        {
            _bNeedRestart = true;
        }
    }
    else if (WIFSTOPPED(status))
    {
        LOG(LOG_LEVEL_NODE, "[Demaon] Child process stopped, signal number = " << WSTOPSIG(status));
    }

    return _bNeedRestart;
}

void dump_debug(int signo)
{
    void *array[30] = {0};
    size_t size;
    char **strings = NULL;
    size_t i;

    std::string strFile = "signal.log";
    FILE* pf = fopen(strFile.c_str(), "w"); //每次清空,防止程序不断重启导致signal.log文件过大

    size = backtrace(array, 30);
    strings = backtrace_symbols(array, size);
    if (strings != NULL)
    {
        fprintf(stderr, "[%s] get SIGSEGV[%d] signel.\n", NOW_STR, signo);
        fprintf(stderr, "[%s] Obtained %zd stack frames.\n", NOW_STR, size);
        if (pf != NULL)
        {
            fprintf(pf, "[%s] get SIGSEGV[%d] signel.\n", NOW_STR, signo);
            fprintf(pf, "[%s] Obtained %zd stack frames.\n", NOW_STR, size);
            fflush(pf);
        }

        for (i = 0; i < size; i++)
        {
            fprintf(stderr, "[%s] %s\n", NOW_STR, strings[i]);
            fflush(stderr);

            if (pf != NULL)
            {
                fprintf(pf, "[%s] %s\n", NOW_STR, strings[i]);
                fflush(pf);
            }
        }
    }

    if (pf != NULL)
    {
        fclose(pf);
        pf = NULL;
    }
    free(strings);

    exit(0);
}

void Debug_Printf_FrameInfos()
{
    signal(SIGSEGV, dump_debug);
    signal(SIGABRT, dump_debug);
}


#endif /* DEBUG_H */

