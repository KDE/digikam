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

// Qt includes

#include <QObject>
#include <QUrl>

// Local includes

#include "iojobsthread.h"
#include "imageinfo.h"

namespace Digikam
{

class IOJobsManager : public QObject
{

public:

    /**
     * @brief instance: returns the singleton of IO Jobs Manager
     * @return IOJobsManager global instance
     */
    static IOJobsManager* instance();

    /**
     * @brief startCopy: Starts a thread to copy items to destination
     * @param srcsList: Sources urls to copy
     * @param destAlbum: Destenation url of album folder
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startCopy(const QList<QUrl>& srcsList, const QUrl& destAlbum);

    /**
     * @brief startMove: Starts a thread to move items to destination
     * @param srcsList: Sources urls to move
     * @param destAlbum: Destenation url of album folder
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startMove(const QList<QUrl>& srcsList, const QUrl& destAlbum);

    /**
     * @brief startDelete: Starts a thread to delete items
     * @param filesToDelete: Sources urls to delete
     * @param useTrash: a flag to specify if the deletion is to trash or permanent
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startDelete(const QList<QUrl>& filesToDelete, bool useTrash = true);

    /**
     * @brief startRenameFile: Starts a thread to rename a single file
     * @param srcToRename: The url of the old file name
     * @param newUrl: The url that represents the new file
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startRenameFile(const QUrl& srcToRename, const QUrl& newUrl);

private:

    friend class IOJobsManagerCreator;
    IOJobsManager();
};

} // namespace Digikam

#endif // FILESYSTEMJOBSMANAGER_H
