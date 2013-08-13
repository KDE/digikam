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

void MaintenanceThread::processItems(const ImageInfoList& items, Mode mode, Settings set)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < items.size(); i++)
    {
        switch(mode)
        {
            case ThumbsGenerator:
                //TODO

                break;

            case FingerprintsGenerator:
                //TODO

                break;

            default:  // MetadataSynchronizer

                MetadataTask* const t                   = new MetadataTask();
                MetadataSynchronizer::SyncDirection dir = (MetadataSynchronizer::SyncDirection)set.value("SyncDirection", MetadataSynchronizer::WriteFromDatabaseToFile).toInt();
                bool tagsOnly = set.value("TagsOnly",false).toBool();
                t->setItem(items.at(i), dir);
                t->setTagsOnly(tagsOnly);

                connect(t, SIGNAL(signalFinished()),
                        this, SIGNAL(signalAdvance()));

                connect(this, SIGNAL(signalCanceled()),
                        t, SLOT(slotCancel()), Qt::QueuedConnection);

                collection->addJob(t);
                break;
        }
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
