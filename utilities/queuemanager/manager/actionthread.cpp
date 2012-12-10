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

#include <kde_file.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>

// Local includes

#include "config-digikam.h"
#include "dimg.h"
#include "dmetadata.h"
#include "queuesettings.h"
#include "imageinfo.h"
#include "fileactionmngr.h"

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

    bool          exifSetOrientation;
    bool          createNewVersion;

    QueueSettings queuePrm;
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
    // NOTE: BatchTool include settings widget data, it cannot be cloned in thread as well, but in main thread.

    for (BatchToolMap::iterator it = d->item.m_toolsMap.begin();
         it != d->item.m_toolsMap.end() ; ++it)
    {
        BatchTool* const tool = it.value().tool->clone(this);
        it.value().tool       = tool;
    }
}

void Task::slotCancel()
{
    d->cancel = true;

    if (d->tool)
        d->tool->cancel();
}

void Task::emitActionData(ActionData::ActionStatus st, const QString& mess, const KUrl& dest)
{
    ActionData ad;
    ad.fileUrl = d->item.m_itemUrl;
    ad.status  = st;
    ad.message = mess;
    ad.destUrl = dest;
    emit signalFinished(ad);
}

void Task::run()
{
    if(d->cancel)
    {
        return;
    }

    emitActionData(ActionData::BatchStarted);

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

        emitActionData(ActionData::TaskStarted);

        d->tool->setImageData(tmpImage);
        d->tool->setWorkingUrl(d->settings.queuePrm.workingUrl);
        d->tool->setRawDecodingSettings(d->settings.queuePrm.rawDecodingSettings);
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
            emitActionData(ActionData::TaskDone);
        }
        else if (d->cancel)
        {
            emitActionData(ActionData::TaskCanceled);
            emitActionData(ActionData::BatchCanceled);
            break;
        }
        else
        {
            emitActionData(ActionData::TaskFailed);
            emitActionData(ActionData::BatchFailed, errMsg);
            break;
        }
    }

    if (success && !d->cancel)
    {
        // if success, we don't remove last output tmp url.
        tmp2del.removeAll(outUrl);
    }

    // Clean up all tmp url.

    foreach (KUrl url, tmp2del)
    {
        unlink(QFile::encodeName(url.toLocalFile()));
    }

    // Move processed temp file to target

    KUrl dest = d->settings.queuePrm.workingUrl;
    dest.setFileName(d->item.m_destFileName);
    QString renameMess;
    QFileInfo fi(dest.toLocalFile());

    if (fi.exists())
    {
        if (d->settings.queuePrm.conflictRule != QueueSettings::OVERWRITE)
        {
            int i          = 0;
            bool fileFound = false;

            do
            {
                QFileInfo nfi(dest.toLocalFile());

                if (!nfi.exists())
                {
                    fileFound = false;
                }
                else
                {
                    i++;
                    dest.setFileName(nfi.completeBaseName() + QString("_%1.").arg(i) + nfi.completeSuffix());
                    fileFound = true;
                }
            }
            while (fileFound);

            renameMess = i18n("renamed to %1...", dest.fileName());
        }
        else
        {
            renameMess = i18n("overwritten...");
        }
    }

    if (!dest.isEmpty())
    {
        if (DMetadata::hasSidecar(outUrl.toLocalFile()))
        {
            if (KDE::rename(DMetadata::sidecarPath(outUrl.toLocalFile()),
                            DMetadata::sidecarPath(dest.toLocalFile())) != 0)
            {
                emitActionData(ActionData::BatchFailed, i18n("Failed to rename sidecar file..."), dest);
            }
        }

        if (KDE::rename(outUrl.toLocalFile(), dest.toLocalFile()) != 0)
        {
            emitActionData(ActionData::BatchFailed, i18n("Failed to save file..."), dest);
        }
        else
        {
            // -- Now copy the digiKam attributes from original file to the new file ------------

            // ImageInfo must be tread-safe.
            ImageInfo source(d->item.m_itemUrl.toLocalFile());
            FileActionMngr::instance()->copyAttributes(source, dest.toLocalFile());

            emitActionData(ActionData::BatchDone, i18n("Item processed successfully (%1)", renameMess), dest);
        }
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

void ActionThread::setSettings(const QueueSettings& queuePrm)
{
    d->settings.queuePrm = queuePrm;
}

void ActionThread::setResetExifOrientationAllowed(bool b)
{
    d->settings.exifSetOrientation = b;
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
