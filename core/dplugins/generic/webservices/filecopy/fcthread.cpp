/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy thread.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2019      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "fcthread.h"

// Local includes

#include "dinfointerface.h"
#include "fctask.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCThread::Private
{

public:

    explicit Private()
    {
    }

    QList<QUrl> itemsList;
};

FCThread::FCThread(QObject* const parent)
    : ActionThreadBase(parent),
      d(new Private)
{
}

FCThread::~FCThread()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d;
}

void FCThread::setItemUrlList(const QList<QUrl>& itemsList, const QUrl& dstUrl)
{
    d->itemsList = itemsList;
    ActionJobCollection collection;

    foreach (const QUrl& srcUrl, d->itemsList)
    {
        FCTask* const t = new FCTask(srcUrl, dstUrl);

        connect(t, SIGNAL(signalUrlProcessed(QUrl,QUrl)),
                this, SIGNAL(signalUrlProcessed(QUrl,QUrl)));

        connect(this, SIGNAL(signalCancelTask()),
                t, SLOT(cancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void FCThread::cancel()
{
    if (isRunning())
        emit signalCancelTask();

    ActionThreadBase::cancel();
}

}  // namespace DigikamGenericFileCopyPlugin
