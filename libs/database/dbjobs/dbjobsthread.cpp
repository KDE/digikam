/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs thread for listing and scanning
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

#include "dbjobsthread.h"

#include "databaseaccess.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "duplicatesprogressobserver.h"

namespace Digikam
{

DBJobsThread::DBJobsThread(QObject* const parent)
    : RActionThreadBase(parent)
{
}

DBJobsThread::~DBJobsThread()
{
}

bool DBJobsThread::hasErrors()
{
    return !m_errorsList.isEmpty();
}

QList<QString> &DBJobsThread::errorsList()
{
    return m_errorsList;
}

void DBJobsThread::connectFinishAndErrorSignals(DBJob *j)
{
    connect(j, SIGNAL(signalDone()),
            this, SLOT(finished()));

    connect(j, SIGNAL(error(QString)),
            this, SLOT(error(QString)));
}

void DBJobsThread::error(const QString &errString)
{
    m_errorsList.append(errString);
}

// -------------------------------------------------

AlbumsDBJobsThread::AlbumsDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

AlbumsDBJobsThread::~AlbumsDBJobsThread()
{
}

void AlbumsDBJobsThread::albumsListing(AlbumsDBJobInfo *info)
{
    AlbumsJob *j = new AlbumsJob(info);

    connectFinishAndErrorSignals(j);

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(QMap<int,int>)),
                this, SIGNAL(foldersData(QMap<int,int>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

TagsDBJobsThread::TagsDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

TagsDBJobsThread::~TagsDBJobsThread()
{
}

void TagsDBJobsThread::tagsListing(TagsDBJobInfo *info)
{
    TagsJob *j = new TagsJob(info);

    connectFinishAndErrorSignals(j);

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(QMap<int,int>)),
                this, SIGNAL(foldersData(QMap<int,int>)));
    }
    else if(info->faceFolders)
    {
        connect(j, SIGNAL(faceFoldersData(QMap<QString,QMap<int,int> >)),
                this, SIGNAL(faceFoldersData(QMap<QString,QMap<int,int> >)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

DatesDBJobsThread::DatesDBJobsThread(QObject *const parent)
    : DBJobsThread(parent)
{
}

DatesDBJobsThread::~DatesDBJobsThread()
{
}

void DatesDBJobsThread::datesListing(DatesDBJobInfo *info)
{
    DatesJob *j = new DatesJob(info);

    connectFinishAndErrorSignals(j);

    if(info->folders)
    {
        connect(j, SIGNAL(foldersData(const QMap<QDateTime,int> &)),
                this, SIGNAL(foldersData(const QMap<QDateTime,int> &)));
    }
    else
    {
        connect(j, SIGNAL(data(const QList<ImageListerRecord> &)),
                this, SIGNAL(data(const QList<ImageListerRecord> &)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

GPSDBJobsThread::GPSDBJobsThread(QObject * const parent)
    : DBJobsThread(parent)
{
}

GPSDBJobsThread::~GPSDBJobsThread()
{
}

void GPSDBJobsThread::GPSListing(GPSDBJobInfo *info)
{
    GPSJob *j = new GPSJob(info);

    connectFinishAndErrorSignals(j);

    if(info->wantDirectQuery)
    {
        connect(j, SIGNAL(directQueryData(QList<QVariant>)),
                this, SIGNAL(directQueryData(QList<QVariant>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

// -------------------------------------------------

SearchesDBJobsThread::SearchesDBJobsThread(QObject * const parent)
    : DBJobsThread(parent)
{
}

SearchesDBJobsThread::~SearchesDBJobsThread()
{
}

void SearchesDBJobsThread::searchesListing(SearchesDBJobInfo *info)
{
    SearchesJob *j = new SearchesJob(info);

    connectFinishAndErrorSignals(j);

    if(info->duplicates)
    {

        connect(j, SIGNAL(totalSize(int)),
                this, SIGNAL(totalSize(int)));

        connect(j, SIGNAL(processedSize(int)),
                this, SIGNAL(processedSize(int)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ImageListerRecord>)),
                this, SIGNAL(data(QList<ImageListerRecord>)));
    }

    RJobCollection collection;
    collection.insert(j,0);

    appendJobs(collection);
}

} // namespace Digikam
