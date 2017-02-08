/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// OpenCV includes need to show up before Qt includes
#include "lbphfacemodel.h"

// Local includes

#include "facedb.h"
#include "digikam_debug.h"

namespace FacesEngine
{

class FaceDb::Private
{

public:
    Private()
        : db(0)
    {
    }

    FaceDbBackend* db;
};

FaceDb::FaceDb(FaceDbBackend* const db)
    : d(new Private)
{
    d->db = db;
}

FaceDb::~FaceDb()
{
    delete d;
}

BdEngineBackend::QueryState FaceDb::setSetting(const QString& keyword, const QString& value)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    parameters.insert(QLatin1String(":value"), value);
    return d->db->execDBAction(d->db->getDBAction(QLatin1String("ReplaceFaceSetting")), parameters);
}

QString FaceDb::setting(const QString& keyword) const
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;
    // TODO Should really check return status here
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectFaceSetting")), parameters, &values);
    qCDebug(DIGIKAM_FACEDB_LOG) << "FaceDB SelectFaceSetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

int FaceDb::addIdentity() const
{
    QVariant id;
    d->db->execSql(QString::fromLatin1("INSERT INTO Identities (type) VALUES (0);"), 0, &id);
    return id.toInt();
}

void FaceDb::updateIdentity(const Identity& p)
{
    d->db->execSql(QString::fromLatin1("DELETE FROM IdentityAttributes WHERE id=?;"), p.id());
    const QMap<QString, QString> map = p.attributesMap();
    QMap<QString, QString>::const_iterator it;

    for (it = map.constBegin(); it != map.constEnd(); ++it)
    {
        d->db->execSql(QString::fromLatin1("INSERT INTO IdentityAttributes (id, attribute, value) VALUES (?, ?,?);"),
                       p.id(), it.key(), it.value());
    }
}

void FaceDb::deleteIdentity(int id)
{
    // Triggers do the rest
    d->db->execSql(QString::fromLatin1("DELETE FROM Identities WHERE id=?;"), id);
}

void FaceDb::deleteIdentity(const QString & uuid)
{
    QList<QVariant> ids;
    d->db->execSql(QString::fromLatin1("SELECT Identities.id FROM Identities LEFT JOIN IdentityAttributes ON "
                                       " Identities.id=IdentityAttributes.id WHERE "
                                       " IdentityAttributes.attribute='uuid' AND IdentityAttributes.value=?;"), uuid, &ids);
    if (ids.size() == 1)
    {
        deleteIdentity(ids.first().toInt());
    }
    else
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot delete identity with uuid "
                                      << uuid << ". There are " << ids.size()
                                      << " identities with this uuid.";
    }
}

QList<Identity> FaceDb::identities() const
{
    QList<QVariant> ids;
    QList<Identity> results;
    d->db->execSql(QString::fromLatin1("SELECT id FROM Identities;"), &ids);

    foreach (const QVariant& v, ids)
    {
        QList<QVariant> values;
        Identity p;
        p.setId(v.toInt());
        d->db->execSql(QString::fromLatin1("SELECT attribute, value FROM IdentityAttributes WHERE id=?;"), p.id(), &values);

        for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
        {
            QString attribute = it->toString();
            ++it;
            QString value     = it->toString();
            ++it;

            p.setAttribute(attribute, value);
        }

        results << p;
    }

    return results;
}

QList<int> FaceDb::identityIds() const
{
    QList<QVariant> ids;
    d->db->execSql(QString::fromLatin1("SELECT id FROM Identities;"), &ids);

    QList<int> results;

    foreach (const QVariant& var, ids)
    {
        results << var.toInt();
    }

    return results;
}

namespace
{
    enum
    {
        LBPHStorageVersion = 1
    };
}

