/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
 * Copyright (C) 2003      by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef HAARIFACE_H
#define HAARIFACE_H

// Qt includes

#include <QString>
#include <QMap>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "haar.h"

class QImage;

namespace Digikam
{

class DImg;

class HaarProgressObserver
{
public:

    virtual ~HaarProgressObserver()
    {
    };

    virtual void totalNumberToScan(int number) = 0;
    virtual void processedNumber(int numberThatHasBeenProcessed) = 0;
};

// --------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT HaarIface
{

public:

    enum SketchType
    {
        ScannedSketch   = 0,
        HanddrawnSketch = 1
    };

public:

    HaarIface();
    ~HaarIface();

    static int preferredSize();

    /** Adds an image to the index in the database.
     */
    bool indexImage(const QString& filename);
    bool indexImage(const QString& filename, const QImage& image);
    bool indexImage(const QString& filename, const DImg& image);
    bool indexImage(qlonglong imageid, const QImage& image);
    bool indexImage(qlonglong imageid, const DImg& image);

    /** Searches the database for the best matches for the specified query image.
     *  The numberOfResults best matches are returned.
     */
    QList<qlonglong> bestMatchesForImage(qlonglong imageid, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForImage(const QImage& image, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForFile(const QString& filename, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForSignature(const QString& signature, int numberOfResults=20, SketchType type=ScannedSketch);

    /** Searches the database for the best matches for the specified query image.
     *  All matches with a similarity above a given threshold are returned.
     *  The threshold is in the range 0..1, with 1 meaning identical signature.
     */
    QList<qlonglong> bestMatchesForImageWithThreshold(qlonglong imageid,
            double requiredPercentage, SketchType type=ScannedSketch);

    /** Calculates the Haar signature, bring it in a form as stored in the DB,
     *  and encode it to Ascii data. Can be used for bestMatchesForSignature.
     */
    QString signatureAsText(const QImage& image);

    /** For a given signature, find out the highest and lowest possible score
     *  that any other signature could reach, compared to the given signature.
     */
    void getBestAndWorstPossibleScore(Haar::SignatureData* const querySig, SketchType type,
                                      double* const lowestAndBestScore, double* const highestAndWorstScore);

    /** Fill a map of duplicates images found over a list of image to scan.
     *  For each map item, the result values is list of candidate images which are duplicates of the key image.
     *  All images are referenced by id from database.
     *  The threshold is in the range 0..1, with 1 meaning identical signature.
     */
    QMap< qlonglong, QList<qlonglong> > findDuplicates(const QSet<qlonglong>& images2Scan, double requiredPercentage,
            HaarProgressObserver* const observer = 0);

    /** Calls findDuplicates with all images in the given album ids */
    QMap< qlonglong, QList<qlonglong> > findDuplicatesInAlbums(const QList<int>& albums2Scan, double requiredPercentage,
            HaarProgressObserver* const observer = 0);

    /** Calls findDuplicates with all images in the given album and tag ids */
    QMap< qlonglong, QList<qlonglong> > findDuplicatesInAlbumsAndTags(const QList<int>& albums2Scan,
            const QList<int>& tags2Scan,
            double requiredPercentage,
            HaarProgressObserver* const observer = 0);

    /** Rebuilds the special search albums in the database that contain a list of possible candidates
     *  for duplicate images (one album per group of duplicates)
     */
    void rebuildDuplicatesAlbums(const QList<int>& albums2Scan, const QList<int>& tags2Scan,
                                 double requiredPercentage, HaarProgressObserver* const observer = 0);

    /** Retrieve the Haar signature from database using image id.
     *  Return true if item signature exist else false.
     */
    bool retrieveSignatureFromDB(qlonglong imageid, Haar::SignatureData* const sig);

    /** Give a list of albumRoots to which the search shall be limited.
     *  Calling with an empty list will disable filtering.
     */
    void setAlbumRootsToSearch(QList<int> albumRootIds);
    void setAlbumRootsToSearch(QSet<int> albumRootIds);

private:

    QImage loadQImage(const QString& filename);

    bool   indexImage(qlonglong imageid);

    QList<qlonglong> bestMatches(Haar::SignatureData* const data, int numberOfResults, SketchType type);
    QList<qlonglong> bestMatchesWithThreshold(Haar::SignatureData* const querySig,
            double requiredPercentage, SketchType type);

    QMap<qlonglong, double> searchDatabase(Haar::SignatureData* const data, SketchType type);
    double calculateScore(Haar::SignatureData& querySig, Haar::SignatureData& targetSig,
                          Haar::Weights& weights, Haar::SignatureMap** const queryMaps);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // HAARIFACE_H
