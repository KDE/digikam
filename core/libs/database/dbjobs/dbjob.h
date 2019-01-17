/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs for listing and scanning
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#ifndef DIGIKAM_DB_JOB_H
#define DIGIKAM_DB_JOB_H

// Local includes

#include "dbjobinfo.h"
#include "dbjobsthread.h"
#include "itemlisterrecord.h"
#include "duplicatesprogressobserver.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class DuplicatesProgressObserver;

class DIGIKAM_DATABASE_EXPORT DBJob : public ActionJob
{
    Q_OBJECT

protected:

    explicit DBJob();
    ~DBJob();

Q_SIGNALS:

    void data(const QList<ItemListerRecord>& records);
    void error(const QString& err);
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumsJob : public DBJob
{
    Q_OBJECT

public:

    explicit AlbumsJob(const AlbumsDBJobInfo& jobInfo);
    ~AlbumsJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<int, int>&);

private:

    AlbumsDBJobInfo m_jobInfo;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatesJob : public DBJob
{
    Q_OBJECT

public:

    explicit DatesJob(const DatesDBJobInfo& jobInfo);
    ~DatesJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<QDateTime, int>& datesStatMap);

private:

    DatesDBJobInfo m_jobInfo;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT GPSJob : public DBJob
{
    Q_OBJECT

public:

    explicit GPSJob(const GPSDBJobInfo& jobInfo);
    ~GPSJob();

protected:

    void run();

Q_SIGNALS:

    void directQueryData(const QList<QVariant>& data);

private:

    GPSDBJobInfo m_jobInfo;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagsJob : public DBJob
{
    Q_OBJECT

public:

    explicit TagsJob(const TagsDBJobInfo& jobInfo);
    ~TagsJob();

protected:

    void run();

Q_SIGNALS:

    void foldersData(const QMap<int, int>& data);
    void faceFoldersData(const QMap<QString, QMap<int, int> >& data);

private:

    TagsDBJobInfo m_jobInfo;
};

// ----------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchesJob : public DBJob
{
    Q_OBJECT

public:

    explicit SearchesJob(const SearchesDBJobInfo& jobInfo);
    ~SearchesJob();

    bool isCanceled();

Q_SIGNALS:

    void processedSize(int);
    void totalSize(int);

protected:

    void run();

private:

    SearchesDBJobInfo m_jobInfo;
};

} // namespace Digikam

#endif // DIGIKAM_DB_JOB_H
