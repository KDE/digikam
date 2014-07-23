/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for maintenance tools.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "maintenancethread.moc"

// KDE includes

#include <kdebug.h>
#include <threadweaver/JobCollection.h>
#include <solid/device.h>

// Local includes

#include "metadatatask.h"
#include "thumbstask.h"
#include "fingerprintstask.h"
#include "imagequalitytask.h"
#include "imagequalitysettings.h"

using namespace Solid;

namespace Digikam
{


MaintenanceThread::MaintenanceThread(QObject* const parent)
    : RActionThreadBase(parent)
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
        setMaximumNumberOfThreads(qMax(Device::listFromType(DeviceInterface::Processor).count(), 1));
    }
}

void MaintenanceThread::syncMetadata(const ImageInfoList& items, MetadataSynchronizer::SyncDirection dir, bool tagsOnly)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < items.size(); i++)
    {
        MetadataTask* const t = new MetadataTask();
        t->setTagsOnly(tagsOnly);
        t->setItem(items.at(i), dir);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void MaintenanceThread::generateThumbs(const QStringList& paths)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < paths.size(); i++)
    {
        ThumbsTask* const t = new ThumbsTask();
        t->setItem(paths.at(i));

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void MaintenanceThread::generateFingerprints(const QStringList& paths)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < paths.size(); i++)
    {
        FingerprintsTask* const t = new FingerprintsTask();
        t->setItem(paths.at(i));

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void MaintenanceThread::sortByImageQuality(const QStringList& paths, const ImageQualitySettings& quality)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < paths.size(); i++)
    {
        ImageQualityTask* const t = new ImageQualityTask();
        t->setItem(paths.at(i), quality);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void MaintenanceThread::cancel()
{
    if (isRunning())
        emit signalCanceled();

    RActionThreadBase::cancel();
}

void MaintenanceThread::slotThreadFinished()
{
    if (isEmpty())
    {
        kDebug() << "List of Pending Jobs is empty";
        emit signalCompleted();
    }
}

}  // namespace Digikam
