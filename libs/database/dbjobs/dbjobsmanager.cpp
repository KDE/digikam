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

#include "dbjobsmanager.h"
#include "dbjobsthread.h"
#include "dbjobinfo.h"

namespace Digikam
{

class DBJobsManagerCreator
{
public:

    DBJobsManager object;
};

Q_GLOBAL_STATIC(DBJobsManagerCreator, creator)

// -----------------------------------------------

DBJobsManager::DBJobsManager()
{
}

DBJobsManager* DBJobsManager::instance()
{
    return &creator->object;
}

AlbumsDBJobsThread* DBJobsManager::startAlbumsJobThread(AlbumsDBJobInfo* const jInfo)
{
    AlbumsDBJobsThread* const thread = new AlbumsDBJobsThread(this);
    thread->albumsListing(jInfo);
    thread->start();

    return thread;
}

DatesDBJobsThread* DBJobsManager::startDatesJobThread(DatesDBJobInfo* const jInfo)
{
    DatesDBJobsThread* const thread = new DatesDBJobsThread(this);
    thread->datesListing(jInfo);
    thread->start();

    return thread;
}

TagsDBJobsThread* DBJobsManager::startTagsJobThread(TagsDBJobInfo* const jInfo)
{
    TagsDBJobsThread* const thread = new TagsDBJobsThread(this);
    thread->tagsListing(jInfo);
    thread->start();

    return thread;
}

SearchesDBJobsThread* DBJobsManager::startSearchesJobThread(SearchesDBJobInfo* const jInfo)
{
    SearchesDBJobsThread* const thread = new SearchesDBJobsThread(this);
    thread->searchesListing(jInfo);
    thread->start();

    return thread;
}

GPSDBJobsThread* DBJobsManager::startGPSJobThread(GPSDBJobInfo* const jInfo)
{
    GPSDBJobsThread* const thread = new GPSDBJobsThread(this);
    thread->GPSListing(jInfo);
    thread->start();

    return thread;
}

} // namespace Digikam
