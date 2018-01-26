/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tensor.h"
#include "input.h"
#include "layers.h"
#include "loss.h"
#include "core.h"
#include "solvers.h"
#include "cpu_dlib.h"
#include "tensor_tools.h"
#include "utilities.h"
#include "validation.h"
#include "serialize.h"
#include "matrix.h"
#include "matrix_utilities.h"
#include "matrix_subexp.h"
#include "matrix_math_functions.h"
#include "matrix_generic_image.h"
#include "cv_image.h"
#include "assign_image.h"
#include "interpolation.h"
#include "frontal_face_detector.h"

// Local includes

#include "eigenfacemodel.h"
#include "fisherfacemodel.h"
#include "lbphfacemodel.h"
#include "dnnfacemodel.h"
#include "dnn_face.h"
#include "facedb.h"
#include "digikam_debug.h"

namespace Digikam
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

/*
 * NOTE: This constructor is only used in facerec_dnnborrowed.cpp.
 * Create an object of FaceDb to invoke the method getFaceVector
 */
FaceDb::FaceDb()
    : d(new Private)
{
}

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
    d->db->execSql(QString::fromLatin1("INSERT INTO Identities (`type`) VALUES (0);"), 0, &id);
    return id.toInt();
}

void FaceDb::updateIdentity(const Identity& p)
{
    d->db->execSql(QString::fromLatin1("DELETE FROM IdentityAttributes WHERE id=?;"), p.id());
    const QMap<QString, QString> map = p.attributesMap();
    QMap<QString, QString>::const_iterator it;

    for (it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        d->db->execSql(QString::fromLatin1("INSERT INTO IdentityAttributes (id, attribute, `value`) VALUES (?, ?,?);"),
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
        d->db->execSql(QString::fromLatin1("SELECT attribute, `value` FROM IdentityAttributes WHERE id=?;"), p.id(), &values);

        for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
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

                    d->db->execSql(QString::fromLatin1("INSERT INTO OpenCVLBPHistograms (recognizerid, identity, `context`, `type`, `rows`, `cols`, `data`) "
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

        DbEngineSqlQuery query = d->db->execQuery(QString::fromLatin1("SELECT id, identity, `context`, `type`, `rows`, `cols`, `data` "
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
        d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms WHERE `context`=?;"), context);
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
            d->db->execSql(QString::fromLatin1("DELETE FROM OpenCVLBPHistograms WHERE identity=? AND `context`=?;"), id, context);
        }
    }
}

void FaceDb::getFaceVector(cv::Mat data, std::vector<float>& vecdata)
{
    DNNFaceKernel dnnface_kernel;
    dnnface_kernel.getFaceVector(data, vecdata);
}

void FaceDb::updateEIGENFaceModel(EigenFaceModel& model, const std::vector<cv::Mat>& images_rgb)
{
    QList<EigenFaceMatMetadata> metadataList = model.matMetadata();

    for (size_t i = 0, j = 0 ; i < (size_t)metadataList.size() ; i++)
    {
        const EigenFaceMatMetadata& metadata = metadataList[i];

        if (metadata.storageStatus == EigenFaceMatMetadata::Created)
        {
            OpenCVMatData data = model.matData(i);
            cv::Mat mat_rgb;

            if (j >= images_rgb.size())
            {
                qCWarning(DIGIKAM_FACEDB_LOG) << "updateEIGENFaceModel: the size of images_rgb is wrong";
            }
            else
            {
                mat_rgb = images_rgb[j++];
            }

            if (data.data.isEmpty())
            {
                qCWarning(DIGIKAM_FACEDB_LOG) << "Eigenface data to commit in database are empty for Identity " << metadata.identity;
            }
            else
            {
                QByteArray compressed = qCompress(data.data);
                std::vector<float> vecdata;
                this->getFaceVector(mat_rgb, vecdata);
                qCDebug(DIGIKAM_FACEDB_LOG) << "vecdata: " << vecdata[vecdata.size()-2] << " " << vecdata[vecdata.size()-1];

                QByteArray vec_byte(vecdata.size()*sizeof(float), 0);
                float* const fp = (float*)vec_byte.data();

                for (size_t k = 0; k < vecdata.size(); k++)
                {
                    *(fp + k) = vecdata[k];
                }

                QByteArray compressed_vecdata = qCompress(vec_byte);

                if (compressed.isEmpty())
                {
                    qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot compress mat data to commit in database for Identity " << metadata.identity;
                }
                else if (compressed_vecdata.isEmpty())
                {
                    qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot compress face vec data to commit in database for Identity " << metadata.identity;    
                }
                else
                {
                    QVariantList histogramValues;
                    QVariant     insertedId;

                    histogramValues << metadata.identity
                                    << metadata.context
                                    << data.type
                                    << data.rows
                                    << data.cols
                                    << compressed
                                    << compressed_vecdata;

                    d->db->execSql(QString::fromLatin1("INSERT INTO FaceMatrices (identity, `context`, `type`, `rows`, `cols`, `data`, vecdata) "
                                   "VALUES (?,?,?,?,?,?,?);"),
                                   histogramValues, 0, &insertedId);

                    model.setWrittenToDatabase(i, insertedId.toInt());

                    qCDebug(DIGIKAM_FACEDB_LOG) << "Commit compressed matData " << insertedId << " for identity "
                                                << metadata.identity << " with size " << compressed.size();
                }
            }
        }
    }
}

EigenFaceModel FaceDb::eigenFaceModel() const
{
    qCDebug(DIGIKAM_FACEDB_LOG) << "Loading EIGEN model";
    DbEngineSqlQuery query = d->db->execQuery(QString::fromLatin1("SELECT id, identity, `context`, `type`, `rows`, `cols`, `data`, vecdata "
                                                                  "FROM FaceMatrices;"));

    EigenFaceModel model = EigenFaceModel();
    QList<OpenCVMatData>        mats;
    QList<EigenFaceMatMetadata> matMetadata;

    while (query.next())
    {
        EigenFaceMatMetadata metadata;
        OpenCVMatData data;

        metadata.databaseId    = query.value(0).toInt();
        metadata.identity      = query.value(1).toInt();
        metadata.context       = query.value(2).toString();
        metadata.storageStatus = EigenFaceMatMetadata::InDatabase;

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
                qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot uncompress mat data to checkout from database for Identity " << metadata.identity;
            }
            else
            {
                qCDebug(DIGIKAM_FACEDB_LOG) << "Checkout compressed histogram " << metadata.databaseId << " for identity " << metadata.identity << " with size " << cData.size();

                mats        << data;
                matMetadata << metadata;
            }
        }
        else
        {
            qCWarning(DIGIKAM_FACEDB_LOG) << "Mat data to checkout from database are empty for Identity " << metadata.identity;
        }
    }

    model.setMats(mats, matMetadata);

    return model;
}

FisherFaceModel FaceDb::fisherFaceModel() const
{
    qCDebug(DIGIKAM_FACEDB_LOG) << "Loading FISHER model from FaceMatrices";
    DbEngineSqlQuery query = d->db->execQuery(QString::fromLatin1("SELECT id, identity, `context`, `type`, `rows`, `cols`, `data`, vecdata "
                                                                  "FROM FaceMatrices;"));

    FisherFaceModel model  = FisherFaceModel();
    QList<OpenCVMatData>         mats;
    QList<FisherFaceMatMetadata> matMetadata;

    while (query.next())
    {
        FisherFaceMatMetadata metadata;
        OpenCVMatData data;

        metadata.databaseId    = query.value(0).toInt();
        metadata.identity      = query.value(1).toInt();
        metadata.context       = query.value(2).toString();
        metadata.storageStatus = FisherFaceMatMetadata::InDatabase;

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
                qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot uncompress mat data to checkout from database for Identity " << metadata.identity;
            }
            else
            {
                qCDebug(DIGIKAM_FACEDB_LOG) << "Checkout compressed histogram " << metadata.databaseId << " for identity "
                                            << metadata.identity << " with size " << cData.size();

                mats        << data;
                matMetadata << metadata;
            }
        }
        else
        {
            qCWarning(DIGIKAM_FACEDB_LOG) << "Mat data to checkout from database are empty for Identity " << metadata.identity;
        }
    }

    model.setMats(mats, matMetadata);

    return model;
}

