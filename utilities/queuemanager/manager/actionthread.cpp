/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include <config-digikam.h>
#include "dimg.h"

namespace Digikam
{

class ActionThread::ActionThreadPriv
{
public:

    ActionThreadPriv() :
        running(false),
        cancel(false),
        exifSetOrientation(true),
        tool(0)
    {
    }

    class Task
    {
    public:

        Task() {};

        AssignedBatchTools item;
    };

    bool           running;
    bool           cancel;
    bool           exifSetOrientation;

    QMutex         mutex;

    QWaitCondition condVar;

    QList<Task*>   todo;

    KUrl           workingUrl;

    BatchTool*     tool;

    DRawDecoding   rawDecodingSettings;
};

ActionThread::ActionThread(QObject* parent)
    : QThread(parent), d(new ActionThreadPriv)
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
    ActionThreadPriv::Task* t = new ActionThreadPriv::Task;
    t->item                   = item;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ActionThread::cancel()
{
    QMutexLocker lock(&d->mutex);
    d->cancel  = true;

    if (d->tool)
    {
        d->tool->cancel();
    }

    d->todo.clear();
    d->running = false;
    d->condVar.wakeAll();
}

void ActionThread::run()
{
    d->running = true;

    while (d->running)
    {
        ActionThreadPriv::Task* t = 0;
        {
            QMutexLocker lock(&d->mutex);

            if (!d->todo.isEmpty())
            {
                t = d->todo.takeFirst();
            }
            else
            {
                d->condVar.wait(&d->mutex);
            }
        }

        if (t)
        {
            ActionData ad1;
            ad1.fileUrl = t->item.m_itemUrl;
            ad1.status  = ActionData::BatchStarted;
            emit starting(ad1);

            // Loop with all batch tools operations to apply on item.

            d->cancel          = false;
            int        index   = 0;
            bool       success = false;
            KUrl       outUrl  = t->item.m_itemUrl;
            KUrl       inUrl;
            KUrl::List tmp2del;
            DImg       tmpImage;
            QString    errMsg;

            for (BatchToolMap::const_iterator it = t->item.m_toolsMap.constBegin();
                 !d->cancel && (it != t->item.m_toolsMap.constEnd()) ; ++it)
            {
                index                      = it.key();
                BatchToolSet set           = it.value();
                d->tool                    = set.tool;
                BatchToolSettings settings = set.settings;
                inUrl                      = outUrl;

                kDebug() << "Tool Index: " << index;

                ActionData ad2;
                ad2.fileUrl = t->item.m_itemUrl;
                ad2.status  = ActionData::TaskStarted;
                ad2.index   = index;
                emit finished(ad2);

                d->tool->setImageData(tmpImage);
                d->tool->setWorkingUrl(d->workingUrl);
                d->tool->setRawDecodingSettings(d->rawDecodingSettings);
                d->tool->setResetExifOrientationAllowed(d->exifSetOrientation);
                d->tool->setLastChainedTool(index == t->item.m_toolsMap.count());
                d->tool->setInputUrl(inUrl);
                d->tool->setSettings(settings);
                d->tool->setInputUrl(inUrl);
                d->tool->setOutputUrlFromInputUrl();

                outUrl   = d->tool->outputUrl();
                success  = d->tool->apply();
                tmpImage = d->tool->imageData();
                errMsg   = d->tool->errorDescription();
                tmp2del.append(outUrl);
                d->tool  = 0;

                if (success && !d->cancel)
                {
                    ActionData ad3;
                    ad3.fileUrl = t->item.m_itemUrl;
                    ad3.status  = ActionData::TaskDone;
                    ad3.index   = index;
                    emit finished(ad3);
                }
                else if (d->cancel)
                {
                    ActionData ad4;
                    ad4.fileUrl = t->item.m_itemUrl;
                    ad4.status  = ActionData::TaskCanceled;
                    ad4.index   = index;
                    emit finished(ad4);

                    ActionData ad5;
                    ad5.fileUrl = t->item.m_itemUrl;
                    ad5.status  = ActionData::BatchCanceled;
                    emit finished(ad5);

                    break;
                }
                else
                {
                    ActionData ad4;
                    ad4.fileUrl = t->item.m_itemUrl;
                    ad4.status  = ActionData::TaskFailed;
                    ad4.index   = index;
                    emit finished(ad4);

                    ActionData ad5;
                    ad5.fileUrl = t->item.m_itemUrl;
                    ad5.status  = ActionData::BatchFailed;

                    if (!errMsg.isEmpty())
                    {
                        ad5.message = errMsg;
                    }

                    emit finished(ad5);

                    break;
                }
            }

            if (success && !d->cancel)
            {
                // if success, we don't remove last output tmp url.
                tmp2del.removeAll(outUrl);

                ActionData ad6;
                ad6.fileUrl = t->item.m_itemUrl;
                ad6.destUrl = outUrl;
                ad6.status  = ActionData::BatchDone;
                emit finished(ad6);
            }

            // Clean up all tmp url.

            for (KUrl::List::const_iterator it = tmp2del.constBegin(); it != tmp2del.constEnd() ; ++it)
            {
                unlink(QFile::encodeName((*it).toLocalFile()));
            }
        }

        delete t;
    }
}

}  // namespace Digikam
