/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-06
 * Description : Multithreaded worker object
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

#include "workerobject.h"

// Qt includes

#include <QCoreApplication>
#include <QEvent>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "threadmanager.h"

namespace Digikam
{

class WorkerObject::WorkerObjectPriv
{
public:

    WorkerObjectPriv()
    {
        state         = WorkerObject::Inactive;
        eventLoop     = 0;
        runnable      = 0;
        inDestruction = false;
        priority      = QThread::InheritPriority;
    }

    volatile WorkerObject::State state;
    QMutex                       mutex;
    QWaitCondition               condVar;
    QEventLoop*                  eventLoop;
    WorkerObjectRunnable*        runnable;
    bool                         inDestruction;
    QThread::Priority            priority;
};

WorkerObject::WorkerObject()
    : d(new WorkerObjectPriv)
{
    ThreadManager::instance()->initialize(this);
}

WorkerObject::~WorkerObject()
{
    shutDown();
    delete d;
}

void WorkerObject::shutDown()
{
    {
        QMutexLocker locker(&d->mutex);
        d->inDestruction = true;
    }
    deactivate(PhaseOut);
    wait();
}

void WorkerObject::wait()
{
    QMutexLocker locker(&d->mutex);

    while (d->state != Inactive || d->runnable)
    {
        d->condVar.wait(&d->mutex);
    }
}

bool WorkerObject::connectAndSchedule(const QObject* sender, const char* signal, const char* method,
                                      Qt::ConnectionType type) const
{
    connect(sender, signal, this, SLOT(schedule()), Qt::DirectConnection);
    return QObject::connect(sender, signal, method, type);
}

bool WorkerObject::connectAndSchedule(const QObject* sender, const char* signal,
                                      const WorkerObject* receiver, const char* method,
                                      Qt::ConnectionType type)
{
    connect(sender, signal, receiver, SLOT(schedule()), Qt::DirectConnection);
    return QObject::connect(sender, signal, receiver, method, type);
}

bool WorkerObject::disconnectAndSchedule(const QObject* sender, const char* signal,
        const WorkerObject* receiver, const char* method)
{
    disconnect(sender, signal, receiver, SLOT(schedule()));
    return QObject::disconnect(sender, signal, receiver, method);
}

WorkerObject::State WorkerObject::state() const
{
    return d->state;
}

void WorkerObject::setPriority(QThread::Priority priority)
{
    if (d->priority == priority)
    {
        return;
    }

    d->priority = priority;

    if (d->priority != QThread::InheritPriority)
    {
        QMutexLocker locker(&d->mutex);

        if (d->state == Running)
        {
            thread()->setPriority(d->priority);
        }
    }
}

QThread::Priority WorkerObject::priority() const
{
    return d->priority;
}

bool WorkerObject::event(QEvent* e)
{
    if (e->type() == QEvent::User)
    {
        aboutToQuitLoop();
        d->eventLoop->quit();
        return true;
    }

    return QObject::event(e);
}

void WorkerObject::aboutToQuitLoop()
{
}

void WorkerObject::setEventLoop(QEventLoop* loop)
{
    d->eventLoop = loop;
}

void WorkerObject::addRunnable(WorkerObjectRunnable* runnable)
{
    QMutexLocker locker(&d->mutex);
    d->runnable = runnable;
}

void WorkerObject::removeRunnable(WorkerObjectRunnable* runnable)
{
    QMutexLocker locker(&d->mutex);

    // there could be a second runnable in the meantime, waiting for the other, leaving runnable to park
    if (d->runnable == runnable)
    {
        d->runnable = 0;
    }

    d->condVar.wakeAll();
}

void WorkerObject::schedule()
{
    {
        QMutexLocker locker(&d->mutex);

        if (d->inDestruction)
        {
            return;
        }

        switch (d->state)
        {
            case Inactive:
            case Deactivating:
                d->state = Scheduled;
                break;
            case Scheduled:
            case Running:
                return;
        }
    }

    ThreadManager::instance()->schedule(this);
}

void WorkerObject::deactivate(DeactivatingMode mode)
{
    {
        QMutexLocker locker(&d->mutex);

        switch (d->state)
        {
            case Scheduled:
            case Running:
                d->state = Deactivating;
                break;
            case Inactive:
            case Deactivating:
                return;
        }
    }

    aboutToDeactivate();

    if (mode == FlushSignals)
    {
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
    }

    // cannot say that this is thread-safe: thread()->quit();
    if (mode == KeepSignals)
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User), Qt::HighEventPriority);
    }
    else
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }
}

void WorkerObject::aboutToDeactivate()
{
}

bool WorkerObject::transitionToRunning()
{
    QMutexLocker locker(&d->mutex);

    switch (d->state)
    {
        case Running:
        case Scheduled:
            d->state = Running;
            return true;
        case Deactivating:
        default:
            return false;
    }
}

void WorkerObject::transitionToInactive()
{
    QMutexLocker locker(&d->mutex);

    switch (d->state)
    {
        case Scheduled:
            break;
        case Deactivating:
        default:
            d->state = Inactive;
            d->condVar.wakeAll();
            break;
    }
}

} // namespace Digikam
