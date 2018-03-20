/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : Manager for creating and starting IO jobs threads
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "iojobsmanager.h"

// Local includes

#include "iojobdata.h"

namespace Digikam
{

class IOJobsManagerCreator
{
public:

    IOJobsManager object;
};

Q_GLOBAL_STATIC(IOJobsManagerCreator, creator)

// ----------------------------------------------

IOJobsManager::IOJobsManager()
{
}

IOJobsManager* IOJobsManager::instance()
{
    return& creator->object;
}

IOJobsThread* IOJobsManager::startCopy(IOJobData* const data)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->copy(data);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startMove(IOJobData* const data)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->move(data);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startDelete(IOJobData* const data)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->deleteFiles(data);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startRenameFile(IOJobData* const data)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->renameFile(data);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startDTrashItemsListingForCollection(const QString& collectionPath)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->listDTrashItems(collectionPath);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startRestoringDTrashItems(const DTrashItemInfoList& trashItemsList)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->restoreDTrashItems(trashItemsList);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startDeletingDTrashItems(const DTrashItemInfoList& trashItemsList)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->deleteDTrashItems(trashItemsList);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

} // namespace Digikam
