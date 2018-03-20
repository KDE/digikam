/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
 * Copyright (C) 2016-2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
 * Copyright (C) 2003      by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "haariface.h"

// C++ includes

#include <fstream>
#include <cmath>
#include <cstring>

// Qt includes

#include <QByteArray>
#include <QDataStream>
#include <QImage>
#include <QImageReader>
#include <QMap>

// Local includes

#include "digikam_debug.h"
#include "jpegutils.h"
#include "dimg.h"
#include "imageinfo.h"
#include "coredbaccess.h"
#include "coredbtransaction.h"
#include "coredb.h"
#include "coredbbackend.h"
#include "coredbsearchxml.h"
#include "dbenginesqlquery.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"

using namespace std;

// TODO: Always store similarities in the similaritydb

namespace Digikam
{

typedef QMap<qlonglong, Haar::SignatureData> SignatureCache;
typedef QMap<qlonglong, int>                 AlbumCache;

/** This class encapsulates the Haar signature in a QByteArray
 *  that can be stored as a BLOB in the database.
 *
 *  Reading and writing is done in a platform-independent manner, which
 *  induces a certain overhead, but which is necessary IMO.
 */
class DatabaseBlob
{
public:

    enum
    {
        Version = 1
    };

public:

    DatabaseBlob() = default;

    /** Read the QByteArray into the Haar::SignatureData.
     */
    void read(const QByteArray& array, Haar::SignatureData* const data)
    {
        QDataStream stream(array);

        // check version
        qint32 version;
        stream >> version;

        if (version != Version)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Unsupported binary version of Haar Blob in database";
            return;
        }

        stream.setVersion(QDataStream::Qt_4_3);

        // read averages
        for (int i = 0; i < 3; ++i)
        {
            stream >> data->avg[i];
        }

        // read coefficients
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < Haar::NumberOfCoefficients; ++j)
            {
                stream >> data->sig[i][j];
            }
        }
    }

    QByteArray write(Haar::SignatureData* const data)
    {
        QByteArray array;
        array.reserve(sizeof(qint32) + 3*sizeof(double) + 3*sizeof(qint32)*Haar::NumberOfCoefficients);
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_4_3);

        // write version
        stream << (qint32)Version;

        // write averages
        for (int i = 0; i < 3; ++i)
        {
            stream << data->avg[i];
        }

        // write coefficients
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < Haar::NumberOfCoefficients; ++j)
            {
                stream << data->sig[i][j];
            }
        }

        return array;
    }
};

// -----------------------------------------------------------------------------------------------------

class HaarIface::Private
{
public:

    Private()
    {
        data                       = nullptr;
        bin                        = nullptr;
        signatureCache             = nullptr;
        albumCache                 = nullptr;
        useSignatureCache          = false;

        signatureQuery             = QString::fromUtf8("SELECT M.imageid, M.matrix FROM ImageHaarMatrix AS M;");
    }

    ~Private()
    {
        delete data;
        delete bin;
        delete signatureCache;
        delete albumCache;
    }

    void createLoadingBuffer()
    {
        if (!data)
        {
            data = new Haar::ImageData;
        }
    }

    void createWeightBin()
    {
        if (!bin)
        {
            bin = new Haar::WeightBin;
        }
    }

