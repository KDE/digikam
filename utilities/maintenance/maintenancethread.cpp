/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for maintenance tools.
 *
 * Copyright (C) 2013-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancethread.h"

// Local includes

#include "digikam_debug.h"
#include "metadatatask.h"
#include "thumbstask.h"
#include "fingerprintstask.h"
#include "imagequalitytask.h"
#include "imagequalitysettings.h"

namespace Digikam
{

MaintenanceThread::MaintenanceThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    connect(this, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));
}

MaintenanceThread::~MaintenanceThread()
{
    cancel();
    wait();
}

void MaintenanceThread::setUseMultiCore(const bool b)
{
    if (!b)
    {
        setMaximumNumberOfThreads(1);
    }
    else
    {
        defaultMaximumNumberOfThreads();
    }
}

void MaintenanceThread::syncMetadata(const ImageInfoList& items, MetadataSynchronizer::SyncDirection dir, bool tagsOnly)
{
    ActionJobCollection collection;

    for(int i = 0; i < items.size(); i++)
    {
        MetadataTask* const t = new MetadataTask();
        t->setTagsOnly(tagsOnly);
        t->setItem(items.at(i), dir);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void MaintenanceThread::generateThumbs(const QStringList& paths)
{
    ActionJobCollection collection;

    for(int i = 0; i < paths.size(); i++)
    {
        ThumbsTask* const t = new ThumbsTask();
        t->setItem(paths.at(i));

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void MaintenanceThread::generateFingerprints(const QStringList& paths)
{
    ActionJobCollection collection;

    for(int i = 0; i < paths.size(); i++)
    {
        FingerprintsTask* const t = new FingerprintsTask();
        t->setItem(paths.at(i));

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void MaintenanceThread::sortByImageQuality(const QStringList& paths, const ImageQualitySettings& quality)
{
    ActionJobCollection collection;

    for(int i = 0; i < paths.size(); i++)
    {
        ImageQualityTask* const t = new ImageQualityTask();
        t->setItem(paths.at(i), quality);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void MaintenanceThread::cancel()
{
    if (isRunning())
    {
        emit signalCanceled();
    }

    ActionThreadBase::cancel();
}

void MaintenanceThread::slotThreadFinished()
{
    if (isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "List of Pending Jobs is empty";
        emit signalCompleted();
    }
}

}  // namespace Digikam
