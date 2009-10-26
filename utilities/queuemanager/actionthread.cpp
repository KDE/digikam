/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "debug.h"
#include "dimg.h"

namespace Digikam
{

class ActionThreadPriv
{
public:

    ActionThreadPriv()
    {
        tool               = 0;
        running            = false;
        cancel             = false;
        exifSetOrientation = true;
    }

    class Task
    {
        public:

            Task(){};

            AssignedBatchTools item;
    };

    bool            running;
    bool            cancel;
    bool            exifSetOrientation;

    QMutex          mutex;

    QWaitCondition  condVar;

    QList<Task*>    todo;

    KUrl            workingUrl;

    BatchTool      *tool;
};

ActionThread::ActionThread(QObject *parent)
            : QThread(parent), d(new ActionThreadPriv)
{
    qRegisterMetaType<ActionData>();
}

ActionThread::~ActionThread()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d;
}

void ActionThread::setWorkingUrl(const KUrl& workingUrl)
{
    d->workingUrl = workingUrl;
}

void ActionThread::setExifSetOrientation(bool set)
{
    d->exifSetOrientation = set;
}

void ActionThread::processFile(const AssignedBatchTools& item)
{
    ActionThreadPriv::Task *t = new ActionThreadPriv::Task;
    t->item                   = item;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ActionThread::cancel()
{
    d->cancel  = true;
    if (d->tool) d->tool->cancel();

    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->running = false;
    d->condVar.wakeAll();
}

void ActionThread::run()
{
    d->running = true;

    while (d->running)
    {
        ActionThreadPriv::Task *t = 0;
        {
            QMutexLocker lock(&d->mutex);
            if (!d->todo.isEmpty())
                t = d->todo.takeFirst();
            else
                d->condVar.wait(&d->mutex);
        }

        if (t)
        {
            ActionData ad1;
            ad1.fileUrl = t->item.itemUrl;
            ad1.status  = ActionData::BatchStarted;
            emit starting(ad1);

            // Loop with all batch tools operations to apply on item.

            d->cancel    = false;
            int index    = 0;
            bool success = false;
            KUrl outUrl  = t->item.itemUrl;
            KUrl inUrl;
            KUrl::List tmp2del;
            DImg tmpImage;

            for (BatchToolMap::const_iterator it = t->item.toolsMap.constBegin();
                 !d->cancel && (it != t->item.toolsMap.constEnd()) ; ++it)
            {
                index                      = it.key();
                BatchToolSet set           = it.value();
                d->tool                    = set.tool;
                BatchToolSettings settings = set.settings;
                inUrl                      = outUrl;

                kDebug(digiKamAreaCode) << "Tool Index: " << index;

                ActionData ad2;
                ad2.fileUrl = t->item.itemUrl;
                ad2.status  = ActionData::TaskStarted;
                ad2.index   = index;
                emit finished(ad2);

                d->tool->setImageData(tmpImage);
                d->tool->setWorkingUrl(d->workingUrl);
                d->tool->setExifSetOrientation(d->exifSetOrientation);
                d->tool->setLastChainedTool(index == t->item.toolsMap.count());
                d->tool->setInputUrl(inUrl);
                d->tool->setSettings(settings);
                d->tool->setInputUrl(inUrl);
                d->tool->setOutputUrlFromInputUrl();

                outUrl   = d->tool->outputUrl();
                success  = d->tool->apply();
                tmpImage = d->tool->imageData();
                tmp2del.append(outUrl);
                d->tool  = 0;

                if (success && !d->cancel)
                {
                    ActionData ad3;
                    ad3.fileUrl = t->item.itemUrl;
                    ad3.status  = ActionData::TaskDone;
                    ad3.index   = index;
                    emit finished(ad3);
                }
                else
                {
                    ActionData ad4;
                    ad4.fileUrl = t->item.itemUrl;
                    ad4.status  = ActionData::TaskFailed;
                    ad4.index   = index;
                    emit finished(ad4);

                    ActionData ad5;
                    ad5.fileUrl = t->item.itemUrl;
                    ad5.status  = ActionData::BatchFailed;
                    emit finished(ad5);

                    break;
                }
            }

            if (success && !d->cancel)
            {
                // if success, we don't remove last output tmp url.
                tmp2del.removeAll(outUrl);

                ActionData ad6;
                ad6.fileUrl = t->item.itemUrl;
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