    void setSignatureCacheEnabled(bool cache, const QSet<qlonglong>& imageIds)
    {
        setSignatureCacheEnabled(cache);

        // stop here if we disable cached signatures
        if (!cache || imageIds.isEmpty())
        {
            return;
        }

        // Remove all ids from the fully created signatureCache that are not needed for the duplicates search.
        // This is usually faster then starting a query for every single id in imageIds.
        for (SignatureCache::iterator it = signatureCache->begin();
             it != signatureCache->end(); )
        {
            if (!imageIds.contains(it.key()))
            {
                it = signatureCache->erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void setSignatureCacheEnabled(bool cache)
    {
        delete signatureCache;
        signatureCache = nullptr;
        delete albumCache;
        albumCache     = nullptr;

        if (cache)
        {
            signatureCache = new SignatureCache();
            albumCache     = new AlbumCache();
        }
        useSignatureCache = cache;

        // stop here if we disable cached signatures
        if (!cache)
        {
            return;
        }

        // Variables for data read from DB
        SimilarityDbAccess  similarityDbAccess;
        DatabaseBlob        blob;
        qlonglong           imageid;
        int                 albumid;
        Haar::SignatureData targetSig;

        // reference for easier access
        SignatureCache& signatureCache = *this->signatureCache;
        AlbumCache&     albumCache     = *this->albumCache;

        DbEngineSqlQuery query = similarityDbAccess.backend()->prepareQuery(signatureQuery);

        if (!similarityDbAccess.backend()->exec(query))
        {
            return;
        }

        while (query.next())
        {
            imageid = query.value(0).toLongLong();

            // Get the album id and status of the item with the ImageInfo.
            ImageInfo info(imageid);
            if (!info.isNull() && info.isVisible())
            {
                blob.read(query.value(1).toByteArray(), &targetSig);
                albumid = info.albumId();
                signatureCache[imageid] = targetSig;
                albumCache[imageid]     = albumid;
            }
        }
    }

    bool             useSignatureCache;
    Haar::ImageData* data;
    Haar::WeightBin* bin;
    SignatureCache*  signatureCache;
    AlbumCache*      albumCache;

    QString          signatureQuery;
    QSet<int>        albumRootsToSearch;
};

HaarIface::HaarIface()
    : d(new Private())
{
}

HaarIface::~HaarIface()
{
    delete d;
}

void HaarIface::setAlbumRootsToSearch(QList<int> albumRootIds)
{
    setAlbumRootsToSearch(albumRootIds.toSet());
}

void HaarIface::setAlbumRootsToSearch(QSet<int> albumRootIds)
{
    d->albumRootsToSearch = albumRootIds;
}

int HaarIface::preferredSize()
{
    return Haar::NumberOfPixels;
}

bool HaarIface::indexImage(const QString& filename)
{
    QImage image = loadQImage(filename);

    if (image.isNull())
    {
        return false;
    }

    return indexImage(filename, image);
}

bool HaarIface::indexImage(const QString& filename, const QImage& image)
{
    ImageInfo info = ImageInfo::fromLocalFile(filename);

    if (info.isNull())
    {
        return false;
    }

    return indexImage(info.id(), image);
}

bool HaarIface::indexImage(const QString& filename, const DImg& image)
{
    ImageInfo info = ImageInfo::fromLocalFile(filename);

    if (info.isNull())
    {
        return false;
    }

    return indexImage(info.id(), image);
}

bool HaarIface::indexImage(qlonglong imageid, const QImage& image)
{
    if (image.isNull())
    {
        return false;
    }

    d->createLoadingBuffer();
    d->data->fillPixelData(image);

    return indexImage(imageid);
}

bool HaarIface::indexImage(qlonglong imageid, const DImg& image)
{
    if (image.isNull())
    {
        return false;
    }

    d->createLoadingBuffer();
    d->data->fillPixelData(image);

    return indexImage(imageid);
}

// private method: d->data has been filled
bool HaarIface::indexImage(qlonglong imageid)
{
    Haar::Calculator haar;
    haar.transform(d->data);

    Haar::SignatureData sig;
    haar.calcHaar(d->data, &sig);

    SimilarityDbAccess access;

    // Store main entry
    {
        // prepare blob
        DatabaseBlob blob;
        QByteArray array = blob.write(&sig);

        ImageInfo info(imageid);

        if (!info.isNull() && info.isVisible()) {

            access.backend()->execSql(QString::fromUtf8("REPLACE INTO ImageHaarMatrix "
                                                                " (imageid, modificationDate, uniqueHash, matrix) "
                                                                " VALUES(?, ?, ?, ?);"),
                                      imageid, info.modDateTime(), info.uniqueHash(), array);
        }
    }

    return true;
}

QString HaarIface::signatureAsText(const QImage& image)
{
    d->createLoadingBuffer();
    d->data->fillPixelData(image);

    Haar::Calculator haar;
    haar.transform(d->data);
    Haar::SignatureData sig;
    haar.calcHaar(d->data, &sig);

    DatabaseBlob blob;
    QByteArray array = blob.write(&sig);

    return QString::fromUtf8(array.toBase64());
}

QList<qlonglong> HaarIface::bestMatchesForImage(const QImage& image, QList<int>& targetAlbums, int numberOfResults, SketchType type)
{
    d->createLoadingBuffer();
    d->data->fillPixelData(image);

    Haar::Calculator haar;
    haar.transform(d->data);
    Haar::SignatureData sig;
    haar.calcHaar(d->data, &sig);

    return bestMatches(&sig, numberOfResults, targetAlbums, type).values();
}

QList<qlonglong> HaarIface::bestMatchesForImage(qlonglong imageid, QList<int>& targetAlbums, int numberOfResults, SketchType type)
{
    Haar::SignatureData sig;

    if (!retrieveSignatureFromDB(imageid, &sig))
    {
        return QList<qlonglong>();
    }

    return bestMatches(&sig, numberOfResults, targetAlbums, type).values();
}

QPair<double,QMap<qlonglong,double>> HaarIface::bestMatchesForImageWithThreshold(const QString& imagePath, double requiredPercentage,
                                                             double maximumPercentage, QList<int>& targetAlbums,
                                                             DuplicatesSearchRestrictions searchResultRestriction, SketchType type)
{
    d->createLoadingBuffer();
    DImg image(imagePath);

    if (image.isNull())
    {
        return QPair<double,QMap<qlonglong,double>>();
    }

    d->data->fillPixelData(image);

    Haar::Calculator haar;
    haar.transform(d->data);
    Haar::SignatureData sig;
    haar.calcHaar(d->data, &sig);

    // Remove all previous similarities from pictures
    SimilarityDbAccess().db()->removeImageSimilarity(0);

    // Apply duplicates search for the image. Use the image id 0 which cannot be present.
    return bestMatchesWithThreshold(0, &sig, requiredPercentage, maximumPercentage, targetAlbums, searchResultRestriction, type);
}

QPair<double,QMap<qlonglong,double>> HaarIface::bestMatchesForImageWithThreshold(qlonglong imageid, double requiredPercentage,
                                                             double maximumPercentage, QList<int>& targetAlbums,
                                                             DuplicatesSearchRestrictions searchResultRestriction, SketchType type)
{
    if ( !d->useSignatureCache || (d->signatureCache->isEmpty() && d->useSignatureCache) )
    {
        Haar::SignatureData sig;

        if (!retrieveSignatureFromDB(imageid, &sig))
        {
            return QPair<double,QMap<qlonglong,double>>();
        }

        return bestMatchesWithThreshold(imageid, &sig, requiredPercentage, maximumPercentage, targetAlbums, searchResultRestriction, type);
    }
    else
    {
        // reference for easier access
        SignatureCache& signatureCache = *d->signatureCache;
        Haar::SignatureData& sig       = signatureCache[imageid];
        return bestMatchesWithThreshold(imageid, &sig, requiredPercentage, maximumPercentage, targetAlbums, searchResultRestriction, type);
    }
}

QList<qlonglong> HaarIface::bestMatchesForFile(const QString& filename, QList<int>& targetAlbums, int numberOfResults, SketchType type)
{
    QImage image = loadQImage(filename);

    if (image.isNull())
    {
        return QList<qlonglong>();
    }

    return bestMatchesForImage(image, targetAlbums, numberOfResults, type);
}

QMap<qlonglong,double> HaarIface::bestMatchesForSignature(const QString& signature, QList<int>& targetAlbums, int numberOfResults, SketchType type)
{
    QByteArray bytes = QByteArray::fromBase64(signature.toLatin1());

    DatabaseBlob blobReader;
    Haar::SignatureData sig;
    blobReader.read(bytes, &sig);

    // Get all matching images with their score and save their similarity to the signature, i.e. id -2
    QMultiMap<double,qlonglong> matches = bestMatches(&sig, numberOfResults, targetAlbums, type);
    QMap<qlonglong, double> result;
    for (QMultiMap<double,qlonglong>::const_iterator it = matches.constBegin(); it != matches.constEnd(); ++it)
    {
        // Add the image id and the normalised score (make sure that it is positive and between 0 and 1.
        result.insert(it.value(), ( 0.0 - ( it.key()/100) ));
    }
    return result;
}

QMultiMap<double, qlonglong> HaarIface::bestMatches(Haar::SignatureData* const querySig, int numberOfResults, QList<int>& targetAlbums, SketchType type)
{
    QMap<qlonglong, double> scores = searchDatabase(querySig, type, targetAlbums);

    // Find out the best matches, those with the lowest score
    // We make use of the feature that QMap keys are sorted in ascending order
    // Of course, images can have the same score, so we need a multi map
    QMultiMap<double, qlonglong> bestMatches;
    bool                         initialFill = false;
    double                       score, worstScore, bestScore;
    qlonglong                    id;

    for (QMap<qlonglong, double>::const_iterator it = scores.constBegin(); it != scores.constEnd(); ++it)
    {
        score = it.value();
        id    = it.key();

        if (!initialFill)
        {
            // as long as the maximum number of results is not reached, just fill up the map
            bestMatches.insert(score, id);
            initialFill = (bestMatches.size() >= numberOfResults);
        }
        else
        {
            // find the last entry, the one with the highest (=worst) score
            QMap<double, qlonglong>::iterator last = bestMatches.end();
            --last;
            worstScore = last.key();

            // if the new entry has a higher score, put it in the list and remove that last one
            if (score < worstScore)
            {
                bestMatches.erase(last);
                bestMatches.insert(score, id);
            }
            else if (score == worstScore)
            {
                bestScore = bestMatches.begin().key();

                // if the score is identical for all entries, increase the maximum result number
                if (score == bestScore)
                {
                    bestMatches.insert(score, id);
                }
            }
        }
    }

/*
    for (QMap<double, qlonglong>::iterator it = bestMatches.begin(); it != bestMatches.end(); ++it)
        qCDebug(DIGIKAM_DATABASE_LOG) << it.key() << it.value();
*/

    return bestMatches;
}

QPair<double,QMap<qlonglong,double>> HaarIface::bestMatchesWithThreshold(qlonglong imageid, Haar::SignatureData* const querySig,
                                                                         double requiredPercentage, double maximumPercentage,
                                                                         QList<int>& targetAlbums, DuplicatesSearchRestrictions searchResultRestriction,
                                                                         SketchType type)
{
    int albumId = CoreDbAccess().db()->getItemAlbum(imageid);
    QMap<qlonglong, double> scores = searchDatabase(querySig, type, targetAlbums, searchResultRestriction, imageid, albumId);
    double lowest, highest;
    getBestAndWorstPossibleScore(querySig, type, &lowest, &highest);
    // The range between the highest (worst) and lowest (best) score
    // example: 0.2 and 0.5 -> 0.3
    double scoreRange      = highest - lowest;
    // The lower the requiredPercentage is, the higher will the result be.
    // example: 0.7 -> 0.3
    double percentageRange = 1.0 - requiredPercentage;
    // example: 0.2 + (0.3 * 0.3) = 0.2 + 0.09 = 0.29
    double requiredScore   = lowest + scoreRange * percentageRange;
    // Set the supremum which solves the problem that if
    // required == maximum, no results will be returned.
    // Eg, id required == maximum == 50.0, only images with exactly this
    // similarity are returned. But users expect also to see images
    // with similarity 50,x.
    double supremum = (floor(maximumPercentage*100 + 1.0))/100;

    QMap<qlonglong, double> bestMatches;
    double score, percentage, avgPercentage = 0.0;
    QPair<double,QMap<qlonglong,double>> result;
    qlonglong           id;
    SimilarityDbAccess  access;

    for (QMap<qlonglong, double>::const_iterator it = scores.constBegin(); it != scores.constEnd(); ++it)
    {
        score = it.value();
        id    = it.key();

        // If the score of the picture is at most the required (maximum) score and
        if (score <= requiredScore)
        {
            percentage = 1.0 - (score - lowest) / scoreRange;
            // If the found image is the original one (check by id) or the percentage is below the maximum.
            if ((id == imageid) || (percentage < supremum))
            {
                bestMatches.insert(id, percentage);
                // If the current image is not the original, use the images similarity for the average percentage
                // Also, save the similarity of the found image to the original image.
                if (id != imageid)
                {
                    // Store the similarity if the reference image has a valid image id
                    if (imageid > 0)
                    {
                        access.db()->setImageSimilarity(id,imageid,percentage);
                    }
                    avgPercentage += percentage;
                }
            }
        }
    }

    // Debug output
    if (bestMatches.count() > 1)
    {
        // The average percentage is the sum of all percentages 
        // (without the original picture) divided by the count of pictures -1.
        // Subtracting 1 is necessary since the original picture is not used for the calculation.
        avgPercentage = avgPercentage / (bestMatches.count() - 1);

        qCDebug(DIGIKAM_DATABASE_LOG) << "Duplicates with id and score:";

        for (QMap<qlonglong, double>::const_iterator it = bestMatches.constBegin(); it != bestMatches.constEnd(); ++it)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << it.key() << QString::number(it.value() * 100) + QLatin1Char('%');
        }
    }
    result.first = avgPercentage;
    result.second = bestMatches;
    return result;
}

bool HaarIface::fulfillsRestrictions(qlonglong imageId, int albumId, qlonglong originalImageId, int originalAlbumId,
                              QList<int>& targetAlbums, DuplicatesSearchRestrictions searchResultRestriction)
{
    if (imageId == originalImageId)
    {
        return true;
    }
    else if (targetAlbums.isEmpty() || targetAlbums.contains(albumId))
    {
        return (searchResultRestriction == None) ||
               (searchResultRestriction == SameAlbum && originalAlbumId == albumId) ||
               (searchResultRestriction == DifferentAlbum && originalAlbumId != albumId);
    }
    else
    {
        return false;
    }
}

/// This method is the core functionality: It assigns a score to every image in the db
QMap<qlonglong, double> HaarIface::searchDatabase(Haar::SignatureData* const querySig, SketchType type, QList<int>& targetAlbums,
                                                  DuplicatesSearchRestrictions searchResultRestriction,
                                                  qlonglong originalImageId, int originalAlbumId)
{
    d->createWeightBin();

    // The table of constant weight factors applied to each channel and bin
    Haar::Weights weights((Haar::Weights::SketchType)type);

    // layout the query signature for fast lookup
    Haar::SignatureMap queryMapY, queryMapI, queryMapQ;
    queryMapY.fill(querySig->sig[0]);
    queryMapI.fill(querySig->sig[1]);
    queryMapQ.fill(querySig->sig[2]);
    Haar::SignatureMap* queryMaps[3] = { &queryMapY, &queryMapI, &queryMapQ };

    // Map imageid -> score. Lowest score is best.
    // any newly inserted value will be initialized with a score of 0, as required
    QMap<qlonglong, double> scores;

    // Variables for data read from DB
    SimilarityDbAccess  access;
    DatabaseBlob        blob;
    qlonglong           imageid;
    int                 albumid;
    Haar::SignatureData targetSig;

    // reference for easier access
    SignatureCache& signatureCache = *d->signatureCache;
    AlbumCache&     albumCache     = *d->albumCache;

    bool filterByAlbumRoots = !d->albumRootsToSearch.isEmpty();

    // if no cache is used or the cache signature map is empty, query the database
    if (!d->useSignatureCache || (signatureCache.isEmpty() && d->useSignatureCache))
    {
        DbEngineSqlQuery query = access.backend()->prepareQuery(d->signatureQuery);

        if (!access.backend()->exec(query))
        {
            return scores;
        }

        // We don't use SimilarityDb's convenience calls, as the result set is large
        // and we try to avoid copying in a temporary QList<QVariant>
        int albumRootId = 0;

        while (query.next())
        {
            imageid = query.value(0).toLongLong();

            // Get the album id, album root id and status of the item with the ImageInfo.
            ImageInfo info(imageid);
            if (!info.isNull() && info.isVisible())
            {
                if (filterByAlbumRoots)
                {
                    albumRootId = info.albumRootId();

                    if (!d->albumRootsToSearch.contains(albumRootId))
                    {
                        continue;
                    }
                }

                blob.read(query.value(1).toByteArray(), &targetSig);
                albumid = info.albumId();

                if (d->useSignatureCache)
                {
                    signatureCache[imageid] = targetSig;
                    albumCache[imageid]     = albumid;
                }

                // If the image is the original one or
                // No restrictions apply or
                // SameAlbum restriction applies and the albums are equal or
                // DifferentAlbum restriction applies and the albums differ
                // then calculate the score.
                // Also, restrict to target album
                if ( fulfillsRestrictions(imageid, albumid, originalImageId, originalAlbumId, targetAlbums, searchResultRestriction) )
                {
                    double&              score = scores[imageid];
                    Haar::SignatureData& qSig  = *querySig;
                    Haar::SignatureData& tSig  = targetSig;

                    score = calculateScore(qSig, tSig, weights, queryMaps);
                }
            }
        }
    }
    // read cached signature map if possible
    else
    {
        foreach(const qlonglong& imageid, signatureCache.keys())
        {
            albumid = albumCache[imageid];

            // If the image is the original one or
            // No restrictions apply or
            // SameAlbum restriction applies and the albums are equal or
            // DifferentAlbum restriction applies and the albums differ
            // then calculate the score.
            if ( fulfillsRestrictions(imageid, albumid, originalImageId, originalAlbumId, targetAlbums, searchResultRestriction) )
            {
                double& score              = scores[imageid];
                Haar::SignatureData& qSig  = *querySig;
                Haar::SignatureData& tSig  = signatureCache[imageid];

                score = calculateScore(qSig, tSig, weights, queryMaps);
            }
        }
    }

    return scores;
}

QImage HaarIface::loadQImage(const QString& filename)
{
    // NOTE: Can be optimized using DImg.

    QImage image;

    if (JPEGUtils::isJpegImage(filename))
    {
        // use fast jpeg loading
        if (!JPEGUtils::loadJPEGScaled(image, filename, Haar::NumberOfPixels))
        {
            // try QT now.
            if (!image.load(filename))
            {
                return QImage();
            }
        }
    }
    else
    {
        // use default QT image loading
        if (!image.load(filename))
        {
            return QImage();
        }
    }

    return image;
}

bool HaarIface::retrieveSignatureFromDB(qlonglong imageid, Haar::SignatureData* const sig)
{
    QList<QVariant> values;
    SimilarityDbAccess().backend()->execSql(QString::fromUtf8("SELECT matrix FROM ImageHaarMatrix WHERE imageid=?"),
                                        imageid, &values);

    if (values.isEmpty())
    {
        return false;
    }

    DatabaseBlob blob;

    blob.read(values.first().toByteArray(), sig);
    return true;
}

void HaarIface::getBestAndWorstPossibleScore(Haar::SignatureData* const sig, SketchType type,
        double* const lowestAndBestScore, double* const highestAndWorstScore)
{
    Haar::Weights weights((Haar::Weights::SketchType)type);
    double score = 0;

    // In the first step, the score is initialized with the weighted color channel averages.
    // We don't know the target channel average here, we only now its not negative => assume 0
    for (int channel=0; channel<3; ++channel)
    {
        score += weights.weightForAverage(channel) * fabs( sig->avg[channel] /*- targetSig.avg[channel]*/ );
    }

    *highestAndWorstScore = score;

    // Next consideration: The lowest possible score is reached if the signature is identical.
    // The first step (see above) will result in 0 - skip it.
    // In the second step, for every coefficient in the sig that have query and target in common,
    // so in our case all 3*40, subtract the specifically assigned weighting.
    score = 0;

    for (int channel = 0; channel < 3; ++channel)
    {
        Haar::Idx* coefs = sig->sig[channel];

        for (int coef = 0; coef < Haar::NumberOfCoefficients; ++coef)
        {
            score -= weights.weight(d->bin->binAbs(coefs[coef]), channel);
        }
    }

    *lowestAndBestScore = score;
}


QMap<QString, QString> HaarIface::writeSAlbumQueries(QMap< double,QMap< qlonglong,QList<qlonglong> > > searchResults)
{
    // Build search XML from the results. Store list of ids of similar images.
    QMap<QString, QString> queries;

    // Iterate over the similarity
    for (QMap< double,QMap< qlonglong,QList<qlonglong> > >::const_iterator similarity_it = searchResults.constBegin(); similarity_it != searchResults.constEnd(); ++similarity_it)
    {
        double similarity = similarity_it.key() * 100;
        QMap<qlonglong,QList<qlonglong>> sameSimilarityMap = similarity_it.value();
        // Iterate ofer
        for (QMap< qlonglong,QList<qlonglong> >::const_iterator it = sameSimilarityMap.constBegin(); it != sameSimilarityMap.constEnd(); ++it)
        {
            SearchXmlWriter writer;
            writer.writeGroup();
            writer.writeField(QLatin1String("imageid"), SearchXml::OneOf);
            writer.writeValue(it.value());
            writer.finishField();
            // Add the average similarity as field
            writer.writeField(QLatin1String("noeffect_avgsim"), SearchXml::Equal);
            writer.writeValue(similarity);
            writer.finishField();
            writer.finishGroup();
            writer.finish();
            // Use the id of the first duplicate as name of the search
            queries.insert(QString::number(it.key()), writer.xml());
        }
    }

    return queries;
}

void HaarIface::rebuildDuplicatesAlbums(const QList<qlonglong>& imageIds, double requiredPercentage, double maximumPercentage, DuplicatesSearchRestrictions searchResultRestriction,
                                 HaarProgressObserver* const observer)
{
    QMap< double,QMap< qlonglong,QList<qlonglong> > > results = findDuplicates(imageIds.toSet(), requiredPercentage, maximumPercentage, searchResultRestriction, observer);

    QMap<QString, QString> queries = writeSAlbumQueries(results);

    // Write the new search albums to the database
    {
        CoreDbAccess access;
        CoreDbTransaction transaction(&access);

        // Update existing searches by deleting and adding them.
        for (QMap<QString, QString>::const_iterator it = queries.constBegin(); it != queries.constEnd(); ++it)
        {
            access.db()->deleteSearch(it.key().toInt());
            access.db()->addSearch(DatabaseSearch::DuplicatesSearch, it.key(), it.value());
        }
    }
}

void HaarIface::rebuildDuplicatesAlbums(const QList<int>& albums2Scan, const QList<int>& tags2Scan, AlbumTagRelation relation,
                                        double requiredPercentage, double maximumPercentage, DuplicatesSearchRestrictions searchResultRestriction, HaarProgressObserver* const observer)
{
    // Carry out search. This takes long.
    QMap< double,QMap< qlonglong,QList<qlonglong> > > results = findDuplicatesInAlbumsAndTags(albums2Scan, tags2Scan, relation,
                                                                                              requiredPercentage, maximumPercentage,
                                                                                              searchResultRestriction, observer);

    // Build search XML from the results. Store list of ids of similar images.
    QMap<QString, QString> queries = writeSAlbumQueries(results);

    // Write search albums to database
    {
        CoreDbAccess access;
        CoreDbTransaction transaction(&access);

        // delete all old searches
        access.db()->deleteSearches(DatabaseSearch::DuplicatesSearch);

        // create new groups
        for (QMap<QString, QString>::const_iterator it = queries.constBegin(); it != queries.constEnd(); ++it)
        {
            access.db()->addSearch(DatabaseSearch::DuplicatesSearch, it.key(), it.value());
        }
    }
}

QMap< double,QMap< qlonglong,QList<qlonglong> > > HaarIface::findDuplicatesInAlbums(const QList<int>& albums2Scan,
                                                                      double requiredPercentage,
                                                                      double maximumPercentage,
                                                                      HaarProgressObserver* const observer)
{
    QSet<qlonglong> idList;

    // Get all items DB id from all albums and all collections
    foreach(int albumId, albums2Scan)
    {
        idList.unite(CoreDbAccess().db()->getItemIDsInAlbum(albumId).toSet());
    }

    return findDuplicates(idList, requiredPercentage, maximumPercentage, DuplicatesSearchRestrictions::None, observer);
}

QMap< double,QMap< qlonglong,QList<qlonglong> > > HaarIface::findDuplicatesInAlbumsAndTags(const QList<int>& albums2Scan,
                                                                             const QList<int>& tags2Scan,
                                                                             AlbumTagRelation relation,
                                                                             double requiredPercentage,
                                                                             double maximumPercentage,
                                                                             DuplicatesSearchRestrictions searchResultRestriction,
                                                                             HaarProgressObserver* const observer)
{
    QSet<qlonglong> imagesFromAlbums;
    QSet<qlonglong> imagesFromTags;

    QSet<qlonglong> idList;

    // Get all items DB id from all albums and all collections
    foreach(int albumId, albums2Scan)
    {
        imagesFromAlbums.unite(CoreDbAccess().db()->getItemIDsInAlbum(albumId).toSet());
    }

    // Get all items DB id from all tags
    foreach(int albumId, tags2Scan)
    {
        imagesFromTags.unite(CoreDbAccess().db()->getItemIDsInTag(albumId).toSet());
    }

    switch (relation)
    {
        case Union:
        {
            // ({} UNION A) UNION T = A UNION T
            idList.unite(imagesFromAlbums).unite(imagesFromTags);
            break;
        }
        case Intersection:
        {
            // ({} UNION A) INTERSECT T = A INTERSECT T
            idList.unite(imagesFromAlbums).intersect(imagesFromTags);
            break;
        }
        case AlbumExclusive:
        {
            // ({} UNION A) = A
            idList.unite(imagesFromAlbums);
            // (A INTERSECT T) = A'
            imagesFromAlbums.intersect(imagesFromTags);
            // A\A' = albums without tags
            idList.subtract(imagesFromAlbums);
            break;
        }
        case TagExclusive:
        {
            // ({} UNION T) = TT
            idList.unite(imagesFromTags);
            // (A INTERSECT T) = A' = T'
            imagesFromAlbums.intersect(imagesFromTags);
            // T\T' = tags without albums
            idList.subtract(imagesFromAlbums);
            break;
        }
        case NoMix:
        {
            //
            if ((albums2Scan.isEmpty() && tags2Scan.isEmpty()))
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Duplicates search: Both the albums and the tags list are non-empty but the album/tag relation stated a NoMix. Skipping duplicates search";
                return QMap< double,QMap< qlonglong,QList<qlonglong> > >();
            }
            else
            {
                // ({} UNION A) UNION T = A UNION T = A Xor T
                idList.unite(imagesFromAlbums).unite(imagesFromTags);
            }
        }
    }

    return findDuplicates(idList, requiredPercentage, maximumPercentage, searchResultRestriction, observer);
}

QMap< double,QMap< qlonglong,QList<qlonglong> > > HaarIface::findDuplicates(const QSet<qlonglong>& images2Scan,
                                                              double requiredPercentage,
                                                              double maximumPercentage,
                                                              DuplicatesSearchRestrictions searchResultRestriction,
                                                              HaarProgressObserver* const observer)
{
    QMap<double,QMap<qlonglong,QList<qlonglong>>> resultsMap;
    QMap<double,QMap<qlonglong,QList<qlonglong>>>::iterator similarity_it;
    QSet<qlonglong>::const_iterator     it;
    QPair<double,QMap<qlonglong,double>>      bestMatches;
    QList<qlonglong>                    imageIdList;
    QSet<qlonglong>                     resultsCandidates;

    int                                 total        = 0;
    int                                 progress     = 0;
    int                                 progressStep = 20;

    if (observer)
    {
        total        = images2Scan.count();
        progressStep = qMax(progressStep, total / 100);
        observer->totalNumberToScan(total);
    }

    // create signature cache map for fast lookup
    d->setSignatureCacheEnabled(true, images2Scan);

    for (it = images2Scan.constBegin(); it != images2Scan.constEnd(); ++it)
    {
        if (observer && observer->isCanceled())
        {
            break;
        }

        if (!resultsCandidates.contains(*it))
        {
            QList<int> targetAlbums;
            // find images with required similarity
            bestMatches = bestMatchesForImageWithThreshold(*it, requiredPercentage, maximumPercentage, targetAlbums, searchResultRestriction, ScannedSketch);
            // We need only the image ids from the best matches map.
            imageIdList = bestMatches.second.keys();
            if (!imageIdList.isEmpty())
            {
                // the list will usually contain one image: the original. Filter out.
                if (!(imageIdList.count() == 1 && imageIdList.first() == *it))
                {
                    // make a lookup for the average similarity
                    similarity_it = resultsMap.find(bestMatches.first);
                    // If there is an entry for this similarity, add the result set. Else, create a new similarity entry.
                    if (similarity_it != resultsMap.end())
                    {
                        similarity_it->insert(*it,imageIdList);
                    }
                    else
                    {
                        QMap<qlonglong,QList<qlonglong>> result;
                        result.insert(*it, imageIdList);
                        resultsMap.insert(bestMatches.first,result);
                    }
                    resultsCandidates << *it;
                    resultsCandidates.unite(imageIdList.toSet());
                }
            }
        }

        // if an imageid is not a results candidate, remove it from the cached signature map as well,
        // to greatly improve speed
        if (!resultsCandidates.contains(*it))
        {
            d->signatureCache->remove(*it);
        }

        ++progress;

        if (observer && (progress == total || progress % progressStep == 0))
        {
            observer->processedNumber(progress);
        }
    }

    // make sure the progress bar is really set to 100% when search is finished
    if (observer)
    {
        observer->processedNumber(total);
    }

    // disable cache
    d->setSignatureCacheEnabled(false);

    return resultsMap;
}

double HaarIface::calculateScore(Haar::SignatureData& querySig, Haar::SignatureData& targetSig,
                                 Haar::Weights& weights, Haar::SignatureMap** const queryMaps)
{
    double score = 0.0;

    // Step 1: Initialize scores with average intensity values of all three channels
    for (int channel = 0; channel < 3; ++channel)
    {
        score += weights.weightForAverage(channel) * fabs( querySig.avg[channel] - targetSig.avg[channel] );
    }

    // Step 2: Decrease the score if query and target have significant coefficients in common
    Haar::Idx* sig               = nullptr;
    Haar::SignatureMap* queryMap = nullptr;
    int x                        = 0;

    for (int channel = 0; channel < 3; ++channel)
    {
        sig      = targetSig.sig[channel];
        queryMap = queryMaps[channel];

        for (int coef = 0; coef < Haar::NumberOfCoefficients; ++coef)
        {
            // x is a pixel index, either positive or negative, 0..16384
            x = sig[coef];

            // If x is a significant coefficient with the same sign in the query signature as well,
            // decrease the score (lower is better)
            // Note: both method calls called with x accept positive or negative values
            if ((*queryMap)[x])
            {
                score -= weights.weight(d->bin->binAbs(x), channel);
            }
        }
    }

    return score;
}

}  // namespace Digikam
