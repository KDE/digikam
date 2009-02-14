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

// C ANSI includes.

extern "C"
{
#include <unistd.h>
}

// Qt includes.

#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace Digikam
{

class ActionThreadPriv
{
public:

    ActionThreadPriv()
    {
        running = false;
        cancel  = false;
    }

    class Task
    {
        public:

            Task(){};

            AssignedBatchTools item;
    };

    bool           running;
    bool           cancel;

    QMutex         mutex;

    QWaitCondition condVar;

    QList<Task*>   todo;

    KUrl           workingUrl;
};

ActionThread::ActionThread(QObject *parent)
            : QThread(parent), d(new ActionThreadPriv)
{
    qRegisterMetaType<ActionData>("ActionData");
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
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->running = false;
    d->condVar.wakeAll();
}

void ActionThread::run()
{
    d->cancel  = false;;
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

            int index    = 0;
            bool success = false;
            KUrl outUrl  = t->item.itemUrl;
            KUrl inUrl;
            KUrl::List tmp2del;

            for (BatchToolMap::const_iterator it = t->item.toolsMap.begin(); 
                 !d->cancel && (it != t->item.toolsMap.end()) ; ++it)
            {
                index                      = it.key();
                BatchToolSet set           = it.value();
                BatchTool *tool            = set.tool;
                BatchToolSettings settings = set.settings;
                inUrl                      = outUrl;

                kDebug(50003) << "Tool Index: " << index << endl;

                ActionData ad2;
                ad2.fileUrl = t->item.itemUrl;
                ad2.status  = ActionData::TaskStarted;
                ad2.index   = index;
                emit finished(ad2);

                tool->setCancelFlag(&d->cancel);
                tool->setWorkingUrl(d->workingUrl);
                tool->setInputUrl(inUrl);
                tool->setSettings(settings);
                tool->setInputUrl(inUrl);
                tool->setOutputUrlFromInputUrl();

                outUrl  = tool->outputUrl();
                success = tool->apply();
                tmp2del.append(outUrl);

                if (success)
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

            if (success)
            {
                // if success, we don't remove last ouput tmp url.
                tmp2del.removeAll(outUrl);

                ActionData ad6;
                ad6.fileUrl = t->item.itemUrl;
                ad6.destUrl = outUrl;
                ad6.status  = ActionData::BatchDone;
                emit finished(ad6);
            }

            // Clean up all tmp url.

            for (KUrl::List::const_iterator it = tmp2del.begin(); it != tmp2del.end() ; ++it)
            {
                unlink(QFile::encodeName((*it).path()));
            }

        }

        delete t;
    }
}

}  // namespace Digikam