DNNFaceModel FaceDb::dnnFaceModel() const
{
    qCDebug(DIGIKAM_FACEDB_LOG) << "Loading DNN model";
    DbEngineSqlQuery query = d->db->execQuery(QString::fromLatin1("SELECT id, identity, `context`, `type`, `rows`, `cols`, `data`, vecdata "
                                                                  "FROM FaceMatrices;"));

    DNNFaceModel model = DNNFaceModel();
    QList<std::vector<float>> mats;
    QList<DNNFaceVecMetadata> matMetadata;

    while (query.next())
    {
        DNNFaceVecMetadata metadata;
        std::vector<float> vecdata;

        metadata.databaseId    = query.value(0).toInt();
        metadata.identity      = query.value(1).toInt();
        metadata.context       = query.value(2).toString();
        metadata.storageStatus = DNNFaceVecMetadata::InDatabase;
        QByteArray cData       = query.value(7).toByteArray();

        if (!cData.isEmpty())
        {
            QByteArray new_vec = qUncompress(cData);

            if (new_vec.isEmpty())
            {
                qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot uncompress mat data to checkout from database for Identity " << metadata.identity;
            }
            else
            {
                qCDebug(DIGIKAM_FACEDB_LOG) << "Checkout compressed histogram " << metadata.databaseId << " for identity "
                                            << metadata.identity << " with size " << cData.size();

                float* const it = (float *)new_vec.data();

                for (int i = 0; i < 128; i++)
                {
                    vecdata.push_back(*(it+i));
                }

                mats        << vecdata;
                matMetadata << metadata;
            }
        }
        else
        {
            qCWarning(DIGIKAM_FACEDB_LOG) << "Mat data to checkout from database are empty for Identity " << metadata.identity;
        }
    }

    model.setMats(mats, matMetadata);

    return model;
}

