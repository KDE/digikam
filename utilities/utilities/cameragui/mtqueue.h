/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-30
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

// Qt includes.

#include <qptrqueue.h>
#include <qmutex.h>

namespace Digikam
{

template<class Type> class MTQueue
{

public:

    MTQueue()
    {
        queue_.setAutoDelete(true);
    }
    
    ~MTQueue()
    {
        flush();
    }
    
    bool isEmpty()
    {
        mutex_.lock();
        bool empty = queue_.isEmpty();
        mutex_.unlock();
        return empty;
    }
    
    void flush()
    {
        mutex_.lock();
        queue_.clear();
        mutex_.unlock();
    }
    
    void enqueue(Type * t)
    {
        mutex_.lock();
        queue_.enqueue(t);
        mutex_.unlock();
    }
    
    Type * dequeue()
    {
        mutex_.lock();
        Type * i = queue_.dequeue();
        mutex_.unlock();
        return i;
    }
    
    Type * head(bool lock=true)
    {
        if (lock)
        mutex_.lock();
        Type * i = queue_.head();
        if (lock)
            mutex_.unlock();
        return i;
    }
    
    int count()
    {
        mutex_.lock();
        int c = queue_.count();
        mutex_.unlock();
        return c;
    }
    
    void lock()
    {
        mutex_.lock();
    }
    
    void unlock()
    {
        mutex_.unlock();
    }
    
private:

    QPtrQueue<Type> queue_;
    QMutex mutex_;
};

}  // namespace Digikam

#endif  // COMMAND_QUEUE_H
