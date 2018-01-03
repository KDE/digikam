/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Pankaj Kumar <me at panks dot me>
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

#include "actionthread.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "collectionscanner.h"
#include "task.h"

namespace Digikam
{

class ActionThread::Private
{
public:

    Private()
    {
    }

    QueueSettings settings;
};

// --------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : ActionThreadBase(parent), d(new Private)
{
    qRegisterMetaType<ActionData>();

    connect(this, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));
}

ActionThread::~ActionThread()
{
    cancel();

    wait();

    delete d;
}

void ActionThread::setSettings(const QueueSettings& settings)
{
    d->settings = settings;

    if (!d->settings.useMultiCoreCPU)
    {
        setMaximumNumberOfThreads(1);
    }
    else
    {
        defaultMaximumNumberOfThreads();
    }
}

void ActionThread::processQueueItems(const QList<AssignedBatchTools>& items)
{
    ActionJobCollection collection;

    for (int i = 0 ; i < items.size() ; i++)
    {
        Task* const t = new Task();
        t->setSettings(d->settings);
        t->setItem(items.at(i));

        connect(t, SIGNAL(signalStarting(Digikam::ActionData)),
                this, SIGNAL(signalStarting(Digikam::ActionData)));

        connect(t, SIGNAL(signalFinished(Digikam::ActionData)),
                this, SLOT(slotUpdateImageInfo(Digikam::ActionData)),
                Qt::BlockingQueuedConnection);

        connect(this, SIGNAL(signalCancelTask()),
                t, SLOT(slotCancel()),
                Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void ActionThread::cancel()
{
    if (isRunning())
        emit signalCancelTask();

    ActionThreadBase::cancel();
}

void ActionThread::slotUpdateImageInfo(const Digikam::ActionData& ad)
{
    if (ad.status == ActionData::BatchDone)
    {
        CollectionScanner scanner;
        ImageInfo source = ImageInfo::fromUrl(ad.fileUrl);
        qlonglong id     = scanner.scanFile(ad.destUrl.toLocalFile(), CollectionScanner::NormalScan);
        ImageInfo info(id);
        // Copy the digiKam attributes from original file to the new file
        CollectionScanner::copyFileProperties(source, info);
        // Read again new file that the database is up to date
        scanner.scanFile(info, CollectionScanner::Rescan);
    }

    emit signalFinished(ad);
}

void ActionThread::slotThreadFinished()
{
    if (isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "List of Pending Jobs is empty";
        emit signalQueueProcessed();
    }
}

}  // namespace Digikam
