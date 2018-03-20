/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Dynamically active thread
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

#include "dynamicthread.h"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "threadmanager.h"

namespace Digikam
{

class DynamicThread::DynamicThreadPriv : public QRunnable
{
public:

    explicit DynamicThreadPriv(DynamicThread* const q)
        : q(q)
    {
        setAutoDelete(false);

        state            = DynamicThread::Inactive;
        running          = true;
        assignedThread   = 0;
        emitSignals      = false;
        inDestruction    = false;
        threadRequested  = false;
        priority         = QThread::InheritPriority;
        previousPriority = QThread::InheritPriority;
    };

    virtual void run();
    void takingThread();
    bool transitionToRunning();
    void transitionToInactive();

public:

    DynamicThread* const          q;
    QThread*                      assignedThread;

    volatile bool                 running;
    volatile bool                 emitSignals;
    bool                          inDestruction;
    bool                          threadRequested;

    volatile DynamicThread::State state;

    QThread::Priority             priority;
    QThread::Priority             previousPriority;

    QMutex                        mutex;
    QWaitCondition                condVar;
};

void DynamicThread::DynamicThreadPriv::takingThread()
{
    QMutexLocker locker(&mutex);
    // The thread we requested from the pool has now "arrived"
    threadRequested = false;
}

bool DynamicThread::DynamicThreadPriv::transitionToRunning()
{
    QMutexLocker locker(&mutex);

    switch (state)
    {
        case DynamicThread::Scheduled:
        {
            // ensure that a newly scheduled thread does not run
            // while an old, deactivated one has not yet called transitionToInactive
            while (assignedThread)
            {
                condVar.wait(&mutex);
            }

            state            = DynamicThread::Running;
            running          = true;
            assignedThread   = QThread::currentThread();
            previousPriority = assignedThread->priority();

            if (priority != QThread::InheritPriority)
            {
                assignedThread->setPriority(priority);
            }

            return true;
        }
        case DynamicThread::Deactivating:
        {
            return false;
        }
        case DynamicThread::Running:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Transition to Running: Invalid Running state" << q;
            return false;
        }
        case DynamicThread::Inactive:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Transition to Running: Invalid Inactive state" << q;
            return false;
        }
        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Transition to Running: Should never reach here: assert?" << q;
            return false;
        }
    }
}

void DynamicThread::DynamicThreadPriv::transitionToInactive()
{
    QMutexLocker locker(&mutex);

    switch (state)
    {
        case DynamicThread::Scheduled:
        case DynamicThread::Deactivating:
        case DynamicThread::Running:
        {
            if (previousPriority != QThread::InheritPriority)
            {
                assignedThread->setPriority(previousPriority);
                previousPriority = QThread::InheritPriority;
            }

            assignedThread = 0;

            if (state != DynamicThread::Scheduled)
            {
                state = DynamicThread::Inactive;
            }

            condVar.wakeAll();
            break;
        }
        case DynamicThread::Inactive:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Transition to Inactive: Invalid Inactive state" << q;
            break;
        }
    }
}

void DynamicThread::DynamicThreadPriv::run()
{
    if (emitSignals)
    {
        emit (q->starting());
    }

    if (transitionToRunning())
    {
        takingThread();
        q->run();
    }
    else
    {
        takingThread();
    }

    if (emitSignals)
    {
        emit (q->finished());
    }

    transitionToInactive();
    // as soon as we are inactive, we may get deleted!
}

// -----------------------------------------------------------------------------------------------

DynamicThread::DynamicThread(QObject* const parent)
    : QObject(parent), d(new DynamicThreadPriv(this))
{
    setAutoDelete(false);
    ThreadManager::instance()->initialize(this);
}

DynamicThread::~DynamicThread()
{
    shutDown();
    delete d;
}

void DynamicThread::shutDown()
{
    QMutexLocker locker(&d->mutex);
    d->inDestruction = true;
    stop(locker);
    wait(locker);
}

DynamicThread::State DynamicThread::state() const
{
    return d->state;
}

bool DynamicThread::isRunning() const
{
    return d->state == Scheduled || d->state == Running || d->state == Deactivating;
}

QMutex* DynamicThread::threadMutex() const
{
    return &d->mutex;
}

bool DynamicThread::isFinished() const
{
    return d->state == Inactive;
}

void DynamicThread::setEmitSignals(bool emitThem)
{
    d->emitSignals = emitThem;
}

void DynamicThread::setPriority(QThread::Priority priority)
{
    if (d->priority == priority)
    {
        return;
    }

    d->priority = priority;

    if (d->priority != QThread::InheritPriority)
    {
        QMutexLocker locker(&d->mutex);

        if (d->assignedThread)
        {
            d->assignedThread->setPriority(d->priority);
        }
    }
}

QThread::Priority DynamicThread::priority() const
{
    return d->priority;
}

void DynamicThread::start()
{
    QMutexLocker locker(&d->mutex);
    start(locker);
}

void DynamicThread::stop()
{
    QMutexLocker locker(&d->mutex);
    stop(locker);
}

void DynamicThread::wait()
{
    QMutexLocker locker(&d->mutex);
    wait(locker);
}

void DynamicThread::start(QMutexLocker& locker)
{
    if (d->inDestruction)
    {
        return;
    }

    switch (d->state)
    {
        case Inactive:
        case Deactivating:
        {
            d->running = true;
            d->state   = Scheduled;
            break;
        }
        case Running:
        case Scheduled:
        {
            return;
        }
    }

    if (!d->threadRequested)
    {
        // avoid issuing multiple thread requests after very fast start/stop/start calls
        d->threadRequested = true;

        locker.unlock();
        ThreadManager::instance()->schedule(d);
        locker.relock();
    }
}

void DynamicThread::stop(QMutexLocker& locker)
{
    Q_UNUSED(locker);

    switch (d->state)
    {
        case Scheduled:
        case Running:
        {
            d->running = false;
            d->state   = Deactivating;
            break;
        }
        case Inactive:
        case Deactivating:
        {
            d->running = false;
            break;
        }
    }
}

void DynamicThread::wait(QMutexLocker& locker)
{
    while (d->state != Inactive)
    {
        d->condVar.wait(locker.mutex());
    }
}

bool DynamicThread::runningFlag() const volatile
{
    return d->running;
}

} // namespace Digikam
