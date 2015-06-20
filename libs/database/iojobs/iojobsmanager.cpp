/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : Manager for creating and starting IO jobs threads
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

IOJobsManager *IOJobsManager::instance()
{
    return &creator->object;
}

IOJobsThread *IOJobsManager::startCopyJob(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->copyPAlbum(srcAlbum, destAlbum, opType);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startCopyJob(const QList<QUrl> &srcsList, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->copyFiles(srcsList, destAlbum, opType);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startDeleteJob(const PAlbum *albumToDelete, bool useTrash)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->deletePAlbum(albumToDelete, useTrash);
    thread->start();

    return thread;
}

IOJobsThread *IOJobsManager::startDeleteJob(const QList<ImageInfo> &filesToDelete, bool useTrash)
{
    IOJobsThread *thread = new IOJobsThread(this);
    thread->deleteFiles(filesToDelete, useTrash);
    thread->start();

    return thread;
}

} // namespace Digikam
