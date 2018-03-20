/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-08
 * Description : Similarity database interface.
 *
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#include "similaritydb.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class SimilarityDb::Private
{

public:

    Private() :
        db(0)
    {
    }

    SimilarityDbBackend* db;
    //bool                 dbAvailable;

};

SimilarityDb::SimilarityDb(SimilarityDbBackend* const backend)
    : d(new Private)
{
    d->db = backend;
}

SimilarityDb::~SimilarityDb()
{
    delete d;
}

bool SimilarityDb::setSetting(const QString& keyword, const QString& value )
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    parameters.insert(QLatin1String(":value"), value);
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("ReplaceSimilaritySetting")), parameters);

    return (queryStateResult == BdEngineBackend::NoErrors);
}

QString SimilarityDb::getSetting(const QString& keyword)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;
    // TODO Should really check return status here
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectSimilaritySetting")), parameters, &values);
    qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "SimilarityDb SelectSimilaritySetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

QString SimilarityDb::getLegacySetting(const QString& keyword)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;
    // TODO Should really check return status here
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectSimilarityLegacySetting")), parameters, &values);
    qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "SimilarityDb SelectSimilaritySetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

// ----------- General methods for entry access ----------

QSet<qlonglong> SimilarityDb::registeredImageIds() const
{
    QSet<qlonglong> imageIds;

    {
        // Get all image ids from the first and second imageid column of the ImageSimilarity table.
        QList<QVariant> values;
        d->db->execSql(QString::fromUtf8("SELECT imageid1, imageId2 FROM ImageSimilarity;"),
                       &values);

        for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
        {
            imageIds << (*it).toLongLong();
            ++it;
            imageIds << (*it).toLongLong();
            ++it;
        }
    }

    {
        // get all image ids from the ImageHaarMatrix table.
        QList<QVariant> values;
        d->db->execSql(QString::fromUtf8("SELECT imageid FROM ImageHaarMatrix;"),
                       &values);

        foreach(const QVariant& var, values)
        {
            imageIds << var.toLongLong();
        }
    }

    return imageIds;
}

// ----------- Methods for fingerprint (ImageHaarMatrix) table access ----------

bool SimilarityDb::hasFingerprints()
{
    return hasFingerprints(FuzzyAlgorithm::Haar);
}

bool SimilarityDb::hasFingerprints(FuzzyAlgorithm algorithm) const
{
    if (algorithm == FuzzyAlgorithm::Haar)
    {
        QList<QVariant> values;

        d->db->execSql(QString::fromUtf8("SELECT imageid FROM ImageHaarMatrix "
                                                 "WHERE matrix IS NOT NULL LIMIT 1;"),
                       &values);

        // return true if there is at least one fingerprint
        return !values.isEmpty();
    }
    else
    {
        return false;
    }
}

QList<qlonglong> SimilarityDb::getDirtyOrMissingFingerprints(QList<ImageInfo> imageInfos,
                                                             FuzzyAlgorithm algorithm)
{
    QList<qlonglong> itemIDs;

    if (algorithm == FuzzyAlgorithm::Haar)
    {
        foreach(const ImageInfo& info, imageInfos)
        {
            QList<QVariant> values;
            d->db->execSql(QString::fromUtf8("SELECT ImageHaarMatrix.modificationDate, ImageHaarMatrix.uniqueHash FROM ImageHaarMatrix "
                                                     "WHERE imageid=?;"),
                           info.id(), &values);

            if (values.empty())
            {
                // The image id does not exist -> missing fingerprint
                itemIDs << info.id();
            }
            else
            {
                // The image id exists -> if uniqueHash or modificationDate differ, we need a new fingerprint.
                if (values.size() == 2)
                {
                    if ((values.at(0).toDateTime() != info.modDateTime()) ||
                        (values.at(1).toString()   != info.uniqueHash()))
                    {
                        itemIDs << info.id();
                    }
                }
            }
        }
    }

    return itemIDs;
}

