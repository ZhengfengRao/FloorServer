#include "common.h"
#include "Server.h"
#include "test.h"
#include "debug.h"

void CheckConfigure()
{
    try
    {
        boost::property_tree::ptree _pt;
        boost::property_tree::ini_parser::read_ini(LOG_CONFIG, _pt);

        try
        {
            std::string _strLevel = _pt.get<std::string>("loglevel");
            LogLevel _level = LOG_LEVEL_NONE;
            for (int i = 0; i<sizeof (g_strLogLevel) / sizeof (std::string); i++)
            {
                if (strcasecmp(_strLevel.c_str(), g_strLogLevel[i]) == 0)
                {
                    _level = (LogLevel) i;
                    break;
                }
            }

            if (_level != g_logLevel)
            {
                g_logLevel = _level;
                LOG(LOG_LEVEL_NODE, "Log level set to:" << g_strLogLevel[_level]);
            }
        }
        catch (boost::property_tree::ptree_error _error)
        {
            //LOG(LOG_LEVEL_DEBUG, _error.what());
        }

        try
        {
            std::string _strLogRaw = _pt.get<std::string>("lograw");
            if (strcasecmp(_strLogRaw.c_str(), "yes") == 0)
            {
                if (g_logRaw == false)
                {
                    LOG(LOG_LEVEL_NODE, "LogRaw set to: Yes");
                    g_logRaw = true;
                }
            }
            else if (strcasecmp(_strLogRaw.c_str(), "no") == 0)
            {
                if (g_logRaw == true)
                {
                    LOG(LOG_LEVEL_NODE, "LogRaw set to: No");
                    g_logRaw = false;
                }
            }
        }
        catch (boost::property_tree::ptree_error _error)
        {
            //LOG(LOG_LEVEL_DEBUG, _error.what());
        }

        try
        {
            std::string _strLog2File = _pt.get<std::string>("log2file");
            if (strcasecmp(_strLog2File.c_str(), "yes") == 0)
            {
                if (g_log2File == false)
                {
                    LOG(LOG_LEVEL_NODE, "Log2File set to: Yes");
                    g_log2File = true;
                    SET_LOG();
                }
            }
            else if (strcasecmp(_strLog2File.c_str(), "no") == 0)
            {
                if (g_log2File == true)
                {
                    LOG(LOG_LEVEL_NODE, "Log2File set to: No");
                    g_log2File = false;
                    SET_LOG();
                }
            }
        }
        catch (boost::property_tree::ptree_error _error)
        {
            //LOG(LOG_LEVEL_DEBUG, _error.what());
        }
    }
    catch (boost::property_tree::ini_parser_error _error)
    {
        //LOG(LOG_LEVEL_DEBUG, _error.what());
    }

}

void CheckLog()
{
    if(g_log2File == true)
    {
        if(g_logFileDate != DATE)
        {
            REOPEN_LOG()
        }
    }
}

bool CheckExit()
{
    try
    {
        boost::property_tree::ptree _pt;
        boost::property_tree::ini_parser::read_ini(GRACE_EXIT, _pt);

        try
        {
            std::string _strExit = _pt.get<std::string>("exit");
            if (strcasecmp(_strExit.c_str(), "now") == 0)
            {
                LOG(LOG_LEVEL_NODE, "********************* Detect Grace Exit! ****************************");
                LOG(LOG_LEVEL_NODE, "");
                LOG(LOG_LEVEL_NODE, "program will exit now.");
                LOG(LOG_LEVEL_NODE, "");
                LOG(LOG_LEVEL_NODE, "*********************************************************************");
                return true;
            }
        }
        catch (boost::property_tree::ptree_error _error)
        {
            //LOG(LOG_LEVEL_DEBUG, _error.what());
        }
    }
    catch (boost::property_tree::ini_parser_error _error)
    {
        //LOG(LOG_LEVEL_DEBUG, _error.what());
    }

    return false;
}

void Verbose()
{
    LOG(LOG_LEVEL_NODE, "*********************************************************************");
    LOG(LOG_LEVEL_NODE, " Aimu Server.");
    LOG(LOG_LEVEL_NODE, "Version: 0.99");
    LOG(LOG_LEVEL_NODE, "Build: " << __DATE__ << ", " << __TIME__);
    LOG(LOG_LEVEL_NODE, "Gcc: " << __VERSION__);
    LOG(LOG_LEVEL_NODE, "Author: Zhengfeng Rao, gisrzf@gmail.com");
    LOG(LOG_LEVEL_NODE, "");
    LOG(LOG_LEVEL_NODE, "Copyright ©2013, All rights reserved.");
    LOG(LOG_LEVEL_NODE, "*********************************************************************");
}

void DoParams(int& server_port, int argc, char** argv)
{
    if (argc >= 2)
    {
        int port = atoi(argv[1]);
        if (port > 0 && port < 65535)
        {
            server_port = port;
        }
        else
        {
            LOG(LOG_LEVEL_ERROR, "Invalid port:" << argv[1]);
        }
    }
    LOG(LOG_LEVEL_INFO, "Use port: " << server_port);
}

int mainFun(int argc, char** argv)
{
    Debug_Printf_FrameInfos();
    LOG(LOG_LEVEL_NODE, "Server starting...");
    Verbose();

    int server_port = SERVER_PORT;
    DoParams(server_port, argc, argv);

    Server* _pServer = Server::NewServer(server_port);
    if (_pServer != NULL)
    {
        _pServer->Run();
    }

    while (1)
    {
        sleep(3);
        CheckConfigure();
        CheckLog();
        if (CheckExit() == true)//need to exit.
        {
            Server::DestoryServer(_pServer);

            LOG(LOG_LEVEL_NODE, "All threads exited!");
            exit(EXIT_SUCCESS);
        }
    }
}


static int g_nPID = 0;

int main(int argc, char** argv)
{
    SET_LOG()
    LOG(LOG_LEVEL_NODE, "[Demaon] App start!");
    if ((g_nPID = fork()) == 0)//executed by child process
    {
        LOG(LOG_LEVEL_NODE, "Child process forked!");
        mainFun(argc, argv);
    }
    else//executed by parent process
    {
        int nStatus = 0;
        while (1)
        {
            int n = waitpid(g_nPID, &nStatus, 0); //will block
            LOG(LOG_LEVEL_NODE, "[Demaon] Child process exited!");
            if (n <= 0)
            {
                //continue;
            }
            else
            {
                if (pre_exit(g_nPID, nStatus) == true)
                {
                    LOG(LOG_LEVEL_NODE, "[Demaon] Need restart.");
                    sleep(3);
                    if ((g_nPID = fork()) == 0)
                    {
                        LOG(LOG_LEVEL_NODE, "[Demaon] Program restarting...");
                        mainFun(argc, argv);
                    }
                }
                else
                {
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}

