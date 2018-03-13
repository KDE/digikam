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

#ifndef IOJOBSTHREAD_H
#define IOJOBSTHREAD_H

// Local includes

#include "actionthreadbase.h"
#include "digikam_export.h"
#include "dtrashiteminfo.h"

namespace Digikam
{

class IOJob;
class IOJobData;

class DIGIKAM_EXPORT IOJobsThread : public ActionThreadBase
{
    Q_OBJECT

public:

    IOJobsThread(QObject* const parent);
    ~IOJobsThread();

    /**
     * @brief Starts a number of jobs to copy source files to destination
     * @param data: IOJobsData container
     * @param srcFiles: files to be copied
     * @param destAlbum: destination folder
     */
    void copy(IOJobData* const data, const QList<QUrl>& srcFiles, const QUrl destAlbum);

    /**
     * @brief Starts a number of jobs to move source files to destination
     * @param data: IOJobsData container
     * @param srcFiles: files to be moved
     * @param destAlbum: destination folder
     */
    void move(IOJobData* const data, const QList<QUrl>& srcFiles, const QUrl destAlbum);

    /**
     * @brief Starts a job for listing trash items in a collection
     * @param collectionPath
     */
    void listDTrashItems(const QString& collectionPath);

    /**
     * @brief creates a job for every item to restore back to album
     * @param items to restore
     */
    void restoreDTrashItems(const DTrashItemInfoList& items);

    /**
     * @brief creates a job for every item to delete from collection trash
     * @param items to delete
     */
    void deleteDTrashItems(const DTrashItemInfoList& items);

    /**
     * @brief Starts a number of jobs to delete multiple files
     * @param data: IOJobsData container
     * @param srcsToDelete: files to be deleted
     * @param useTrash: a flag to use trash or not
     */
    void deleteFiles(IOJobData* const data, const QList<QUrl>& srcsToDelete, bool useTrash);

    /**
     * @brief Starts one job to rename a file to a new name
     * @param data: IOJobsData container
     * @param srcToRename: the url to be renamed
     * @param newName: the url of the renamed item
     */
    void renameFile(IOJobData* const data, const QUrl& srcToRename, const QUrl& newName);

    /**
     * @brief isCanceled
     * @return true if the thread was inturrupted
     */
    bool isCanceled();

    /**
     * @brief hasErrors
     * @return true if string list was not empty
     */
    bool hasErrors();

    /**
     * @brief errorsList
     * @return
     */
    QList<QString>& errorsList();

    /**
     * @brief jobData
     * @return
     */
    IOJobData* jobData();

public Q_SLOTS:

    /**
     * @brief cancels thread execution
     */
    void slotCancel();

Q_SIGNALS:

    void finished();

    void signalRenamed(const QUrl& oldUrl, const QUrl& newURl);
    void signalRenameFailed(const QUrl& oldUrl);
    void signalOneProccessed(int operation);

    void collectionTrashItemInfo(const DTrashItemInfo& trashItemInfo);

private:

    /**
     * @brief connects the job with signals/slots
     * @param job to be connected
     */
    void connectOneJob(IOJob* const j);

    /**
     * @brief Recursive method to find the suitable to restore items from trash
     * @param colPath: Path of item in collection before deleting to trash
     * @param usedUrls: a list of all used urls to rename previous files,
     *        to prevent duplication
     * @param version: to add to the base name in case the name was taken
     * @return QUrl to use in the renameFile() method
     */
    QUrl getAvailableQUrlToRestoreInCollection(const QString& fileColPath, QList<QUrl>& usedUrls, int version = 0);

private Q_SLOTS:

    /**
     * @brief connected to all active jobs and checks if the job
     *        list has finished to report that thread is finished
     */
    void slotOneJobFinished();

    /**
     * @brief A slot to receive the error from the job
     * @param errString: string to be appended
     */
    void slotError(const QString& errString);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IOJOBSTHREAD_H
