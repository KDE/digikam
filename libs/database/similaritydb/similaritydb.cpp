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
#include "collectionmanager.h"
#include "collectionlocation.h"

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

bool SimilarityDb::hasHaarFingerprints() const
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT imageid FROM ImageHaarMatrix "
                                     "WHERE matrix IS NOT NULL LIMIT 1;"),
                   &values);

    // return true if there is at least one fingerprint
    return !values.isEmpty();
}

QList<qlonglong> SimilarityDb::getDirtyOrMissingFingerprints()
{
    QList<qlonglong> itemIDs;
    QList<QVariant>  values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                        "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                        " WHERE Images.status=1 AND Images.category=1 AND "
                        " ( ImageHaarMatrix.imageid IS NULL "
                        "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                        "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QStringList SimilarityDb::getDirtyOrMissingFingerprintURLs()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                           "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                           "LEFT JOIN Albums ON Albums.id=Images.album "
                           " WHERE Images.status=1 AND Images.category=1 AND "
                           " ( ImageHaarMatrix.imageid IS NULL "
                           "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                           "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                   &values);

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath  = (*it).toString();
        ++it;
        name          = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
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

QString SimilarityDb::getImageSimilarity(qlonglong imageID1, qlonglong imageID2)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT value FROM ImageSimilarity "
                           "WHERE imageid1=? and imageid2=?;"),
                   imageID1, imageID2,
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

void SimilarityDb::setImageSimilarity(qlonglong imageID1, qlonglong imageID2, double value)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageSimilarity "
                           "(imageid1, imageid2, value) "
                           "VALUES(?, ?, ?);"),
                   imageID1, imageID2, value);
}

QString SimilarityDb::getImageSimilarityAlgorithm(qlonglong imageID1, qlonglong imageID2)
{
    QString algo;
    
    d->db->execSql(QString::fromUtf8("SELECT algorithm FROM ImageSimilarity "
                           "WHERE imageid1=? and imageid2=?;"),
                   imageID1, imageID2,
                   &id);
    
    if(id.toInt() == 1)
        algo = "Haar";
    else
        algo = "Unknown";
    
    return algo;
}

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
            if ( (values.size() % 4) != 0)
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

} // namespace Digikam
