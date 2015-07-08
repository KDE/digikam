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
#include "iojobsthread.h"
#include "imageinfo.h"

namespace Digikam
{

class IOJobsManager : public QObject
{

public:
    IOJobsManager();

    static IOJobsManager *instance();

    IOJobsThread *startCopy(const QList<QUrl> &srcsList, const QUrl &destAlbum);
    IOJobsThread *startMove(const QList<QUrl> &srcsList, const QUrl &destAlbum);
    IOJobsThread *startDelete(const QList<QUrl> &filesToDelete, bool useTrash = true);
    IOJobsThread *startRenameFile(const QUrl &srcToRename, const QUrl &newUrl);

private:

    friend class IOJobsManagerCreator;
};

} // namespace Digikam

#endif // FILESYSTEMJOBSMANAGER_H
