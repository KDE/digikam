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

#ifndef IOJOBSMANAGER_H
#define IOJOBSMANAGER_H

#include <QObject>
#include <QUrl>
#include "album.h"
#include "iojobsthread.h"
#include "imageinfo.h"

namespace Digikam
{

class IOJobsManager : public QObject
{

public:
    IOJobsManager();

    static IOJobsManager *instance();

    IOJobsThread *startCopyJob(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType);
    IOJobsThread *startCopyJob(const QList<QUrl> &srcsList, const PAlbum *destAlbum, const CopyJob::OperationType opType);

    IOJobsThread *startDeleteJob(const PAlbum *albumToDelete, bool useTrash = true);
    IOJobsThread *startDeleteJob(const QList<ImageInfo> &filesToDelete, bool useTrash = true);

// TODO
//    IOJobsThread *startRenameFileJob();
//    IOJobsThread *startRenameAlbumJob();

private:

    friend class FileSystemJobsManagerCreator;
};

} // namespace Digikam

#endif // FILESYSTEMJOBSMANAGER_H
