/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Thread object scheduling
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "threadmanager.moc"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QThread>
#include <QThreadPool>
#include <QWaitCondition>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dynamicthread.h"
#include "workerobject.h"

namespace Digikam
{

class ParkingThread : public QThread
{
public:

    ParkingThread(QObject* parent = 0)
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

    void parkObject(QObject* object)
    {
        object->moveToThread(this);
        QMutexLocker locker(&mutex);
        condVar.wakeAll();
    }

    void moveToCurrentThread(QObject* parkedObject)
    {
        if (parkedObject->thread() == QThread::currentThread())
            return;

        // We have 1. The current thread
        //         2. ParkingThread's thread
        //         3. The parkedObject's thread, subject to change.

        QMutexLocker locker(&mutex);

        // first, wait until the object has been parked in ParkingThread by its owning thread.
        while (parkedObject->thread() != this)
            condVar.wait(&mutex);

        // then, now that it's parked in ParkingThread, make ParkingThread move it to the current thread.
        todo << qMakePair(parkedObject, QThread::currentThread());
        condVar.wakeAll();
    }

    virtual void run()
    {
        /* The quirk here is that this thread never runs an event loop.
         * That means events queud for parked object are only emitted when
         * these object have been moved to their own thread.
         */
        while (running)
        {
            QList<TodoPair> copyTodo;
            {
                QMutexLocker locker(&mutex);
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
            foreach (const TodoPair &pair, copyTodo)
            {
                pair.first->moveToThread(pair.second);
            }
        }
    }

    volatile bool running;
    typedef QPair<QObject*, QThread*> TodoPair;
    QMutex mutex;
    QWaitCondition condVar;
    QList<TodoPair> todo;
};

class WorkerObjectRunnable : public QRunnable
{
public:

    WorkerObjectRunnable(WorkerObject* object, ParkingThread* parkingThread);

protected:

    WorkerObject*  object;
    ParkingThread* parkingThread;

protected:

    virtual void run();
};

// --------------------------------------------------------------------------------------------------

WorkerObjectRunnable::WorkerObjectRunnable(WorkerObject* object, ParkingThread* parkingThread)
                    : object(object), parkingThread(parkingThread)
{
    setAutoDelete(true);
}

void WorkerObjectRunnable::run()
{
    if (!object)
        return;

    // if another thread should still be running, wait until the object is parked in ParkingThread
    parkingThread->moveToCurrentThread(object);

    QEventLoop loop;
    QObject::connect(object, SIGNAL(deactivating()),
                     &loop, SLOT(quit()));

    if (object->transitionToRunning())
        loop.exec();
    object->transitionToInactive();

    // if this is rescheduled, it will wait in the other thread at moveToCurrentThread() above until we park
    parkingThread->parkObject(object);
}

// -------------------------------------------------------------------------------------------------

class ThreadManagerPriv
{
public:

    ThreadManagerPriv()
    {
        parkingThread = 0;
    }

    ParkingThread* parkingThread;
    QThreadPool*   pool;

    void changeMaxThreadCount(int diff)
    {
        pool->setMaxThreadCount(pool->maxThreadCount() + diff);
    }
};

class ThreadManagerCreator { public: ThreadManager object; };
K_GLOBAL_STATIC(ThreadManagerCreator, creator)

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

void ThreadManager::initialize(WorkerObject* object)
{
    connect(object, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDestroyed(QObject*)));

    d->changeMaxThreadCount(+1);

    d->parkingThread->parkObject(object);
}

void ThreadManager::initialize(DynamicThread* dynamicThread)
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
