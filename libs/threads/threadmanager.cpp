/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Thread object scheduling
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
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

#include "threadmanager.h"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QThread>
#include <QThreadPool>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "dynamicthread.h"
#include "workerobject.h"

namespace Digikam
{

class ParkingThread : public QThread
{
public:

    explicit ParkingThread(QObject* const parent = 0)
        : QThread(parent), running(true)
    {
        start();
    }

    ~ParkingThread()
    {
        running = false;
        condVar.wakeAll();
        wait();
    }

    void parkObject(QObject* const object)
    {
        object->moveToThread(this);
        QMutexLocker locker(&mutex);
        condVar.wakeAll();
    }

    void moveToCurrentThread(QObject* const parkedObject)
    {
        if (parkedObject->thread() == QThread::currentThread())
        {
            return;
        }

        // We have 1. The current thread
        //         2. ParkingThread's thread
        //         3. The parkedObject's thread, subject to change.

        QMutexLocker locker(&mutex);

        // first, wait until the object has been parked in ParkingThread by its owning thread.
        while (parkedObject->thread() != this)
        {
            condVar.wait(&mutex);
        }

        QThread* targetThread = QThread::currentThread();
        // then, now that it's parked in ParkingThread, make ParkingThread move it to the current thread.
        todo << qMakePair(parkedObject, targetThread);
        condVar.wakeAll();

        // wait until ParkingThread has pushed the object to current thread
        while (parkedObject->thread() != targetThread)
        {
            condVar.wait(&mutex);
        }
    }

    virtual void run()
    {
        /* The quirk here is that this thread never runs an event loop.
         * That means events queued for parked object are only emitted when
         * these object have been moved to their own thread.
         */
        while (running)
        {
            QList<TodoPair> copyTodo;
            {
                QMutexLocker locker(&mutex);
                condVar.wakeAll();

                if (todo.isEmpty())
                {
                    condVar.wait(&mutex);
                    continue;
                }
                else
                {
                    copyTodo = todo;
                    todo.clear();
                }
            }
            foreach(const TodoPair& pair, copyTodo)
            {
                pair.first->moveToThread(pair.second);
            }
        }
    }

public:

    volatile bool                     running;
    typedef QPair<QObject*, QThread*> TodoPair;
    QMutex                            mutex;
    QWaitCondition                    condVar;
    QList<TodoPair>                   todo;
};

// --------------------------------------------------------------------------------------------------

class WorkerObjectRunnable : public QRunnable
{
public:

    WorkerObjectRunnable(WorkerObject* const object, ParkingThread* const parkingThread);

protected:

    WorkerObject*  object;
    ParkingThread* parkingThread;

protected:

    virtual void run();
};

// --------------------------------------------------------------------------------------------------

WorkerObjectRunnable::WorkerObjectRunnable(WorkerObject* const object, ParkingThread* const parkingThread)
    : object(object), parkingThread(parkingThread)
{
    setAutoDelete(true);
}

void WorkerObjectRunnable::run()
{
    if (!object)
    {
        return;
    }

    // if another thread should still be running, wait until the object is parked in ParkingThread
    parkingThread->moveToCurrentThread(object);

    // The object is in state Scheduled or Deactivating now.
    // It won't be deleted until Inactive, and as long a runnable is set.
    object->addRunnable(this);

    emit (object->started());

    if (object->transitionToRunning())
    {
        QThread::Priority previousPriority = QThread::currentThread()->priority();
        if (object->priority() != QThread::InheritPriority)
        {
            QThread::currentThread()->setPriority(object->priority());
        }

        QEventLoop loop;
        object->setEventLoop(&loop);
        loop.exec();
        object->setEventLoop(0);

        if (previousPriority != QThread::InheritPriority)
        {
            QThread::currentThread()->setPriority(previousPriority);
        }
    }

    object->transitionToInactive();
    emit (object->finished());

    // if this is rescheduled, it will wait in the other thread at moveToCurrentThread() above until we park
    parkingThread->parkObject(object);

    // now, free the object - in case it wants to get deleted
    object->removeRunnable(this);
}

// -------------------------------------------------------------------------------------------------

class ThreadManager::ThreadManagerPriv
{
public:

    ThreadManagerPriv()
    {
        parkingThread = 0;
        pool          = 0;
    }

    ParkingThread* parkingThread;
    QThreadPool*   pool;

    void changeMaxThreadCount(int diff)
    {
        pool->setMaxThreadCount(pool->maxThreadCount() + diff);
    }
};

class ThreadManagerCreator
{
public:
    ThreadManager object;
};
Q_GLOBAL_STATIC(ThreadManagerCreator, creator)

ThreadManager* ThreadManager::instance()
{
    return &creator->object;
}

ThreadManager::ThreadManager()
    : d(new ThreadManagerPriv)
{
    d->parkingThread = new ParkingThread(this);
    d->pool          = new QThreadPool(this);

    d->pool->setMaxThreadCount(5);
}

ThreadManager::~ThreadManager()
{
    delete d;
}

void ThreadManager::initialize(WorkerObject* const object)
{
    connect(object, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDestroyed(QObject*)));

    d->changeMaxThreadCount(+1);

    d->parkingThread->parkObject(object);
}

void ThreadManager::initialize(DynamicThread* const dynamicThread)
{
    connect(dynamicThread, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDestroyed(QObject*)));

    d->changeMaxThreadCount(+1);
}

void ThreadManager::schedule(WorkerObject* object)
{
    d->pool->start(new WorkerObjectRunnable(object, d->parkingThread));
}

void ThreadManager::schedule(QRunnable* runnable)
{
    d->pool->start(runnable);
}

void ThreadManager::slotDestroyed(QObject*)
{
    d->changeMaxThreadCount(-1);
}

} // namespace Digikam
