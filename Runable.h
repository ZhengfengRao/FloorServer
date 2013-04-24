/*
 * File: Runable.h
 * Author: raozf
 *
 * Created on 2013年3月19日, 上午11:08
 */

#ifndef RUNABLE_H
#define RUNABLE_H

class Runable
{
protected:
    bool m_bExit;

public:

    Runable() : m_bExit(false) { }

    virtual void Run() = 0;

    void Stop()
    {
        m_bExit = true;
    }
};

#endif /* RUNABLE_H */