void FaceDb::clearEIGENTraining(const QString& context)
{
    if (context.isNull())
    {
        d->db->execSql(QString::fromLatin1("DELETE FROM FaceMatrices;"));
    }
    else
    {
        d->db->execSql(QString::fromLatin1("DELETE FROM FaceMatrices WHERE `context`=?;"), context);
    }
}

void FaceDb::clearEIGENTraining(const QList<int>& identities, const QString& context)
{
    foreach (int id, identities)
    {
        if (context.isNull())
        {
            d->db->execSql(QString::fromLatin1("DELETE FROM FaceMatrices WHERE identity=?;"), id);
        }
        else
        {
            d->db->execSql(QString::fromLatin1("DELETE FROM FaceMatrices WHERE identity=? AND `context`=?;"), id, context);
        }
    }
}

bool FaceDb::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkRecognitionDbIntegrity")), &values);

    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:

            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.

            return( (values.size() == 1) &&
                     (values.first().toString().toLower().compare(QLatin1String("ok")) == 0)
                  );

        case BdEngineBackend::DbType::MySQL:

            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".

            //qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";

            if ( (values.size() % 4) != 0)
            {
                return false;
            }

            for (QList<QVariant>::iterator it = values.begin() ; it != values.end() ; )
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
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table " << tableName << ". Reason:" << messageText;
                    return false;
                }
                else
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table " << tableName;
                }
            }

            // No error conditions. Db passed the integrity check.
            return true;

        default:
            return false;
    }
}

void FaceDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumRecognitionDB")));
}

} // namespace Digikam
