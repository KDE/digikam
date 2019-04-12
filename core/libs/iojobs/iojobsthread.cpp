/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "iojobsthread.h"

// Qt includes

#include <QFileInfo>
#include <QDir>

// Local includes

#include "iojob.h"
#include "iojobdata.h"
#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN IOJobsThread::Private
{

public:

    explicit Private()
      : jobsCount(0),
        isCanceled(false),
        jobData(nullptr)
    {
    }

    int         jobsCount;
    bool        isCanceled;

    IOJobData*  jobData;

    QStringList errorsList;
};

IOJobsThread::IOJobsThread(QObject* const parent)
    : ActionThreadBase(parent),
      d(new Private)
{
    setObjectName(QLatin1String("IOJobsThread"));
}

IOJobsThread::~IOJobsThread()
{
    delete d->jobData;
    delete d;
}

void IOJobsThread::copyOrMove(IOJobData* const data)
{
    d->jobData = data;

    ActionJobCollection collection;

    int threads = qMin(maximumNumberOfThreads(),
                       data->sourceUrls().count());

    for (int i = 0 ; i < threads ; ++i)
    {
        CopyOrMoveJob* const j = new CopyOrMoveJob(data);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::deleteFiles(IOJobData* const data)
{
    d->jobData = data;

    ActionJobCollection collection;

    int threads = qMin(maximumNumberOfThreads(),
                       data->sourceUrls().count());

    for (int i = 0 ; i < threads ; ++i)
    {
        DeleteJob* const j = new DeleteJob(data);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::renameFile(IOJobData* const data)
{
    d->jobData = data;
    ActionJobCollection collection;

    RenameFileJob* const j = new RenameFileJob(data);

    connectOneJob(j);

    connect(j, SIGNAL(signalRenameFailed(QUrl)),
            this, SIGNAL(signalRenameFailed(QUrl)));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::listDTrashItems(const QString& collectionPath)
{
    ActionJobCollection collection;

    DTrashItemsListingJob* const j = new DTrashItemsListingJob(collectionPath);

    connect(j, SIGNAL(trashItemInfo(DTrashItemInfo)),
            this, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)));

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::restoreDTrashItems(const DTrashItemInfoList& items)
{
    ActionJobCollection collection;

    RestoreDTrashItemsJob* const j = new RestoreDTrashItemsJob(items);

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::deleteDTrashItems(const DTrashItemInfoList& items)
{
    ActionJobCollection collection;

    DeleteDTrashItemsJob* const j = new DeleteDTrashItemsJob(items);

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

bool IOJobsThread::isCanceled() const
{
    return d->isCanceled;
}

bool IOJobsThread::hasErrors() const
{
    return !d->errorsList.isEmpty();
}

QStringList& IOJobsThread::errorsList() const
{
    return d->errorsList;
}

IOJobData* IOJobsThread::jobData() const
{
    return d->jobData;
}

void IOJobsThread::connectOneJob(IOJob* const j)
{
    connect(j, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));

    connect(j, SIGNAL(signalDone()),
            this, SLOT(slotOneJobFinished()));

    connect(j, SIGNAL(signalOneProccessed(QUrl)),
            this, SIGNAL(signalOneProccessed(QUrl)));
}

void IOJobsThread::slotOneJobFinished()
{
    d->jobsCount--;

    if (d->jobsCount == 0)
    {
        emit finished();
        qCDebug(DIGIKAM_IOJOB_LOG) << "Thread Finished";
    }
}

void IOJobsThread::slotError(const QString& errString)
{
    d->errorsList.append(errString);
}

void IOJobsThread::slotCancel()
{
    d->isCanceled = true;
    ActionThreadBase::cancel();
}

} // namespace Digikam
