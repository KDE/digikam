/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2011-12-28
 * @brief  Low level threads management for batch processing on multi-core
 *
 * @author Copyright (C) 2011-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 * @author Copyright (C) 2011-2012 by A Janardhan Reddy
 *         <a href="annapareddyjanardhanreddy at gmail dot com">annapareddyjanardhanreddy at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "actionthreadbase.h"

// Qt includes

#include <QMutexLocker>
#include <QWaitCondition>
#include <QMutex>
#include <QThreadPool>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

ActionJob::ActionJob()
    : QObject(),
      QRunnable(),
      m_cancel(false)
{
    setAutoDelete(false);
}

ActionJob::~ActionJob()
{
    cancel();
}

void ActionJob::cancel()
{
    m_cancel = true;
}

// -----------------------------------------------------------------

class Q_DECL_HIDDEN ActionThreadBase::Private
{
public:

    Private()
    {
        running = false;
        pool    = QThreadPool::globalInstance();
    }

    volatile bool       running;

    QWaitCondition      condVarJobs;
    QMutex              mutex;

    ActionJobCollection todo;
    ActionJobCollection pending;
    ActionJobCollection processed;

    QThreadPool*        pool;
};

ActionThreadBase::ActionThreadBase(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    defaultMaximumNumberOfThreads();
}

ActionThreadBase::~ActionThreadBase()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    //wait for the jobs to finish
    d->pool->waitForDone();

    // Cleanup all jobs from memory
    foreach(ActionJob* const job, d->todo.keys())
    {
        delete(job);
    }

    foreach(ActionJob* const job, d->pending.keys())
    {
        delete(job);
    }

    foreach(ActionJob* const job, d->processed.keys())
    {
        delete(job);
    }

    delete d;
}

void ActionThreadBase::setMaximumNumberOfThreads(int n)
{
    d->pool->setMaxThreadCount(n);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Using " << n << " CPU core to run threads";
}

int ActionThreadBase::maximumNumberOfThreads() const
{
    return d->pool->maxThreadCount();
}

void ActionThreadBase::defaultMaximumNumberOfThreads()
{
    const int maximumNumberOfThreads = qMax(QThreadPool::globalInstance()->maxThreadCount(), 1);
    setMaximumNumberOfThreads(maximumNumberOfThreads);
}

void ActionThreadBase::slotJobFinished()
{
    ActionJob* const job = dynamic_cast<ActionJob*>(sender());

    if (!job)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "One job is done";

    QMutexLocker lock(&d->mutex);

    d->processed.insert(job, 0);
    d->pending.remove(job);

    if (isEmpty())
    {
        d->running = false;
    }

    d->condVarJobs.wakeAll();
}

void ActionThreadBase::cancel()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Cancel Main Thread";
    QMutexLocker lock(&d->mutex);

    d->todo.clear();

    foreach(ActionJob* const job, d->pending.keys())
    {
        job->cancel();
        d->processed.insert(job, 0);
    }

    d->pending.clear();
    d->running = false;

    d->condVarJobs.wakeAll();
}

bool ActionThreadBase::isEmpty() const
{
    return d->pending.isEmpty();
}

void ActionThreadBase::appendJobs(const ActionJobCollection& jobs)
{
    QMutexLocker lock(&d->mutex);

    for (ActionJobCollection::const_iterator it = jobs.begin() ; it != jobs.end(); ++it)
    {
        d->todo.insert(it.key(), it.value());
    }

    d->condVarJobs.wakeAll();
}

void ActionThreadBase::run()
{
    d->running = true;

    while (d->running)
    {
        QMutexLocker lock(&d->mutex);

        if (!d->todo.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Action Thread run " << d->todo.count() << " new jobs";

            for (ActionJobCollection::iterator it = d->todo.begin() ; it != d->todo.end(); ++it)
            {
                ActionJob* const job = it.key();
                int priority         = it.value();

                connect(job, SIGNAL(signalDone()),
                        this, SLOT(slotJobFinished()));

                d->pool->start(job, priority);
                d->pending.insert(job, priority);
            }

            d->todo.clear();
        }
        else
        {
            d->condVarJobs.wait(&d->mutex);
        }
    }
}

}  // namespace Digikam
