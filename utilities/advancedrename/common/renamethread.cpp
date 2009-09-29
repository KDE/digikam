/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : renaming thread
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "renamethread.h"
#include "renamethread.moc"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

class RenameThreadPriv
{
public:

    RenameThreadPriv()
    {
        running = false;
    }

    bool                               running;

    QMutex                             mutex;
    QWaitCondition                     condVar;

    AdvancedRenameDialog::NewNamesList todo;
};

RenameThread::RenameThread(QObject* parent)
            : QThread(parent), d(new RenameThreadPriv)
{
}

RenameThread::~RenameThread()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d;
}

void RenameThread::addNewNames(const AdvancedRenameDialog::NewNamesList& newNames)
{
    QMutexLocker lock(&d->mutex);
    d->todo << newNames;
    d->condVar.wakeAll();
}

void RenameThread::cancel()
{
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->running = false;
    d->condVar.wakeAll();
}

void RenameThread::run()
{
    d->running = true;

    while (d->running)
    {
        AdvancedRenameDialog::NewNameInfo info;
        {
            QMutexLocker lock(&d->mutex);
            if (!d->todo.isEmpty())
            {
                info = d->todo.takeFirst();
            }
            else
            {
                d->condVar.wait(&d->mutex);
            }
        }

        if (!info.first.isNull() && !info.second.isEmpty())
        {
            emit renameFile(info.first, info.second);
            d->condVar.wait(&d->mutex);
        }
    }
}

void RenameThread::processNext()
{
    d->condVar.wakeAll();
}

}  // namespace Digikam
