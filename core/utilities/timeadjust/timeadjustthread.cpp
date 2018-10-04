/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust thread.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "timeadjustthread.h"

// Local includes

#include "dinfointerface.h"
#include "timeadjusttask.h"

namespace Digikam
{

class Q_DECL_HIDDEN TimeAdjustThread::Private
{

public:

    explicit Private()
    {
    }

    // Settings from GUI.
    TimeAdjustContainer   settings;

    // Map of item urls and Updated Timestamps.
    QMap<QUrl, QDateTime> itemsMap;
};


TimeAdjustThread::TimeAdjustThread(QObject* const parent)
    : ActionThreadBase(parent),
      d(new Private)
{
}

TimeAdjustThread::~TimeAdjustThread()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d;
}

void TimeAdjustThread::setUpdatedDates(const QMap<QUrl, QDateTime>& itemsMap)
{
    d->itemsMap = itemsMap;
    ActionJobCollection collection;

    foreach (const QUrl& url, d->itemsMap.keys())
    {
        TimeAdjustTask* const t = new TimeAdjustTask(url);
        t->setSettings(d->settings);
        t->setItemsMap(d->itemsMap);

        connect(t, SIGNAL(signalProcessStarted(QUrl)),
                this, SIGNAL(signalProcessStarted(QUrl)));

        connect(t, SIGNAL(signalProcessEnded(QUrl,int)),
                this, SIGNAL(signalProcessEnded(QUrl,int)));

        connect(t, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)),
                this, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)));

        connect(this, SIGNAL(signalCancelTask()),
                t, SLOT(cancel()), Qt::QueuedConnection);

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void TimeAdjustThread::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;
}

void TimeAdjustThread::cancel()
{
    if (isRunning())
        emit signalCancelTask();

    ActionThreadBase::cancel();
}

}  // namespace Digikam
