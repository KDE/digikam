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

// Local includes

#include "dbengineparameters.h"
#include "dbjobinfo.h"
#include "dbjob.h"
#include "haariface.h"
#include "imagelisterrecord.h"
#include "actionthreadbase.h"
#include "digikam_export.h"

namespace Digikam
{

class DBJob;

class DIGIKAM_DATABASE_EXPORT DBJobsThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit DBJobsThread(QObject* const parent);
    ~DBJobsThread();

    /**
     * @brief hasErrors: a method to check for jobs errors
     * @return bool: true if the error list is not empty
     */
    bool hasErrors();

    /**
     * @brief A method to get all errors reported from jobs
     * @return String list with errors
     */
    QList<QString>& errorsList();

protected:

    /**
     * @brief Connects the signals of job to the signals of the thread
     * @param j: Job that wanted to be connected
     */
    void connectFinishAndErrorSignals(DBJob* const j);

public Q_SLOTS:

    /**
     * @brief Appends the error string to m_errorsList
     * @param errString: error string reported from the job
     */
    void error(const QString& errString);

Q_SIGNALS:

    void finished();
    void data(const QList<ImageListerRecord>& records);

private:

    QStringList m_errorsList;
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit AlbumsDBJobsThread(QObject* const parent);
    ~AlbumsDBJobsThread();

    /**
     * @brief Starts PAlbums listing and scanning job(s)
     * @param info: represents the albums job info
     */
    void albumsListing(const AlbumsDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QMap<int, int>&);
    void faceFoldersData(const QMap<QString, QMap<int, int> >&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagsDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit TagsDBJobsThread(QObject* const parent);
    ~TagsDBJobsThread();

    /**
     * @brief Starts tags listing and scanning job(s)
     * @param info: represents the tags job info
     */
    void tagsListing(const TagsDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QMap<int, int>&);
    void faceFoldersData(const QMap<QString, QMap<int, int> >&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT DatesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit DatesDBJobsThread(QObject* const parent);
    ~DatesDBJobsThread();

    /**
     * @brief Starts dates listing and scanning
     * @param info: represents the dates job info
     */
    void datesListing(const DatesDBJobInfo& info);

Q_SIGNALS:

    void foldersData(const QMap<QDateTime, int>&);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchesDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit SearchesDBJobsThread(QObject* const parent);
    ~SearchesDBJobsThread();

    /**
     * @brief Starts searches listing and scanning
     * @param info: represents the searches job info
     */
    void searchesListing(const SearchesDBJobInfo& info);

Q_SIGNALS:

    void processedSize(int number);
    void totalSize(int number);
};

// ---------------------------------------------

class DIGIKAM_DATABASE_EXPORT GPSDBJobsThread : public DBJobsThread
{
    Q_OBJECT

public:

    explicit GPSDBJobsThread(QObject* const parent);
    ~GPSDBJobsThread();

    /**
     * @brief Starts GPS listing and scanning
     * @param info: represents the GPS job info
     */
    void GPSListing(const GPSDBJobInfo& info);

Q_SIGNALS:

    void directQueryData(const QList<QVariant>& data);
};

} // namespace Digikam

#endif // DBJOBSTHREAD_H
