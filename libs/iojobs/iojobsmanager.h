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

#ifndef IOJOBSMANAGER_H
#define IOJOBSMANAGER_H

// Qt includes

#include <QObject>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dtrashiteminfo.h"
#include "iojobsthread.h"

namespace Digikam
{

class IOJobData;

class DIGIKAM_EXPORT IOJobsManager : public QObject
{

public:

    /**
     * @brief instance: returns the singleton of IO Jobs Manager
     * @return IOJobsManager global instance
     */
    static IOJobsManager* instance();

    /**
     * @brief startCopy: Starts a thread to copy items to destination
     * @param data: IOJobData container with source and destination url
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startCopy(IOJobData* const data);

    /**
     * @brief startMove: Starts a thread to move items to destination
     * @param data: IOJobData container with source and destination url
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startMove(IOJobData* const data);

    /**
     * @brief startDelete: Starts a thread to delete items
     * @param data: IOJobData container with source and destination url
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startDelete(IOJobData* const data);

    /**
     * @brief startRenameFile: Starts a thread to rename a single file
     * @param data: IOJobData container with source and destination url
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startRenameFile(IOJobData* const data);

    /**
     * @brief Starts a thread for listing items inside trash for specific collection
     * @param collectionPath: the path for collection to list items for it's trash
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startDTrashItemsListingForCollection(const QString& collectionPath);

    /**
     * @brief Starts a thread to restore mutiple trash items
     * @param trashItemsList: list of selected trash items to restore
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startRestoringDTrashItems(const DTrashItemInfoList& trashItemsList);

    /**
     * @brief Starts a thread to delete mutiple trash items
     * @param trashItemsList: list of selected trash items to delete
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startDeletingDTrashItems(const DTrashItemInfoList& trashItemsList);

private:

    friend class IOJobsManagerCreator;
    IOJobsManager();
};

} // namespace Digikam

#endif // FILESYSTEMJOBSMANAGER_H
