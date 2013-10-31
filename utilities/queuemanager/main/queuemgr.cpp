/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QMessageBox>

// KDE includes

#include <kdebug.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes


#include "actionthread.h"
#include "queuesettings.h"
#include "batchtoolutils.h"
#include <actions.h>
#include "etrunner.h"
#include "queuemgr.h"

namespace Digikam
{
    
static const char* first_c = "first";
    
class QueueMgr::Private
{

public:

    Private() : runner(0)
    {
    }
    
    QScopedPointer<ActionThread>thread;
    QueueSettings               settings;
    QList<AssignedBatchTools>   tools4Items;
    QList<KUrl>                 processedImages;
    QScopedPointer<ETRunner>    runner;
};

QueueMgr::QueueMgr(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

QueueMgr::~QueueMgr()
{
    cancel();
}

bool QueueMgr::setup(const QueueSettings& settings, const QList<AssignedBatchTools>& tools)
{
    if (d->thread || d->runner)
    {
        return false;
    }
        
    d->processedImages.clear();
    d->settings = settings;
    d->tools4Items = tools;

    d->thread.reset(new ActionThread(this));
    d->thread->setProperty(first_c, true);
    
    d->thread->setSettings(d->settings);
    d->thread->processQueueItems(d->tools4Items);
    
    connect(d->thread.data(), SIGNAL(signalStarting(Digikam::ActionData)),
            this, SLOT(slotStartAction(Digikam::ActionData)));

    connect(d->thread.data(), SIGNAL(signalFinished(Digikam::ActionData)),
            this, SLOT(slotFinishedAction(Digikam::ActionData)));

    connect(d->thread.data(), SIGNAL(signalQueueProcessed()),
            this, SLOT(slotQueueProcessed()));
    
    d->thread->start();
    return true;
}

void QueueMgr::cancel()
{
    if (d->thread)
    {
        d->thread->cancel();
    }
    
    if (d->runner)
    {
        d->runner->terminate();
    }
}

void QueueMgr::slotStartAction(const ActionData& ad)
{
    if (d->thread->property(first_c).toBool())
    {
        d->thread->setProperty(first_c, false);
        Q_EMIT started();
    }
    
    d->processedImages << ad.fileUrl;
    
    Q_EMIT action(ad);
}

void QueueMgr::slotFinishedAction(const ActionData& ad)
{
    if (ad.status == ActionData::BatchDone) 
    {
        d->processedImages.removeAll(ad.fileUrl);
        d->processedImages << ad.destUrl;
    }
    Q_EMIT action(ad);
}

void QueueMgr::slotQueueProcessed()
{
    d->thread.take()->deleteLater();
    
    if (d->settings.useETools)
    {
        d->runner.reset(new ETRunner(this, d->settings.externalTool, d->processedImages));
        connect(d->runner.data(), SIGNAL(finished()), this, SLOT(runnerFinished()));
        connect(d->runner.data(), SIGNAL(error(QString,QString)), this, SLOT(runnerError(QString,QString)));
        d->runner->run();        
    }
    else
    {
        Q_EMIT finished();
    }
}

void QueueMgr::runnerFinished()
{
    d->runner.take()->deleteLater();
    Q_EMIT finished();
}

void QueueMgr::runnerError(const QString& title, const QString& text)
{
    QMessageBox::critical(0, title, text);
}



}  // namespace Digikam
