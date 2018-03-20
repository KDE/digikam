/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
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
     */
    void copy(IOJobData* const data);

    /**
     * @brief Starts a number of jobs to move source files to destination
     * @param data: IOJobsData container
     */
    void move(IOJobData* const data);

    /**
     * @brief Starts a number of jobs to delete multiple files
     * @param data: IOJobsData container
     */
    void deleteFiles(IOJobData* const data);

    /**
     * @brief Starts one job to rename a file to a new name
     * @param data: IOJobsData container
     */
    void renameFile(IOJobData* const data);

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

    void signalRenamed(const QUrl& oldUrl);
    void signalRenameFailed(const QUrl& oldUrl);
    void signalOneProccessed(int operation);

    void collectionTrashItemInfo(const DTrashItemInfo& trashItemInfo);

private:

    /**
     * @brief connects the job with signals/slots
     * @param job to be connected
     */
    void connectOneJob(IOJob* const j);

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
