/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
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

#ifndef HAARIFACE_H
#define HAARIFACE_H

// Qt includes

#include <QString>
#include <QMap>
#include <QList>

// Local includes

#include "haar.h"
#include "digikam_export.h"

class QImage;

namespace Digikam
{

class DImg;
class ImageInfo;

class HaarProgressObserver
{
public:

    virtual ~HaarProgressObserver()
    {
    };

    virtual void totalNumberToScan(int number) = 0;
    virtual void processedNumber(int numberThatHasBeenProcessed) = 0;
    virtual bool isCanceled()
    {
        return false;
    };
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

    enum DuplicatesSearchRestrictions
    {
        None    = 0,
        SameAlbum = 1,
        DifferentAlbum = 2
    };

    enum AlbumTagRelation
    {
        NoMix          = 0,
        Union          = 1,
        Intersection   = 2,
        AlbumExclusive = 3,
        TagExclusive   = 4
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
    QList<qlonglong> bestMatchesForImage(qlonglong imageid, QList<int>& targetAlbums, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForImage(const QImage& image, QList<int>& targetAlbums, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForFile(const QString& filename, QList<int>& targetAlbums, int numberOfResults=20, SketchType type=ScannedSketch);
    QMap<qlonglong,double> bestMatchesForSignature(const QString& signature,
                                                   QList<int>& targetAlbums,
                                                   int numberOfResults=20,
                                                   SketchType type=ScannedSketch);

    /** Searches the database for the best matches for the specified query image.
     *  All matches with a similarity in a given threshold interval are returned.
     *  The threshold is in the range requiredPercentage..maximumPercentage.
     */
    QPair<double,QMap<qlonglong,double>> bestMatchesForImageWithThreshold(qlonglong imageid,
            double requiredPercentage, double maximumPercentage,
            QList<int>& targetAlbums,
            DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None, SketchType type=ScannedSketch);

    /** Searches the database for the best matches for the specified query image.
     *  All matches with a similarity in a given threshold interval are returned.
     *  The threshold is in the range requiredPercentage..maximumPercentage.
     */
    QPair<double,QMap<qlonglong,double>> bestMatchesForImageWithThreshold(const QString& imagePath,
            double requiredPercentage, double maximumPercentage,
            QList<int>& targetAlbums,
            DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None, SketchType type=ScannedSketch);

    /** Calculates the Haar signature, bring it in a form as stored in the DB,
     *  and encode it to Ascii data. Can be used for bestMatchesForSignature.
     */
    QString signatureAsText(const QImage& image);

    /** Checks whether the image with the given imageId fulfills all restrictions given in
     * targetAlbums and in respect to searchResultRestriction.
     */
    bool fulfillsRestrictions(qlonglong imageId, int albumId, qlonglong originalImageId, int originalAlbumId,
                              QList<int>& targetAlbums, DuplicatesSearchRestrictions searchResultRestriction);

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
    QMap< double,QMap< qlonglong,QList<qlonglong> > > findDuplicates(const QSet<qlonglong>& images2Scan, double requiredPercentage,
            double maximumPercentage, DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None, HaarProgressObserver* const observer = 0);

    /** Calls findDuplicates with all images in the given album ids */
    QMap< double,QMap< qlonglong,QList<qlonglong> > > findDuplicatesInAlbums(const QList<int>& albums2Scan, double requiredPercentage,
            double maximumPercentage, HaarProgressObserver* const observer = 0);

    /** Calls findDuplicates with all images in the given album and tag ids */
    QMap< double,QMap< qlonglong,QList<qlonglong> > > findDuplicatesInAlbumsAndTags(const QList<int>& albums2Scan,
            const QList<int>& tags2Scan,
            AlbumTagRelation relation,
            double requiredPercentage,
            double maximumPercentage,
            DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
            HaarProgressObserver* const observer = 0);

    /** Rebuilds the special search albums in the database that contain a list of possible candidates
     *  for duplicate images (one album per group of duplicates)
     */
    void rebuildDuplicatesAlbums(const QList<int>& albums2Scan, const QList<int>& tags2Scan, AlbumTagRelation relation,
                                 double requiredPercentage, double maximumPercentage,
                                 DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
                                 HaarProgressObserver* const observer = 0);

    /** 
     * This method rebuilds the given SAlbums by searching duplicates and replacing the SAlbums by the updated versions.
     * @param imageIds The set of images to scan for duplicates.
     * @param requiredPercentage The minimum similarity for duplicate recognition.
     * @param maximumPercentage The maximum similarity for duplicate recognition.
     * @param observer The progress observer.
     */
    void rebuildDuplicatesAlbums(const QList<qlonglong>& imageIds, double requiredPercentage, double maximumPercentage,
                                 DuplicatesSearchRestrictions searchResultRestriction = DuplicatesSearchRestrictions::None,
                                 HaarProgressObserver* const observer = 0);

    /** Retrieve the Haar signature from database using image id.
     *  Return true if item signature exist else false.
     */
    bool retrieveSignatureFromDB(qlonglong imageid, Haar::SignatureData* const sig);

    /** Give a list of albumRoots to which the search shall be limited.
     *  Calling with an empty list will disable filtering.
     */
    void setAlbumRootsToSearch(QList<int> albumRootIds);
    void setAlbumRootsToSearch(QSet<int> albumRootIds);

    /**
     * This method loads a QImage from the given filename.
     * @param filename the name of the file (path)
     * @return A QImage, non-null on success.
     */
    QImage loadQImage(const QString& filename);

private:

    bool   indexImage(qlonglong imageid);

    /**
     * This method writes the search results to the SearchXml structure.
     * @param searchResults The results to write as XML.
     */
    QMap<QString, QString> writeSAlbumQueries(QMap< double,QMap< qlonglong,QList<qlonglong> > > searchResults);

    QMultiMap<double, qlonglong> bestMatches(Haar::SignatureData* const data, int numberOfResults, QList<int>& targetAlbums, SketchType type);
    QPair<double,QMap<qlonglong,double>> bestMatchesWithThreshold(qlonglong imageid,Haar::SignatureData* const querySig,
            double requiredPercentage, double maximumPercentage, QList<int>& targetAlbums,
            DuplicatesSearchRestrictions searchResultRestriction, SketchType type);

    /**
     * This function generates the scores for all images in database.
     * @param data The signature of the original image for score calculation.
     * @param type The type of the sketch, e.g. scanned.
     * @param searchResultRestriction restrictions to apply to the generated map, i.e. None (default), same album or different album.
     * @param originalImageId the id of the original image to compare to other images. -1 is only used for sketch search.
     * @param albumId The album which images must or must not belong to (depending on searchResultRestriction).
     * @return The map of image ids and scores which fulfill the restrictions, if any.
     */
    QMap<qlonglong, double> searchDatabase(Haar::SignatureData* const data, SketchType type, QList<int>& targetAlbums,
                                           DuplicatesSearchRestrictions searchResultRestriction = None,
                                           qlonglong originalImageId = -1, int albumId = -1);
    double calculateScore(Haar::SignatureData& querySig, Haar::SignatureData& targetSig,
                          Haar::Weights& weights, Haar::SignatureMap** const queryMaps);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // HAARIFACE_H
