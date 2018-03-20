/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-07
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

#ifndef SIMILARITY_DATABASE_H
#define SIMILARITY_DATABASE_H

// Qt includes

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QMap>
#include <QHash>
#include <QSet>

// Local includes

#include "dbenginesqlquery.h"
#include "similaritydbbackend.h"
#include "similaritydbaccess.h"
#include "digikam_export.h"
#include "imageinfo.h"

namespace Digikam
{

enum class FuzzyAlgorithm
{
    Unknown = 0,
    Haar = 1,
    TfIdf = 2
};


class DIGIKAM_EXPORT SimilarityDb
{
public:

    /**
     * Set the database seting entry given by keyword to the given value.
     * @param keyword The keyword, i.e. setting name.
     * @param value The value.
     * @return True, if the value was set and false, else..
     */
    bool setSetting(const QString& keyword, const QString& value);

    /**
     * Returns the setting with the keyword name.
     * @param keyword The setting entry name.
     * @return The setting value.
     */
    QString getSetting(const QString& keyword);

    /**
     * Returns the legacy settings with the keyword name.
     * @param keyword The setting entry name.
     * @return The setting value.
     */
    QString getLegacySetting(const QString& keyword);

    /**
     * This method returns all image ids that are present in the similarity db tables.
     * \return a set of all present image ids.
     */
    QSet<qlonglong> registeredImageIds() const;

    // ----------- Methods for fingerprint (ImageHaarMatrix) table access ----------

    /**
     * This method checks if there are any fingerprints for any algorithm present.
     * @return True, if fingerprints exist.
     */
    bool hasFingerprints();

    /**
     * This method checks if there are any fingerprints for the given algorithm.
     * @param algorithm The algorithm.
     * @return true, if there are fingerprints and false, otherwise.
     */
    bool hasFingerprints(FuzzyAlgorithm algorithm) const;

    /**
     * Returns a list of all item ids (images, videos,...) where either no fingerprint for the given
     * algorithm exists or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * @param imageInfos The image info objects representing the items.
     * @param algorithm The algorithm.
     * @return The ids of the items whose fingerprints are dirty or missing.
     */
    QList<qlonglong> getDirtyOrMissingFingerprints(QList<ImageInfo> imageInfos,
                                                   FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Returns a list of the URLs of all items (images, videos,...) where either no fingerprint for the given
     * algorithm exists or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * @param imageInfos The image info objects representing the items.
     * @param algorithm The algorithm.
     * @return The URLs of the items whose fingerprints are dirty or missing.
     */
    QStringList      getDirtyOrMissingFingerprintURLs(QList<ImageInfo> imageInfos,
                                                      FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the fingerprint entry for the given imageId and algorithm.
     * Also, this automatically removes the entries in the ImageSimilarities table for the
     * given algorithm and image id.
     * @param imageID The image id.
     * @param algorithm The algorithm.
     */
    void removeImageFingerprint(qlonglong imageID,
                                FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Copies all similarity-specific information, from image srcId to destId.
     */
    void copySimilarityAttributes(qlonglong srcId,
                                  qlonglong destId);

    // ----------- Methods for image similarity table access ----------

    /**
     * Returns the similarity value for two images.
     * A value of -1 means nonexistence.
     * A value of -2 means that there is a value that cannot be converted into a double
     */
    double getImageSimilarity(qlonglong imageID1,
                              qlonglong imageID2,
                              FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    void setImageSimilarity(qlonglong imageID1,
                            qlonglong imageID2,
                            double value,
                            FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the image similarity entries for the imageID and algorithm.
     * @param imageID The image id.
     * @param algorithm The algorithm.
     */
    void removeImageSimilarity(qlonglong imageID,
                               FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method removes the image similarity entry for the imageIDs and algorithm.
     * @param imageID1 The first image id.
     * @param imageID2 The second image id.
     * @param algorithm The algorithm.
     */
    void removeImageSimilarity(qlonglong imageID1,
                               qlonglong imageID2,
                               FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * Returns the algorithms for which a similarity value exists for the given image ids.
     * @param imageID1 The first image id.
     * @param imageID2 The second image id.
     * @return a list of all algorithms for which a similarity value exists.
     */
    QList<FuzzyAlgorithm> getImageSimilarityAlgorithms(qlonglong imageID1,
                                                       qlonglong imageID2);

    // ----------- Database shrinking and integrity check methods ----------

    /**
     * This method checks the integrity of the similarity database.
     * @return true, if the integrity check was passed and false, else.
     */
    bool integrityCheck();

    /**
     * This method shrinks the database.
     */
    void vacuum();

private:

    /**
     * This private variant of getImageSimilarity assumes that imageID1 <= imageID2
     * @param imageID1 the id of the first image.
     * @param imageID2 the id of the second image.
     * @param algorithm the algorithm
     * @return An empty string, if no similarity value exists for the ids and the algorithm, and the value as string, else.
     */
    QString getImageSimilarityOrdered(qlonglong imageID1,
                                      qlonglong imageID2,
                                      FuzzyAlgorithm algorithm = FuzzyAlgorithm::Haar);

    /**
     * This method applies a partial ordering to id1 and id2, i.e.
     * the result is a pair where the first is id1 iff id1 <= id2 and id2 otherwise.
     * @param id1 The first id.
     * @param id2 The second id.
     * @return A pair in which the ids are in partial ascending order.
     */
    QPair<qlonglong, qlonglong> orderIds(qlonglong id1, qlonglong id2);

    /**
     * The constructor of the similarity db class
     * @param backend The database backend.
     */
    explicit SimilarityDb(SimilarityDbBackend* const backend);

    /**
     * The destructor of the similarity db class.
     */
    ~SimilarityDb();

private:

    class Private;
    Private* const d;

    friend class SimilarityDbAccess;
};

} // namespace Digikam

#endif // SIMILARITY_DATABASE_H
