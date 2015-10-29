/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-05
 * Description : Manager for creating and starting DB jobs threads
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

#ifndef DBJOBSMANAGER_H
#define DBJOBSMANAGER_H

// Qt includes

#include <QObject>

// Local includes

#include "dbjobinfo.h"
#include "dbjobsthread.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DBJobsManager : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief instance: returns DBJobsManager singleton
     * @return DBJobsManager global instance
     */
    static DBJobsManager* instance();

    /**
     * @brief startGPSJobThread: creates and starts GPS Job Thread
     * @param jInfo: holds job info about the DB job
     * @return GPSDBJobsThread instance for signal/slot connection
     */
    GPSDBJobsThread* startGPSJobThread(const GPSDBJobInfo& jInfo);

    /**
     * @brief startTagsJobThread: creates and starts Tag Job Thread
     * @param jInfo: holds job info about the DB job
     * @return TagsDBJobsThread instance for signal/slot connection
     */
    TagsDBJobsThread* startTagsJobThread(const TagsDBJobInfo& jInfo);

    /**
     * @brief startDatesJobThread: creates and starts Dates Job Thread
     * @param jInfo: holds job info about the DB job
     * @return DatesDBJobsThread instance for signal/slot connection
     */
    DatesDBJobsThread* startDatesJobThread(const DatesDBJobInfo& jInfo);

    /**
     * @brief startAlbumsJobThread: creates and starts Albums Job Thread
     * @param jInfo: holds job info about the DB job
     * @return AlbumsDBJobsThread instance for signal/slot connection
     */
    AlbumsDBJobsThread* startAlbumsJobThread(const AlbumsDBJobInfo& jInfo);

    /**
     * @brief startSearchesJobThread: creates and starts Searches Job Thread
     * @param jInfo: holds job info about the DB job
     * @return SearchesDBJobsThread instance for signal/slot connection
     */
    SearchesDBJobsThread* startSearchesJobThread(const SearchesDBJobInfo& jInfo);

private:

    friend class DBJobsManagerCreator;
    DBJobsManager();
};

} // namespace Digikam

#endif // DBJOBSMANAGER_H
