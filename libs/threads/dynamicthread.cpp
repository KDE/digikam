/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Dynamically active thread
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

#include "dynamicthread.moc"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <kdebug.h>

// Local includes

#include "threadmanager.h"

namespace Digikam
{

class DynamicThreadPriv : public QRunnable
{
public:

    DynamicThreadPriv(DynamicThread* q) : q(q)
    {
        setAutoDelete(false);

        state = DynamicThread::Inactive;
        running = true;
        emitSignals = false;
    };

    virtual void run();
    bool transitionToRunning();
    void transitionToInactive();

    DynamicThread* const q;

    volatile bool running;
    volatile bool emitSignals;

    DynamicThread::State state;

    QMutex         mutex;
    QWaitCondition condVar;
};

DynamicThread::DynamicThread(QObject* parent)
             : QObject(parent), d(new DynamicThreadPriv(this))
{
    setAutoDelete(false);
    ThreadManager::instance()->initialize(this);
}

DynamicThread::~DynamicThread()
{
    stop();
    wait();
    delete d;
}

DynamicThread::State DynamicThread::state() const
{
    return d->state;
}

bool DynamicThread::isRunning() const
{
    return d->state == Scheduled || d->state == Running || d->state == Deactivating;
}

QMutex *DynamicThread::threadMutex() const
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

void DynamicThread::start(QMutexLocker &locker)
{
    switch (d->state)
    {
        case Inactive:
        case Deactivating:
            d->state = Scheduled;
            d->running = true;
            break;
        case Running:
        case Scheduled:
            return;
    }

    locker.unlock();
    ThreadManager::instance()->schedule(d);
    locker.relock();
}

void DynamicThread::stop(QMutexLocker &locker)
{
    Q_UNUSED(locker);
    switch (d->state)
    {
        case Scheduled:
        case Running:
            d->running = false;
            d->state = Deactivating;
            break;
        case Inactive:
        case Deactivating:
            break;
    }
}

bool DynamicThreadPriv::transitionToRunning()
{
    QMutexLocker locker(&mutex);
    switch (state)
    {
        case DynamicThread::Scheduled:
        case DynamicThread::Running:
            state = DynamicThread::Running;
            return true;
        case DynamicThread::Deactivating:
        default:
            return false;
    }
}

void DynamicThreadPriv::transitionToInactive()
{
    QMutexLocker locker(&mutex);
    switch (state)
    {
        case DynamicThread::Scheduled:
            return;
        case DynamicThread::Deactivating:
        default:
            state = DynamicThread::Inactive;
            condVar.wakeAll();
            break;
    }
}

void DynamicThreadPriv::run()
{
    if (emitSignals)
        emit q->started();
    if (transitionToRunning())
        q->run();
    if (emitSignals)
        emit q->finished();
    transitionToInactive();
    // as soon as we are inactive, we may get deleted!
}

void DynamicThread::wait(QMutexLocker &locker)
{
    Q_UNUSED(locker);
    while (d->state != Inactive)
        d->condVar.wait(&d->mutex);
}

bool DynamicThread::runningFlag() const
{
    return d->running;
}

} // namespace Digikam

