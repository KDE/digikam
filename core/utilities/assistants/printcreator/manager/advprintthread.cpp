/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintthread.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

AdvPrintThread::AdvPrintThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

AdvPrintThread::~AdvPrintThread()
{
    cancel();
    wait();
}

void AdvPrintThread::preparePrint(AdvPrintSettings* const settings, int sizeIndex)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PREPAREPRINT,
                                             QSize(),
                                             sizeIndex);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalDone(bool)),
            this, SIGNAL(signalDone(bool)));

    connect(t, SIGNAL(signalMessage(QString, bool)),
            this, SIGNAL(signalMessage(QString, bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void AdvPrintThread::print(AdvPrintSettings* const settings)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PRINT);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalDone(bool)),
            this, SIGNAL(signalDone(bool)));

    connect(t, SIGNAL(signalMessage(QString, bool)),
            this, SIGNAL(signalMessage(QString, bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void AdvPrintThread::preview(AdvPrintSettings* const settings, const QSize& size)
{
    ActionJobCollection collection;

    AdvPrintTask* const t = new AdvPrintTask(settings,
                                             AdvPrintTask::PREVIEW,
                                             size);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalDone(bool)),
            this, SIGNAL(signalDone(bool)));

    connect(t, SIGNAL(signalMessage(QString, bool)),
            this, SIGNAL(signalMessage(QString, bool)));

    connect(t, SIGNAL(signalPreview(QImage)),
            this, SIGNAL(signalPreview(QImage)));

    collection.insert(t, 0);

    appendJobs(collection);
}

} // namespace Digikam
