/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "actionthread.moc"

// KDE includes

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <threadweaver/JobCollection.h>
#include <solid/device.h>

// Local includes

#include "config-digikam.h"
#include "task.h"

using namespace Solid;

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
    : RActionThreadBase(parent), d(new Private)
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
        setMaximumNumberOfThreads(qMax(Device::listFromType(DeviceInterface::Processor).count(), 1));
    }
}

void ActionThread::processQueueItems(const QList<AssignedBatchTools>& items)
{
    JobCollection* const collection = new JobCollection();

    for(int i=0; i < items.size(); i++)
    {
        Task* const t = new Task();
        t->setSettings(d->settings);
        t->setItem(items.at(i));

        connect(t, SIGNAL(signalStarting(Digikam::ActionData)),
                this, SIGNAL(signalStarting(Digikam::ActionData)));

        connect(t, SIGNAL(signalFinished(Digikam::ActionData)),
                this, SIGNAL(signalFinished(Digikam::ActionData)));

        connect(this, SIGNAL(signalCancelTask()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection->addJob(t);
    }

    appendJob(collection);
}

void ActionThread::cancel()
{
    if (isRunning())
        emit signalCancelTask();

    RActionThreadBase::cancel();
}

void ActionThread::slotThreadFinished()
{
    if (isEmpty())
    {
        kDebug() << "List of Pending Jobs is empty";
        emit signalQueueProcessed();
    }
}

}  // namespace Digikam
