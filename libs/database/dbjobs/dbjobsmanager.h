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

#include <QObject>
#include "dbjobinfo.h"
#include "dbjobsthread.h"

namespace Digikam
{

class DBJobsManager : public QObject
{
    Q_OBJECT

public:

    explicit DBJobsManager();
    ~DBJobsManager();

    static DBJobsManager* instance();

    GPSDBJobsThread*      startGPSJobThread(GPSDBJobInfo *jInfo);
    TagsDBJobsThread*     startTagsJobThread(TagsDBJobInfo *jInfo);
    DatesDBJobsThread*    startDatesJobThread(DatesDBJobInfo *jInfo);
    AlbumsDBJobsThread*   startAlbumsJobThread(AlbumsDBJobInfo *jInfo);
    SearchesDBJobsThread* startSearchesJobThread(SearchesDBJobInfo *jInfo);

private:
    friend class DBJobsManagerCreator;
};

} // namespace Digikam

#endif // DBJOBSMANAGER_H