QStringList SimilarityDb::getDirtyOrMissingFingerprintURLs(QList<ImageInfo> imageInfos,
                                                           FuzzyAlgorithm algorithm)
{
    QStringList urls;

    if (algorithm == FuzzyAlgorithm::Haar)
    {
        foreach(const ImageInfo& info, imageInfos)
        {
            QList<QVariant> values;
            d->db->execSql(QString::fromUtf8("SELECT ImageHaarMatrix.modificationDate, ImageHaarMatrix.uniqueHash FROM ImageHaarMatrix "
                                                     "WHERE imageid=?;"),
                           info.id(), &values);

            if (values.empty())
            {
                // The image id does not exist -> missing fingerprint
                urls << info.filePath();
            }
            else
            {
                // The image id exists -> if uniqueHash or modificationDate differ, we need a new fingerprint.
                if (values.size() == 2)
                {
                    if ((values.at(0).toDateTime() != info.modDateTime()) ||
                        (values.at(1).toString()   != info.uniqueHash()))
                    {
                        urls << info.filePath();
                    }
                }
            }
        }
    }

    return urls;
}

void SimilarityDb::copySimilarityAttributes(qlonglong srcId, qlonglong dstId)
{
    // Go through ImageHaarMatrix table and copy the entries

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageHaarMatrix "
                                             " (imageid, modificationDate, uniqueHash, matrix) "
                                             "SELECT ?, modificationDate, uniqueHash, matrix "
                                             "FROM ImageHaarMatrix WHERE imageid=?;"),
                   dstId, srcId);
}


void SimilarityDb::removeImageFingerprint(qlonglong imageID,
                                          FuzzyAlgorithm algorithm)
{
    if (algorithm == FuzzyAlgorithm::Haar)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageHaarMatrix WHERE imageid=?;"), imageID);
    }
    else if (algorithm == FuzzyAlgorithm::TfIdf)
    {
        // TODO: Extend this when we have another algorithm in place.
    }

    // Also, remove all similarities for the image and algorithm if the backend is a MySQL DB.
    if (d->db->databaseType() == BdEngineBackend::DbType::MySQL)
    {
        removeImageSimilarity(imageID, algorithm);
    }
}

// ----------- Methods for image similarity table access ----------

double SimilarityDb::getImageSimilarity(qlonglong imageID1, qlonglong imageID2, FuzzyAlgorithm algorithm)
{
    // If the image ids are the same, we return 1 which is equivalent to 100%.
    // We do not have to access the database here as the same image id implies
    // the same image and thus identity.
    if (imageID1 == imageID2)
    {
        return 1;
    }

    // The image ids in each entry of the ImageSimilarity table follow a partial ordering,
    // meaning that the first image id is smaller or equal to the second id.
    // This has the advantage that only one entry exists for a pair of two images and one algorithm.
    // Also, no disjunction is necessary in the query.

    QPair<qlonglong, qlonglong> orderedIds = orderIds(imageID1, imageID2);

    QString similarityValueString = getImageSimilarityOrdered(orderedIds.first, orderedIds.second, algorithm);

    // If the similarity is non-null
    if (!similarityValueString.isEmpty())
    {
        bool ok;
        double val = similarityValueString.toDouble(&ok);

        if (ok)
        {
            return val;
        }

        qCDebug(DIGIKAM_SIMILARITYDB_LOG()) << "The similarity value for images with ids "
                                            << imageID1 << " and " << imageID2
                                            << " for similarity algorithm with id " << (int)algorithm
                                            << " cannot be transformed into a double! The value is "
                                            << similarityValueString;
    }

    // Return the info that there is no value.
    return -1;
}

void SimilarityDb::setImageSimilarity(qlonglong imageID1, qlonglong imageID2, double value, FuzzyAlgorithm algorithm)
{
    // We don't do anything if the image ids are identical as this is a waste of space.
    if (imageID1 == imageID2)
    {
        return;
    }

    // The image ids in each entry of the ImageSimilarity table follow a partial ordering,
    // meaning that the first image id is smaller or equal to the second id.
    // This has the advantage that only one entry exists for a pair of two images and one algorithm.
    // Also, no disjunction is necessary in the query.

    QPair<qlonglong, qlonglong> orderedIds = orderIds(imageID1, imageID2);

    //Check if entry already exists for above pair of images.(Avoiding duplicate entries)
    QString res = getImageSimilarityOrdered(orderedIds.first, orderedIds.second, algorithm);

    if (res.isEmpty())
    {
        d->db->execSql(QString::fromUtf8("REPLACE INTO ImageSimilarity "
                                                  "(imageid1, imageid2, value, algorithm) "
                                                  "VALUES(?, ?, ?, ?);"),
                       orderedIds.first, orderedIds.second, value, (int)algorithm);
    }
}

