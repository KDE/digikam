/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Pankaj Kumar <me at panks dot me>
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

namespace Digikam
{

class ActionThread::ActionThreadPriv
{
public:

    ActionThreadPriv() :
        cancel(false),
        exifSetOrientation(true),
        createNewVersion(true),
        tool(0)
    {
    }

    bool         cancel;
    bool         exifSetOrientation;
    bool         createNewVersion;

    KUrl         workingUrl;

    BatchTool*   tool;

    DRawDecoding rawDecodingSettings;

    ActionData   ad;
};

Task::Task(QObject* const parent, const AssignedBatchTools& item, ActionThread::ActionThreadPriv* const d)
    : Job(parent)
{
    m_item = item;
    m_d    = d;
}

Task::~Task()
{
}

void Task::run()
{
    if(m_d->cancel)
    {
        return;
    }
    ActionData ad1;
    ad1.fileUrl = m_item.m_itemUrl;
    ad1.status  = ActionData::BatchStarted;
    emit signalStarting(ad1);

    // Loop with all batch tools operations to apply on item.

    m_d->cancel          = false;
    int        index   = 0;
    bool       success = false;
    KUrl       outUrl  = m_item.m_itemUrl;
    KUrl       inUrl;
    KUrl::List tmp2del;
    DImg       tmpImage;
    QString    errMsg;

    for (BatchToolMap::const_iterator it = m_item.m_toolsMap.constBegin();
         !m_d->cancel && (it != m_item.m_toolsMap.constEnd()) ; ++it)
    {
        index                      = it.key();
        BatchToolSet set           = it.value();
        m_d->tool                  = set.tool;
        BatchToolSettings settings = set.settings;
        inUrl                      = outUrl;

        kDebug() << "Tool Index: " << index;

        ActionData ad2;
        ad2.fileUrl = m_item.m_itemUrl;
        ad2.status  = ActionData::TaskStarted;
        ad2.index   = index;
        emit signalFinished(ad2);

        m_d->tool->setImageData(tmpImage);
        m_d->tool->setWorkingUrl(m_d->workingUrl);
        m_d->tool->setRawDecodingSettings(m_d->rawDecodingSettings);
        m_d->tool->setResetExifOrientationAllowed(m_d->exifSetOrientation);
        m_d->tool->setLastChainedTool(index == m_item.m_toolsMap.count());
        m_d->tool->setInputUrl(inUrl);
        m_d->tool->setSettings(settings);
        m_d->tool->setInputUrl(inUrl);
        m_d->tool->setOutputUrlFromInputUrl();
        m_d->tool->setBranchHistory(m_d->createNewVersion);

        outUrl   = m_d->tool->outputUrl();
        success  = m_d->tool->apply();
        tmpImage = m_d->tool->imageData();
        errMsg   = m_d->tool->errorDescription();
        tmp2del.append(outUrl);
        m_d->tool  = 0;

        if (success && !m_d->cancel)
        {
            ActionData ad3;
            ad3.fileUrl = m_item.m_itemUrl;
            ad3.status  = ActionData::TaskDone;
            ad3.index   = index;
            emit signalFinished(ad3);

        }
        else if (m_d->cancel)
        {
            ActionData ad4;
            ad4.fileUrl = m_item.m_itemUrl;
            ad4.status  = ActionData::TaskCanceled;
            ad4.index   = index;
            emit signalFinished(ad4);

            ActionData ad5;
            ad5.fileUrl = m_item.m_itemUrl;
            ad5.status  = ActionData::BatchCanceled;
            emit signalFinished(ad5);

            break;
        }
        else
        {
            ActionData ad4;
            ad4.fileUrl = m_item.m_itemUrl;
            ad4.status  = ActionData::TaskFailed;
            ad4.index   = index;
            emit signalFinished(ad4);

            ActionData ad5;
            ad5.fileUrl = m_item.m_itemUrl;
            ad5.status  = ActionData::BatchFailed;

            if (!errMsg.isEmpty())
            {
                ad5.message = errMsg;
            }

            emit signalFinished(ad5);

            break;
        }
    }

    if (success && !m_d->cancel)
    {
        // if success, we don't remove last output tmp url.
        tmp2del.removeAll(outUrl);

        ActionData ad6;
        ad6.fileUrl = m_item.m_itemUrl;
        ad6.destUrl = outUrl;
        ad6.status  = ActionData::BatchDone;
        emit signalFinished(ad6);
    }

    // Clean up all tmp url.

    for (KUrl::List::const_iterator it = tmp2del.constBegin(); it != tmp2del.constEnd() ; ++it)
    {
        unlink(QFile::encodeName((*it).toLocalFile()));
    }
}

// -------------------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : DActionThreadBase(parent), d(new ActionThreadPriv)
{
    qRegisterMetaType<ActionData>();
}

ActionThread::~ActionThread()
{
    cancel();

    wait();

    delete d;
}

void ActionThread::setWorkingUrl(const KUrl& workingUrl)
{
    d->workingUrl = workingUrl;
}

void ActionThread::setResetExifOrientationAllowed(bool set)
{
    d->exifSetOrientation = set;
}

void ActionThread::setRawDecodingSettings(const DRawDecoding& settings)
{
    d->rawDecodingSettings = settings;
}

void ActionThread::processFile(const AssignedBatchTools& item)
{
    JobCollection* collection = new JobCollection();
    Task* t                   = new Task(this, item, d);

    connect(t, SIGNAL(signalStarting(Digikam::ActionData)),
            this, SIGNAL(starting(Digikam::ActionData)));

    connect(t, SIGNAL(signalFinished(Digikam::ActionData)),
            this, SIGNAL(finished(Digikam::ActionData)));

    collection->addJob(t);
    appendJob(collection);
}

void ActionThread::cancel()
{
    d->cancel = true;
    DActionThreadBase::cancel();
}

}  // namespace Digikam
