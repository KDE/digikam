/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// Qt includes

#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>

// Local includes

#include "config-digikam.h"
#include "dimg.h"
#include "dimg.h"

namespace Digikam
{

class TaskSettings
{
public:

    explicit TaskSettings()
    {
        exifSetOrientation = true;
        createNewVersion   = true;
    }

    bool         exifSetOrientation;
    bool         createNewVersion;

    KUrl         workingUrl;

    DRawDecoding rawDecodingSettings;
};

// ---------------------------------------------------------------------------------------------------------

class Task::Private
{
public:

    Private()
    {
        cancel = false;
        tool   = 0;
    }

    bool               cancel;

    TaskSettings       settings;
    BatchTool*         tool;
    AssignedBatchTools item;
};

Task::Task()
    : Job(0), d(new Private)
{
}

Task::~Task()
{
    slotCancel();
    delete d;
}

void Task::setSettings(const TaskSettings& settings)
{
    d->settings = settings;
}

void Task::setItem(const AssignedBatchTools& item)
{
    d->item = item;

    // For each tools assigned, create a dedicated instance for the thread to have a safe running between jobs.
    // NOTE: ad BatchTool include settings widget data, it cannot be cloned in thread as well, but in main thread.

    for (BatchToolMap::iterator it = d->item.m_toolsMap.begin();
         it != d->item.m_toolsMap.end() ; ++it)
    {
        BatchTool* tool = it.value().tool->clone(this);
        it.value().tool = tool;
    }
}

void Task::slotCancel()
{
    d->cancel = true;

    if (d->tool)
        d->tool->cancel();
}

void Task::run()
{
    if(d->cancel)
    {
        return;
    }

    ActionData ad1;
    ad1.fileUrl = d->item.m_itemUrl;
    ad1.status  = ActionData::BatchStarted;
    emit signalStarting(ad1);

    // Loop with all batch tools operations to apply on item.

    int        index   = 0;
    bool       success = false;
    KUrl       outUrl  = d->item.m_itemUrl;
    KUrl       inUrl;
    KUrl::List tmp2del;
    DImg       tmpImage;
    QString    errMsg;

    for (BatchToolMap::const_iterator it = d->item.m_toolsMap.constBegin();
         !d->cancel && (it != d->item.m_toolsMap.constEnd()) ; ++it)
    {
        index                      = it.key();
        BatchToolSet set           = it.value();
        d->tool                    = set.tool;
        BatchToolSettings settings = set.settings;
        inUrl                      = outUrl;

        kDebug() << "Tool Index: " << index;

        ActionData ad2;
        ad2.fileUrl = d->item.m_itemUrl;
        ad2.status  = ActionData::TaskStarted;
        ad2.index   = index;
        emit signalFinished(ad2);

        d->tool->setImageData(tmpImage);
        d->tool->setWorkingUrl(d->settings.workingUrl);
        d->tool->setRawDecodingSettings(d->settings.rawDecodingSettings);
        d->tool->setResetExifOrientationAllowed(d->settings.exifSetOrientation);
        d->tool->setLastChainedTool(index == d->item.m_toolsMap.count());
        d->tool->setInputUrl(inUrl);
        d->tool->setSettings(settings);
        d->tool->setInputUrl(inUrl);
        d->tool->setOutputUrlFromInputUrl();
        d->tool->setBranchHistory(d->settings.createNewVersion);

        outUrl    = d->tool->outputUrl();
        success   = d->tool->apply();
        tmpImage  = d->tool->imageData();
        errMsg    = d->tool->errorDescription();
        tmp2del.append(outUrl);

        if (success && !d->cancel)
        {
            ActionData ad3;
            ad3.fileUrl = d->item.m_itemUrl;
            ad3.status  = ActionData::TaskDone;
            ad3.index   = index;
            emit signalFinished(ad3);

        }
        else if (d->cancel)
        {
            ActionData ad4;
            ad4.fileUrl = d->item.m_itemUrl;
            ad4.status  = ActionData::TaskCanceled;
            ad4.index   = index;
            emit signalFinished(ad4);

            ActionData ad5;
            ad5.fileUrl = d->item.m_itemUrl;
            ad5.status  = ActionData::BatchCanceled;
            emit signalFinished(ad5);

            break;
        }
        else
        {
            ActionData ad4;
            ad4.fileUrl = d->item.m_itemUrl;
            ad4.status  = ActionData::TaskFailed;
            ad4.index   = index;
            emit signalFinished(ad4);

            ActionData ad5;
            ad5.fileUrl = d->item.m_itemUrl;
            ad5.status  = ActionData::BatchFailed;

            if (!errMsg.isEmpty())
            {
                ad5.message = errMsg;
            }

            emit signalFinished(ad5);

            break;
        }
    }

    if (success && !d->cancel)
    {
        // if success, we don't remove last output tmp url.
        tmp2del.removeAll(outUrl);

        ActionData ad6;
        ad6.fileUrl = d->item.m_itemUrl;
        ad6.destUrl = outUrl;
        ad6.status  = ActionData::BatchDone;
        emit signalFinished(ad6);
    }

    // Clean up all tmp url.

    foreach (KUrl url, tmp2del)
    {
        unlink(QFile::encodeName(url.toLocalFile()));
    }
}

// -------------------------------------------------------------------------------------------------

class ActionThread::Private
{
public:

    Private()
    {
    }

    TaskSettings settings;
};

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

void ActionThread::setWorkingUrl(const KUrl& url)
{
    d->settings.workingUrl = url;
}

void ActionThread::setResetExifOrientationAllowed(bool b)
{
    d->settings.exifSetOrientation = b;
}

void ActionThread::setRawDecodingSettings(const DRawDecoding& prm)
{
    d->settings.rawDecodingSettings = prm;
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