void SimilarityDb::removeImageSimilarity(qlonglong imageID, FuzzyAlgorithm algorithm)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageSimilarity "
                                             "WHERE ( imageid1=? OR imageid2=? ) AND algorithm=?;"),
                   imageID, imageID, (int)algorithm);
}

void SimilarityDb::removeImageSimilarity(qlonglong imageID1, qlonglong imageID2, FuzzyAlgorithm algorithm)
{
    QPair<qlonglong, qlonglong> orderedIds = orderIds(imageID1, imageID2);

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageSimilarity "
                                             "WHERE imageid1=? AND imageid2=? AND algorithm=?;"),
                   orderedIds.first, orderedIds.second, (int)algorithm);
}

QList<FuzzyAlgorithm> SimilarityDb::getImageSimilarityAlgorithms(qlonglong imageID1, qlonglong imageID2)
{

    QPair<qlonglong, qlonglong> orderedIds = orderIds(imageID1, imageID2);

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT algorithm FROM ImageSimilarity "
                           "WHERE imageid1=? AND imageid2=?;"),
                   orderedIds.first, orderedIds.second,
                   &values);

    QList<FuzzyAlgorithm> algorithms;

    foreach(const QVariant& var, values)
    {
        int algorithmId = var.toInt();

        if (algorithmId == 1)
        {
            algorithms << FuzzyAlgorithm::Haar;
        }
        else if (algorithmId == 2)
        {
            algorithms << FuzzyAlgorithm::TfIdf;
        }
    }

    return algorithms;
}

// ----------- Database shrinking and integrity check methods ----------

bool SimilarityDb::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkSimilarityDbIntegrity")), &values);

    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:
            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.
            return values.size() == 1 && values.first().toString().toLower().compare(QLatin1String("ok")) == 0;

        case BdEngineBackend::DbType::MySQL:
            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".
            //qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";
            if ((values.size() % 4) != 0)
            {
                return false;
            }

            for (QList<QVariant>::iterator it = values.begin(); it != values.end(); )
            {
                QString tableName   = (*it).toString();
                ++it;
                QString operation   = (*it).toString();
                ++it;
                QString messageType = (*it).toString();
                ++it;
                QString messageText = (*it).toString();
                ++it;

                if (messageText.toLower().compare(QLatin1String("ok")) != 0)
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table "
                                                  << tableName << ". Reason:" << messageText;
                    return false;
                }
                else
                {
/*
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table "
                                                  << tableName;
*/
                }
            }

            // No error conditions. Db passed the integrity check.
            return true;

        default:
            return false;
    }
}

void SimilarityDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumSimilarityDB")));
}

// ----------- Private methods ----------

QPair<qlonglong, qlonglong> SimilarityDb::orderIds(qlonglong id1, qlonglong id2)
{
    QPair<qlonglong, qlonglong> ordered;

    if (id1 <= id2)
    {
        // If the first id is smaller or equal to the second, set it as fst.
        ordered.first  = id1;
        ordered.second = id2;
    }
    else
    {
        ordered.first = id2;
        ordered.second = id1;
    }

    return ordered;
}

QString SimilarityDb::getImageSimilarityOrdered(qlonglong imageID1, qlonglong imageID2, FuzzyAlgorithm algorithm)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT value FROM ImageSimilarity "
                                             "WHERE ( imageid1=? OR imageid2=? ) AND algorithm=?;"),
                   imageID1, imageID2, (int)algorithm,
                   &values);

    if (!values.isEmpty())
    {
        return values.first().toString();
    }
    else
    {
        return QString();
    }
}

} // namespace Digikam
