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

#ifndef DBJOBSTHREAD_H
#define DBJOBSTHREAD_H

// KDCraw Includes

#include "KDCRAW/RActionThreadBase"

// Local includes

#include "databaseparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "haariface.h"
#include "imagelisterrecord.h"

using namespace KDcrawIface;

namespace Digikam
{

class DBJob;

class DBJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread();

    bool hasErrors();
    QList<QString> &errorsList();

protected:

    void connectFinishAndErrorSignals(DBJob *j);

public Q_SLOTS:

    void error(const QString &errString);

Q_SIGNALS:

    void finished();
    void data(const QList<ImageListerRecord> &records);

private:

    QStringList m_errorsList;
};

// ---------------------------------------------

class AlbumsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit AlbumsDBJobsThread(QObject *const parent);
    ~AlbumsDBJobsThread();

    void albumsListing(AlbumsDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<int, int> &);
    void faceFoldersData(const QMap<QString, QMap<int, int> > &);
};

// ---------------------------------------------

class TagsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit TagsDBJobsThread(QObject *const parent);
    ~TagsDBJobsThread();

    void tagsListing(TagsDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<int, int> &);
    void faceFoldersData(const QMap<QString, QMap<int, int> > &);
};

// ---------------------------------------------

class DatesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit DatesDBJobsThread(QObject *const parent);
    ~DatesDBJobsThread();

    void datesListing(DatesDBJobInfo *info);

Q_SIGNALS:

    void foldersData(const QMap<QDateTime,int> &);
};

// ---------------------------------------------

class SearchesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit SearchesDBJobsThread(QObject* const parent);
    ~SearchesDBJobsThread();

    void searchesListing(SearchesDBJobInfo *info);

Q_SIGNALS:

    void processedSize(int number);
    void totalSize(int number);
};

// ---------------------------------------------

class GPSDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit GPSDBJobsThread(QObject* const parent);
    ~GPSDBJobsThread();

    void GPSListing(GPSDBJobInfo *info);

Q_SIGNALS:

    void directQueryData(const QList<QVariant> & data);
};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
