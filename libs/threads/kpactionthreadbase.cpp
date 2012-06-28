/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : re-implementation of action thread using threadweaver
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kpactionthreadbase.moc"

// Qt includes

#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <kurl.h>
#include <kdebug.h>
#include <ThreadWeaver/JobCollection>
#include <ThreadWeaver/Weaver>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>
#include <threadweaver/DebuggingAids.h>
#include <solid/device.h>

using namespace Solid;

namespace KIPIPlugins
{

class KPActionThreadBase::KPActionThreadBasePriv
{
public:

    KPActionThreadBasePriv()
    {
        running       = false;
        weaverRunning = false;
        weaver        = 0;
        log           = 0;
    }

    volatile bool         running;
    volatile bool         weaverRunning;

    QWaitCondition        condVarJobs;
    QMutex                mutex;
    QList<JobCollection*> todo;

    Weaver*               weaver;
    KPWeaverObserver*     log;
};

KPActionThreadBase::KPActionThreadBase(QObject* const parent)
    : QThread(parent), d(new KPActionThreadBasePriv)
{
    const int maximumNumberOfThreads = qMax(Device::listFromType(DeviceInterface::Processor).count(), 1);
    d->log                           = new KPWeaverObserver(this);
    d->weaver                        = new Weaver(this);
    d->weaver->registerObserver(d->log);
    d->weaver->setMaximumNumberOfThreads(maximumNumberOfThreads);
    kDebug() << "Starting Main Thread";
}

KPActionThreadBase::~KPActionThreadBase()
{
    kDebug() << "calling action thread destructor";
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d->log;
    delete d->weaver;
    delete d;
}

void KPActionThreadBase::slotFinished()
{
    kDebug() << "Finish Main Thread";
    emit QThread::finished();
    d->weaverRunning = false;
    d->condVarJobs.wakeAll();
}

void KPActionThreadBase::cancel()
{
    kDebug() << "Cancel Main Thread";
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->running       = false;
    d->weaverRunning = true;
    d->weaver->requestAbort();
    d->weaver->dequeue();
    d->condVarJobs.wakeAll();
}

void KPActionThreadBase::finish()
{
    d->weaver->finish();
}

void KPActionThreadBase::appendJob(JobCollection* const job)
{
    QMutexLocker lock(&d->mutex);
    d->todo << job;
    d->condVarJobs.wakeAll();
}

void KPActionThreadBase::run()
{
    d->running       = true;
    d->weaverRunning = false;
    kDebug() << "In action thread Run";

    while (d->running)
    {
        JobCollection* t = 0;
        {
            QMutexLocker lock(&d->mutex);
            if (!d->todo.isEmpty() && !d->weaverRunning)
            {
                t = d->todo.takeFirst();
            }
            else
            {
                d->condVarJobs.wait(&d->mutex);
            }
        }

        if (t)
        {
            connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                    this, SLOT(slotFinished()));

            connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                    t, SLOT(deleteLater()));

            d->weaverRunning = true;
            d->weaver->enqueue(t);
        }
    }

    d->weaver->finish();
    kDebug() << "Exiting Action Thread";
}

}  // namespace KIPIPlugins
