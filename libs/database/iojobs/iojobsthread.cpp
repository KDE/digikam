/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
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

#include "iojobsthread.h"

#include "iojob.h"
#include "imageinfo.h"

namespace Digikam
{

IOJobsThread::IOJobsThread(QObject *const parent)
    : RActionThreadBase(parent)
{
}

void IOJobsThread::copyPAlbum(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    RJobCollection collection;
    CopyAlbumJob *j = new CopyAlbumJob(srcAlbum, destAlbum, opType);

    // TODO: Create a connection here for progress or whatever feedback

    collection.insert(j, 0);
    appendJobs(collection);
}

void IOJobsThread::copyFiles(const QList<QUrl> &srcFiles, const PAlbum *destAlbum, const CopyJob::OperationType opType)
{
    RJobCollection collection;

    foreach (const QUrl &url, srcFiles)
    {
        CopyFileJob *j = new CopyFileJob(url, destAlbum, opType);

        // TODO: Create a connection here for progress or whatever feedback

        collection.insert(j, 0);
    }

    appendJobs(collection);
}

void IOJobsThread::deletePAlbum(const PAlbum *albumToDelete, bool isPermanentDeletion)
{
    RJobCollection collection;
    DeleteAlbumJob *j = new DeleteAlbumJob(albumToDelete, isPermanentDeletion);

    // TODO: Create a connection here for progress or whatever feedback

    collection.insert(j, 0);
    appendJobs(collection);
}

void IOJobsThread::deleteFiles(const QList<ImageInfo> &srcsToDelete, bool useTrash)
{
    RJobCollection collection;

    foreach (const ImageInfo &info, srcsToDelete)
    {
        DeleteFileJob *j = new DeleteFileJob(info, useTrash);

        // TODO: Create a connection here for progress or whatever feedback

        collection.insert(j, 0);
    }

    appendJobs(collection);
}

} // namespace Digikam