void FaceDb::updateLBPHFaceModel(LBPHFaceModel& model)
{
    QVariantList values;
    values << LBPHStorageVersion << model.radius() << model.neighbors() << model.gridX() << model.gridY();

    if (model.databaseId)
    {
        values << model.databaseId;
        d->db->execSql(QString::fromLatin1("UPDATE OpenCVLBPHRecognizer SET version=?, radius=?, neighbors=?, grid_x=?, grid_y=? WHERE id=?;"), values);
    }
    else
    {
        QVariant insertedId;
        d->db->execSql(QString::fromLatin1("INSERT INTO OpenCVLBPHRecognizer (version, radius, neighbors, grid_x, grid_y) VALUES (?,?,?,?,?);"),
                       values, 0, &insertedId);
        model.databaseId = insertedId.toInt();
    }

    QList<LBPHistogramMetadata> metadataList = model.histogramMetadata();

    for (int i = 0 ; i < metadataList.size() ; i++)
    {
        const LBPHistogramMetadata& metadata = metadataList[i];

        if (metadata.storageStatus == LBPHistogramMetadata::Created)
        {
            OpenCVMatData data = model.histogramData(i);

            if (data.data.isEmpty())
            {
                qCWarning(DIGIKAM_FACEDB_LOG) << "Histogram data to commit in database are empty for Identity " << metadata.identity;
            }
            else
            {
                QByteArray compressed = qCompress(data.data);

                if (compressed.isEmpty())
                {
                    qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot compress histogram data to commit in database for Identity " << metadata.identity;
                }
                else
                {
                    QVariantList histogramValues;
                    QVariant     insertedId;

                    histogramValues << model.databaseId
                                    << metadata.identity
                                    << metadata.context
                                    << data.type
                                    << data.rows
                                    << data.cols
                                    << compressed;

                    d->db->execSql(QString::fromLatin1("INSERT INTO OpenCVLBPHistograms (recognizerid, identity, context, type, rows, cols, data) "
                                   "VALUES (?,?,?,?,?,?,?);"),
                                   histogramValues, 0, &insertedId);

                    model.setWrittenToDatabase(i, insertedId.toInt());

                    qCDebug(DIGIKAM_FACEDB_LOG) << "Commit compressed histogram " << metadata.databaseId << " for identity " << metadata.identity << " with size " << compressed.size();
                }
            }
        }
    }
}

LBPHFaceModel FaceDb::lbphFaceModel() const
{
    QVariantList values;
    qCDebug(DIGIKAM_FACEDB_LOG) << "Loading LBPH model";
    d->db->execSql(QString::fromLatin1("SELECT id, version, radius, neighbors, grid_x, grid_y FROM OpenCVLBPHRecognizer;"), &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        LBPHFaceModel model;
        model.databaseId = it->toInt();
        ++it;
        qCDebug(DIGIKAM_FACEDB_LOG) << "Found model id" << model.databaseId;

        int version      = it->toInt();
        ++it;

        if (version > LBPHStorageVersion)
        {
            qCDebug(DIGIKAM_FACEDB_LOG) << "Unsupported LBPH storage version" << version;
            it += 4;
            continue;
        }

        model.setRadius(it->toInt());
        ++it;
        model.setNeighbors(it->toInt());
        ++it;
        model.setGridX(it->toInt());
        ++it;
        model.setGridY(it->toInt());
        ++it;

        DbEngineSqlQuery query = d->db->execQuery(QString::fromLatin1("SELECT id, identity, context, type, rows, cols, data "
                                                                      "FROM OpenCVLBPHistograms WHERE recognizerid=?;"),
                                                                      model.databaseId);
        QList<OpenCVMatData> histograms;
        QList<LBPHistogramMetadata> histogramMetadata;

        while (query.next())
        {
            LBPHistogramMetadata metadata;
            OpenCVMatData        data;

            metadata.databaseId    = query.value(0).toInt();
            metadata.identity      = query.value(1).toInt();
            metadata.context       = query.value(2).toString();
            metadata.storageStatus = LBPHistogramMetadata::InDatabase;

            // cv::Mat
            data.type              = query.value(3).toInt();
            data.rows              = query.value(4).toInt();
            data.cols              = query.value(5).toInt();
            QByteArray cData       = query.value(6).toByteArray();

            if (!cData.isEmpty())
            {
                data.data = qUncompress(cData);

                if (data.data.isEmpty())
                {
                    qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot uncompress histogram data to checkout from database for Identity " << metadata.identity;
                }
                else
                {
                    qCDebug(DIGIKAM_FACEDB_LOG) << "Checkout compressed histogram " << metadata.databaseId << " for identity " << metadata.identity << " with size " << cData.size();

                    histograms        << data;
                    histogramMetadata << metadata;
                }
            }
            else
            {
                qCWarning(DIGIKAM_FACEDB_LOG) << "Histogram data to checkout from database are empty for Identity " << metadata.identity;
            }
        }

        model.setHistograms(histograms, histogramMetadata);
        return model;
    }

    return LBPHFaceModel();
}

void FaceDb::clearLBPHTraining(const QString& context)
{
    if (context.isNull())
    {
        d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms;"));
        d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHRecognizer;"));
    }
    else
    {
        d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms WHERE context=?;"), context);
    }
}

void FaceDb::clearLBPHTraining(const QList<int>& identities, const QString& context)
{
    foreach (int id, identities)
    {
        if (context.isNull())
        {
            d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms WHERE identity=?;"), id);
        }
        else
        {
            d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms WHERE identity=? AND context=?;"), id, context);
        }
    }
}

bool FaceDb::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("integrityCheck")), &values);
    if (values.size() == 1)
    {
        if (values.first().toString().compare(QLatin1String("ok")) == 0)
        {
            return true;
        }
    }
    return false;
}

void FaceDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuum")));
}

} // namespace FacesEngine
