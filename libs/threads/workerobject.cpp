/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-06
 * Description : Multithreaded worker object
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

#include "workerobject.moc"

// Qt includes

#include <QCoreApplication>
#include <QEvent>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

// KDE includes

#include <kdebug.h>

// Local includes

#include "threadmanager.h"

namespace Digikam
{

class WorkerObject::WorkerObjectPriv
{
public:

    WorkerObjectPriv()
    {
        state = WorkerObject::Inactive;
    }

    volatile WorkerObject::State state;
    QMutex                       mutex;
    QWaitCondition               condVar;
};

WorkerObject::WorkerObject()
            : d(new WorkerObjectPriv)
{
    ThreadManager::instance()->initialize(this);
}

WorkerObject::~WorkerObject()
{
    deactivate(FlushSignals);
    wait();
    delete d;
}

void WorkerObject::wait()
{
    QMutexLocker locker(&d->mutex);
    while (d->state != Inactive)
        d->condVar.wait(&d->mutex);
}

bool WorkerObject::connectAndSchedule(const QObject* sender, const char* signal, const char* method,
                                      Qt::ConnectionType type) const
{
    connect(sender, signal,
            this, SLOT(schedule()));

    return QObject::connect(sender, signal, method, type);
}

bool WorkerObject::connectAndSchedule(const QObject* sender, const char* signal,
                                      const QObject* receiver, const char* method,
                                      Qt::ConnectionType type)
{
    if (receiver == this)
        connect(sender, signal, this, SLOT(schedule()));

    return QObject::connect(sender, signal, receiver, method, type);
}

bool WorkerObject::disconnectAndSchedule(const QObject* sender, const char* signal,
                                         const QObject* receiver, const char* method)
{
    if (receiver == this)
    {
        connect(sender, signal,
                this, SLOT(schedule()));
    }

    return QObject::disconnect(sender, signal, receiver, method);
}

WorkerObject::State WorkerObject::state() const
{
    return d->state;
}

bool WorkerObject::event(QEvent *e)
{
    if (e->type() == QEvent::User)
    {
        QThread::currentThread()->quit();
        return true;
    }
    return QObject::event(e);
}

void WorkerObject::schedule()
{
    {
        QMutexLocker locker(&d->mutex);
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
    if (mode == FlushSignals)
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
    // cannot say that this is thread-safe: thread()->quit();
    if (mode == KeepSignals)
        QCoreApplication::postEvent(this, new QEvent(QEvent::User), Qt::HighEventPriority);
    else
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
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
