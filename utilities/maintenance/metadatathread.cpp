/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for metadata synchronizer.
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

#include "metadatathread.moc"

// KDE includes

#include <kdebug.h>
#include <threadweaver/JobCollection.h>
#include <solid/device.h>

// Local includes

#include "metadatatask.h"

using namespace Solid;

namespace Digikam
{

MetadataThread::MetadataThread(QObject* const parent)
    : RActionThreadBase(parent), tagsOnly(false)
{
    connect(this, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));
}

MetadataThread::~MetadataThread()
{
    cancel();

    wait();
}

void MetadataThread::setUseMultiCore(const bool b)
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

void MetadataThread::setTagsOnly(bool value)
{
    this->tagsOnly = value;
}
void MetadataThread::processItems(const ImageInfoList& items, MetadataSynchronizer::SyncDirection dir)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < items.size(); i++)
    {
        MetadataTask* const t = new MetadataTask();
        t->setItem(items.at(i), dir);
        t->setTagsOnly(this->tagsOnly);

        connect(t, SIGNAL(signalFinished()),
                this, SIGNAL(signalAdvance()));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void MetadataThread::cancel()
{
    if (isRunning())
        emit signalCanceled();

    RActionThreadBase::cancel();
}

void MetadataThread::slotThreadFinished()
{
    if (isEmpty())
    {
        kDebug() << "List of Pending Jobs is empty";
        emit signalCompleted();
    }
}

}  // namespace Digikam
