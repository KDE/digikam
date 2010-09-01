/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : database interface, also allowing easy manipulation of face tags
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "faceiface.h"

// Qt includes

#include <QImage>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Libkface Includes

#include <libkface/database.h>
#include <libkface/kface.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "imageinfocache.h"
#include "databaseaccess.h"
#include "databaseconstants.h"
#include "databasetransaction.h"
#include "databasebackend.h"
#include "dimg.h"
#include "imageinfo.h"
#include "imagetagpair.h"
#include "metadatamanager.h"
#include "searchxml.h"
#include "sqlquery.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "tagregion.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class FaceIface::FaceIfacePriv
{
public:

    FaceIfacePriv()
    {
        kfaceDatabase        = 0; // created on demand
        thumbnailLoadThread  = 0;

        setupTags();
    }

    ~FaceIfacePriv()
    {
        delete kfaceDatabase;
        delete thumbnailLoadThread;
    }

    bool                 suggestionsAllowed;

    double               recognitionThreshold;
    int                  detectionAccuracy;

    int                  scannedForFacesTagId;
    int                  needToTrainFacesTagId;
    int                  peopleTagId;
    int                  unknownPeopleTagId;

    ThumbnailLoadThread* thumbnailLoadThread;

public:

    /// Creates a new object if needed
    KFaceIface::Database* database();
    /// Gives current pointer, never creates new object
    const KFaceIface::Database* databaseConst() const;

    void setupTags();
    void checkThumbnailThread();
    QString faceTagPath(const QString& name) const;
    int  makeFaceTag(const QString& tagPath, const QString& fullName, const QString& kfaceId);
    int  findFirstTagWithProperty(const QString& property);
    int  findFirstTagWithProperty(const QString& property, const QString& value);

private:

    KFaceIface::Database* kfaceDatabase;
};

// --- Private methods, face tag utility methods

KFaceIface::Database* FaceIface::FaceIfacePriv::database()
{
    if (!kfaceDatabase)
    {
        QString dbDir = KStandardDirs::locateLocal("data", "database/", true);
        kfaceDatabase = new KFaceIface::Database(KFaceIface::Database::InitAll, dbDir);
    }
    return kfaceDatabase;
}

const KFaceIface::Database* FaceIface::FaceIfacePriv::databaseConst() const
{
    return kfaceDatabase;
}

void FaceIface::FaceIfacePriv::setupTags()
{
    // Internal tag
    scannedForFacesTagId = TagsCache::instance()->getOrCreateInternalTag("Scanned for Faces"); // no i18n
    needToTrainFacesTagId = TagsCache::instance()->getOrCreateInternalTag("Need to Train Faces"); // no i18n

    // Faces parent tag
    peopleTagId          = TagsCache::instance()->getOrCreateTag(i18nc("People on your photos", "People"));

    // Unknown people tag
    unknownPeopleTagId = findFirstTagWithProperty(TagPropertyName::unknownPerson());
    if (!unknownPeopleTagId)
    {
        unknownPeopleTagId = TagsCache::instance()->getOrCreateTag(faceTagPath(i18n("Unknown")));
        TagProperties props(unknownPeopleTagId);
        props.setProperty(TagPropertyName::person(), QString()); // no name associated
        props.setProperty(TagPropertyName::unknownPerson(), QString()); // special property
    }
}

void FaceIface::FaceIfacePriv::checkThumbnailThread()
{
    if (!thumbnailLoadThread)
    {
        thumbnailLoadThread = new ThumbnailLoadThread;
        thumbnailLoadThread->setPixmapRequested(false);
        //TODO d->thumbnailLoadThread->setExifRotate(d->exifRotate);
    }
}

int FaceIface::FaceIfacePriv::findFirstTagWithProperty(const QString& property)
{
    AlbumList candidates = AlbumManager::instance()->findTagsWithProperty(property);
    if (!candidates.isEmpty())
        unknownPeopleTagId = candidates.first()->id();
    return 0;
}

int FaceIface::FaceIfacePriv::findFirstTagWithProperty(const QString& property, const QString& value)
{
    AlbumList candidates = AlbumManager::instance()->findTagsWithProperty(property, value);
    if (!candidates.isEmpty())
        unknownPeopleTagId = candidates.first()->id();
    return 0;
}

QString FaceIface::FaceIfacePriv::faceTagPath(const QString& name) const
{
    QString faceParentTagName = TagsCache::instance()->tagName(peopleTagId);
    return faceParentTagName + '/' + name;
}

int FaceIface::FaceIfacePriv::makeFaceTag(const QString& tagPath, const QString& fullName, const QString& kfaceId)
{
    int tagId = TagsCache::instance()->getOrCreateTag(tagPath);
    TagProperties props(tagId);
    props.setProperty(TagPropertyName::person(), fullName);
    props.setProperty(TagPropertyName::kfaceId(), kfaceId);
    return tagId;
}

// --- Constructor / Destructor

FaceIface::FaceIface()
         : d( new FaceIfacePriv() )
{
    readConfigSettings();
}

FaceIface::~FaceIface()
{
    delete d;
}

// --- Mark for scanning and training ---

bool FaceIface::hasBeenScanned(qlonglong imageid) const
{
    return hasBeenScanned(ImageInfo(imageid));
}

bool FaceIface::hasBeenScanned(const ImageInfo& info) const
{
    return info.tagIds().contains(d->scannedForFacesTagId);
}

void FaceIface::markAsScanned(qlonglong imageid, bool hasBeenScanned) const
{
    return markAsScanned(ImageInfo(imageid), hasBeenScanned);
}

void FaceIface::markAsScanned(const ImageInfo& info, bool hasBeenScanned) const
{
    if (hasBeenScanned)
        ImageInfo(info).setTag(d->scannedForFacesTagId);
    else
        ImageInfo(info).removeTag(d->scannedForFacesTagId);
}

bool FaceIface::needsTraining(qlonglong imageid) const
{
    return needsTraining(ImageInfo(imageid));
}

bool FaceIface::needsTraining(const ImageInfo& info) const
{
    return info.tagIds().contains(d->needToTrainFacesTagId);
}

void FaceIface::markForTraining(qlonglong imageid, bool hasBeenScanned) const
{
    return markForTraining(ImageInfo(imageid), hasBeenScanned);
}

void FaceIface::markForTraining(const ImageInfo& info, bool hasBeenScanned) const
{
    if (hasBeenScanned)
        ImageInfo(info).setTag(d->needToTrainFacesTagId);
    else
        ImageInfo(info).removeTag(d->needToTrainFacesTagId);
}

// --- Face tags ---

int FaceIface::tagForFaceName(const QString& kfaceId) const
{
    if (kfaceId.isNull())
        return d->unknownPeopleTagId;

    // Find in "kfaceId" attribute
    int tagId = d->findFirstTagWithProperty(TagPropertyName::kfaceId(), kfaceId);
    if (tagId)
        return tagId;

    // First find by full name or name. If not, id is in libface's database, but not in ours, so create.
    return getOrCreateTagForPerson(kfaceId);
}

QString FaceIface::faceNameForTag(int tagId) const
{
    TAlbum* album = AlbumManager::instance()->findTAlbum(tagId);
    if (!album)
        return QString();

    QString id = album->property(TagPropertyName::kfaceId());
    if (id.isNull())
        id = album->property(TagPropertyName::person());
    if (id.isNull())
        id = album->title();
    return id;
}

QList< int > FaceIface::allPersonTags() const
{
    AlbumList candidates = AlbumManager::instance()->findTagsWithProperty(TagPropertyName::person());
    QList <int> peopleTagIds;
    foreach (Album* a, candidates)
        peopleTagIds << a->id();
    //peopleTagIds += d->peopleTagId;

    return peopleTagIds;
}

QList< QString > FaceIface::allPersonNames() const
{
    return TagsCache::instance()->tagNames(allPersonTags());
}

QList< QString > FaceIface::allPersonPaths() const
{
    return TagsCache::instance()->tagPaths(allPersonTags());
}

int FaceIface::tagForPerson(const QString& name, const QString &fullName) const
{
    if (name.isNull() && fullName.isNull())
        return d->unknownPeopleTagId;

    // First attempt: Find by full name in "person" attribute
    int tagId = d->findFirstTagWithProperty(TagPropertyName::person(), fullName.isNull() ? name : fullName);
    if (!tagId)
        return tagId;

    // Second attempt: Find by tag name
    QList<int> tagList = TagsCache::instance()->tagsForName(name);
    foreach(int id, tagList)
    {
        if (isPerson(id))
           return id;
    }

    return 0;
}

int FaceIface::getOrCreateTagForPerson(const QString& name, const QString &givenFullName) const
{
    int tagId = tagForPerson(name, givenFullName);
    if (tagId)
        return tagId;

    QString fullName = givenFullName.isNull() ? name : givenFullName;
    QString kfaceId  = fullName;
    for (int i=0; d->findFirstTagWithProperty(TagPropertyName::kfaceId(), kfaceId); ++i)
        kfaceId = fullName + QString(" (%1)").arg(i);
    return d->makeFaceTag(d->faceTagPath(name), fullName, kfaceId);
}

bool FaceIface::isPerson ( int tagId ) const
{
    TAlbum* talbum = AlbumManager::instance()->findTAlbum(tagId);
    return talbum && talbum->hasProperty(TagPropertyName::person());
}

// --- Read from database ---

int FaceIface::faceCountForPersonInImage ( qlonglong imageid, int tagId ) const
{
    ImageTagPair pair(imageid, tagId);
    return pair.values(ImageTagPropertyName::tagRegion()).size();
}

QList< Face > FaceIface::findFacesFromTags(const DImg& image, qlonglong imageid) const
{
    if (!hasBeenScanned(imageid))
    {
        kDebug() << "Image has not been scanned yet.";
        return QList<Face>();
    }

    QList<Face> faceList = findFaces(imageid, QStringList()
                                     << ImageTagPropertyName::tagRegion()
                                     << ImageTagPropertyName::autodetectedFace());

    fillImageInFaces(image, faceList);

    return faceList;
}


QList< QRect > FaceIface::getTagRects(qlonglong imageid) const
{
    QList< QRect > rectList;

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    foreach (const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        foreach (const QString& region, regions)
        {
            QRect rect = TagRegion(region).toRect();
            if (rect.isValid())
                rectList << rect;
        }
    }

    return rectList;
}

/*QString FaceIface::getNameForRect(qlonglong imageid, const QRect &faceRect) const
{
    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    QString regionRect = TagRegion(faceRect).toXml();
    foreach(const ImageTagPair& pair, pairs)
        {
            QMap<QString, QString>  props = pair.properties();

            QMapIterator<QString, QString> i(props);
            while (i.hasNext())
            {
                i.next();
                if(i.value() == regionRect)
                    {
                        QString ret = faceNameForTag(pair.tagId());
                        return ret == "Unknown" ? QString() : ret;
                    }
            }
        }

    return QString();
}
*/

int FaceIface::numberOfFaces(qlonglong imageid) const
{
    // Use case for this? Depending on a use case, we can think of an optimization
    int count = 0;

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    foreach (const ImageTagPair& pair, pairs)
    {
        QStringList regions = pair.values(ImageTagPropertyName::tagRegion());
        count += regions.size();
    }

    return count;
}

Face FaceIface::faceForRectInImage ( qlonglong imageid, const QRect& rect, const QString& name ) const
{
    Face f;
    ImageInfo info(imageid);

    DImg image(info.filePath());
    QImage qimg = image.copyQImage();

    f.setRect(rect);
    f.setName(name);
    f.setImage(qimg.copy(rect));

    return f;
}

void FaceIface::fillImageInFaces(const DImg& image, QList<Face> &faceList) const
{
    for (int i=0; i<faceList.size(); i++)
    {
        Face &f = faceList[i];
        QRect rect = f.toRect();
        if (rect.isValid())
            f.setImage(image.copyQImage(rect));
    }
}

typedef QPair<int, QVariant> TagIdAndTagRegion;

QList< Face > FaceIface::findFaces(qlonglong imageid, const QList<QString>& attributes) const
{
    if (!hasBeenScanned(imageid))
    {
        kDebug() << "Image has not been scanned yet.";
        return QList<Face>();
    }

    QList<TagIdAndTagRegion> regions = findFaceRegions(imageid, attributes);

    return findFaces(regions);
}

QList< Face > FaceIface::findFaces(const QList<QPair<int, QVariant> >& faceRegions) const
{
    QList<Face> faceList;
    foreach (const TagIdAndTagRegion& region, faceRegions)
    {
        QRect rect = region.second.toRect();

        if (!rect.isValid())
            continue;

        Face f;
        f.setRect(rect);
        // FIXME: Later, need support for putting the cropped image in Face f too. I tried it, but I get weird crashes. Will see.
        //f.setImage(DImg.copyQImage(rect);

        if (region.first != d->unknownPeopleTagId)
            f.setName(faceNameForTag(region.first));

        faceList += f;
    }

    return faceList;
}

QList<QPair<int, QVariant> > FaceIface::findFaceRegions(qlonglong imageid, const QList<QString>& attributes) const
{
    QList<QPair<int, QVariant> > regions;
    foreach (const ImageTagPair& pair, findFaceImageTagPairs(imageid, attributes))
    {
        foreach (const QString& rectString, pair.allValues(attributes))
        {
            QRect rect = TagRegion(rectString).toRect();
            kDebug()<<"rect found as "<< rect;

            if (!rect.isValid())
                continue;

            regions << QPair<int, QVariant>(pair.tagId(), rect);
        }
    }

    return regions;
}

QList<ImageTagPair> FaceIface::findFaceImageTagPairs(qlonglong imageid, const QList<QString>& attributes) const
{
    QList<ImageTagPair> pairs;

    foreach (const ImageTagPair& pair, ImageTagPair::availablePairs(imageid))
    {
        //kDebug() << pair.tagId() << pair.properties();
        if (!isPerson(pair.tagId()))
            continue;

        if (!pair.hasAnyProperty(attributes))
            continue;

        pairs << pair;
    }

    return pairs;
}

// --- Face detection and recognition ---

QList< Face > FaceIface::findAndTagFaces(const DImg& image, qlonglong imageid, FaceRecognitionSteps todo)
{
    readConfigSettings(); //FIXME: do by signal

    QImage qimg = image.copyQImage(); // FIXME: memcpy necessary?
    kDebug() << "Image dimensions : " << qimg.rect();
    KFaceIface::Image fimg(qimg);

    // -- Detection --

    QList<KFaceIface::Face> faceList = d->database()->detectFaces(fimg);
    // Mark the image as scanned.
    markAsScanned(imageid);

    // -- Recognition --

    QList<double> distances;
    if (d->database()->peopleCount() && todo >= DetectAndRecognize)
    {
        distances = d->database()->recognizeFaces(faceList);
        for (int i=0; i<faceList.size(); i++)
        {
            if (distances[i] > 100)
            {
                // Dismissing
                faceList[i].clearRecognition();
            }
        }
    }

    // -- Apply tags --

    QListIterator<KFaceIface::Face> it(faceList);
    while (it.hasNext())
    {
        const KFaceIface::Face& face = it.next();

        // Assign the "/People/Unknown" tag. The property for this tag is "faceRegion", which has an SVG rect associated with it.
        // When we assign a name for a person in the image, we assign a new tag "/People/<Person Name>" to this image, and move the
        // "faceRegion" property to this tag, and delete it from the unknown tag."
        // See README.FACE for a nicer explanation.

        // We'll get the unknownPeopleTagId if face.name() is null
        int tagId = tagForFaceName(face.name());

        ImageTagPair pair(imageid, tagId);
        QRect faceRect = face.toRect();
        if (!faceRect.isValid())
            continue;
        QRect fullSizeRect = TagRegion::mapToOriginalSize(image, faceRect);
        QString region = TagRegion(fullSizeRect).toXml();

        kDebug() << "Applying face tag" << tagId << face.name() << faceRect << region;
        pair.addProperty(ImageTagPropertyName::autodetectedFace(), region);
    }

    return faceList;
}

QString FaceIface::recognizeFace(const KFaceIface::Face& face)
{
    if(d->database()->peopleCount() == 0)
        return QString();

    QList<Face> f;
    f.append(face);

    if( face.image().isNull() )
        return QString();
    else
    {
        double distance = d->database()->recognizeFaces(f).at(0)/100000;
        kDebug() << "Possible : " << f[0].name();
        kDebug() << "Distance is " << distance;

        if(distance > 100)
        {
            kDebug() << "Dismissing ";
            return QString();
        }
        else
        {
            return f[0].name();
        }
    }
}

// --- Confirmation ---

int FaceIface::confirmName( qlonglong imageid, const QRect& rect, const QString& name )
{
    // First look in all people tags. If the name is already there, then put it there. For ex, if the name of
    // your "dad" is a subtag of "Family" in the People tags, then assigning a tag "dad" to a person should assign the
    // tag under "Family", and not create a new tag just below "People".

    int nameTagId  = getOrCreateTagForPerson(name);
    QString region = TagRegion(rect).toXml();

    ImageTagPair pairUnknown ( imageid, d->unknownPeopleTagId);

    ImageTagPair pairNamed   ( imageid, nameTagId );

    pairNamed.removeProperty(ImageTagPropertyName::autodetectedFace(), region);
    pairNamed.setProperty(ImageTagPropertyName::tagRegion(), region);

    markForTraining(imageid);
    pairNamed.setProperty(ImageTagPropertyName::faceToTrain(), region);

    MetadataManager::instance()->assignTag(ImageInfo(imageid), nameTagId);

    /*
    if(faceCountForPersonInImage(imageid, d->unknownPeopleTagId) == 0)
    {
            ImageTagPair pair(imageid, d->unknownPeopleTagId);
            pair.removeProperties(ImageTagPropertyName::tagRegion());
            pair.removeProperties("face");
            pair.unAssignTag();
    }
    */

    return nameTagId;
}

// --- Training ---

void FaceIface::trainImages(const QList<ImageInfo>& imageInfos)
{
    d->checkThumbnailThread();

    ThumbnailImageCatcher catcher(d->thumbnailLoadThread);

    typedef QHash<ImageInfo, QList<TagIdAndTagRegion> > ImageInfoToTagIdAndTagRegionHash;
    ImageInfoToTagIdAndTagRegionHash regionsTrained;
    QList<Face> facesToTrain;

    foreach (const ImageInfo& info, imageInfos)
    {
        QString filePath = info.filePath();
        if (filePath.isNull())
            continue;


        QList<TagIdAndTagRegion> regions = findFaceRegions(info.id(), QStringList() << ImageTagPropertyName::faceToTrain());
        QList<Face> faces = findFaces(regions);

        for (int i=0; i<faces.size(); ++i)
        {
            Face &face = faces[i];
            d->thumbnailLoadThread->find(filePath, face.toRect(), ThumbnailLoadThread::maximumThumbnailSize());
            QImage image = catcher.waitForThumbnail(filePath);
            face.setImage(image);
            facesToTrain << face;
            regionsTrained[info] << regions[i];
        }
    }

    trainFaces(facesToTrain);

    // Ok, what's that? We have a list of images. For each image, we have a list of face
    // regions, each with a tag id and a region. For each region (with associated image id and tag id)
    // we remove the faceToTrain() image tag property.
    // Then, per-image, we remove the global tag marking it for training.
    // As we loaded all regions above, we dont need to check that none is left.
    ImageInfoToTagIdAndTagRegionHash::iterator it;
    for (it = regionsTrained.begin(); it != regionsTrained.end(); ++it)
    {
        foreach (const TagIdAndTagRegion& regionsTrained, it.value())
        {
            // possible optimization: only one pair per tag id
            ImageTagPair pair(it.key().id(), regionsTrained.first);
            pair.removeProperty(ImageTagPropertyName::faceToTrain(), TagRegion(regionsTrained.second.toRect()).toXml());
        }
        markForTraining(it.key(), false);
    }
}

void FaceIface::trainFace(const Face& face)
{
    trainFaces(QList<Face>() << face);
}

void FaceIface::trainFaces(const QList< Face >& givenFaceList )
{
    readConfigSettings(); //TODO: do by signal

    QList<Face> faceList = givenFaceList;
    for (int i=0; i<faceList.size(); i++)
    {
        const Face& face = faceList[i];
        if (face.name().isEmpty() || face.image().isNull())
            faceList.removeAt(i);
    }

    d->database()->updateFaces(faceList);
    kDebug() << "DB file is : " << d->database()->configPath();
    d->database()->saveConfig();
}

// --- Clear face entries ---

void FaceIface::removeAllFaces(qlonglong imageid)
{
    ImageInfo info(imageid);

    // Remove the "scanned for faces" tag.
    markAsScanned(info, false);
    markForTraining(info, false);

    QStringList attributes;
    attributes << ImageTagPropertyName::tagRegion()
               << ImageTagPropertyName::autodetectedFace()
               << ImageTagPropertyName::faceToTrain();

    QList<int> tagsToRemove;
    foreach (ImageTagPair pair, findFaceImageTagPairs(imageid, attributes))
    {
        pair.clearProperties();
        if (pair.isAssigned())
            tagsToRemove << pair.tagId();
   }

    MetadataManager::instance()->removeTags(info, tagsToRemove);
}

int FaceIface::removeFace(qlonglong imageid, const QRect& rect)
{
    QStringList attributes;
    attributes << ImageTagPropertyName::tagRegion()
               << ImageTagPropertyName::autodetectedFace()
               << ImageTagPropertyName::faceToTrain();
    QString regionString = TagRegion(rect).toXml();

    foreach (ImageTagPair pair, findFaceImageTagPairs(imageid, attributes))
    {
        foreach (const QString& rectString, pair.allValues(attributes))
        {
            if (rectString == regionString)
            {
                pair.clearProperties();
                if (pair.isAssigned())
                    MetadataManager::instance()->removeTag(ImageInfo(imageid), pair.tagId());
                return pair.tagId();
            }
        }
    }
    return -1;

    /*
    QList<QPair<int, QVariant> > findFaceRegions(imageid, const QList<QString>& attributes) const
    QString region = TagRegion(rect).toXml();
    if(name.isEmpty())
    {
        kDebug()<<"Removing the unknown property";
        ImageTagPair pairUnknown ( imageid, d->unknownPeopleTagId);
        pairUnknown.removeProperty(ImageTagPropertyName::tagRegion(), region);

        if (!pairUnknown.hasProperty(ImageTagPropertyName::tagRegion()))
            MetadataManager::instance()->removeTag(ImageInfo(imageid), pairUnknown.tagId());
    }
    else
    {
        int nameTagId = tagForPerson(name);
        kDebug()<<"Removing person "<<name;
        if (!nameTagId)
        {
            kWarning() << "Request to remove" << name << rect << "from" << imageid << ": No tag for this name!";
            return;
        }

        ImageTagPair pairNamed   ( imageid, nameTagId );

        pairNamed.removeProperty(ImageTagPropertyName::tagRegion(), region);
        // if we removed the last tagRegion of this tag, remove the tag as well
        if (!pairNamed.hasProperty(ImageTagPropertyName::tagRegion()))
            MetadataManager::instance()->removeTag(ImageInfo(imageid), pairNamed.tagId());
    }
    */

    /*
    if(faceCountForPersonInImage(imageid, d->unknownPeopleTagId) == 0)
    {
            ImageTagPair pair(imageid, d->unknownPeopleTagId);
            pair.removeProperties(ImageTagPropertyName::tagRegion());
            pair.removeProperties("face");
            pair.unAssignTag();
    }
    */
}

void FaceIface::readConfigSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Face Tags Settings");

    d->suggestionsAllowed     = group.readEntry("FaceSuggestion", false);
    d->detectionAccuracy      = group.readEntry("DetectionAccuracy", 3);
    d->recognitionThreshold   = 1 - group.readEntry("SuggestionThreshold", 0.2);

    if (d->databaseConst())
        d->database()->setDetectionAccuracy(d->detectionAccuracy);
}

    /**
     * Returns a list of image ids of all images in the DB which have a specified person within.
     * @param tagId The person's id. Or tag id for the person tag. Same thing.
     * @param repeat If repeat is specified as true, then if person with id tagId is found n times in an image,
     * that image's id will be pushed into the returned list n times. Useful for thumbnailing as I see it.
     */
    //QList<qlonglong>    imagesWithPerson(int tagId, bool repeat = false);

/*
What is the use case? This method will be extremely slow.
For listing, we must use a search.
QList< qlonglong > FaceIface::imagesWithPerson(int tagId, bool repeat)
{
    QList<qlonglong> imageIds;
    const AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    QStringList pathList;

    for (AlbumList::ConstIterator it = palbumList.constBegin();
            it != palbumList.constEnd(); ++it)
    {
        pathList += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

    for (QStringList::ConstIterator i = pathList.constBegin();
            i != pathList.constEnd(); ++i)
    {
        ImageInfo info(*i);

        if (this->hasBeenScanned(info.id()))
        {
            kDebug() << "Image " << info.filePath() << "has already been scanned.";

            //dddddd
            DImg img;
            QList<Face> faceList = findFacesFromTags(img, info.id());

            QString nameString = TagsCache::instance()->tagName(tagId);
            if(nameString == QString("Unknown"))
                nameString = QString();    // Because we store names for unknown faces this way in the Face objects.

            QListIterator<Face> faceIt(faceList);
            // The number of times the same face was found in this image
            int count = 0;

            while(faceIt.hasNext())
            {
                if(faceIt.next().name() == nameString)
                {
                    count++;
                    // If we have been told to repeat the image n times if the face was found n times, then do so.
                    if(repeat)
                        imageIds += info.id();
                    // If we have not been told to repeat, then only append the id if the count is 1
                    else
                    {
                        if(count == 1)
                            imageIds += info.id();
                    }
                }
            }
        }
    }

    return imageIds;
}
*/


    /**
     * Method to mark a face as trained, so that we don't retrain it
     */
//     void                markFaceAsTrained(qlonglong imageid, const QRect& rect, const QString& name);
//     void                markFaceAsTrained(qlonglong imageid, const Face& face);

    /**
     * Method to mark a list of faces as trained, so that we don't retrain them
     */
//     void                markFacesAsTrained(qlonglong imageid, QList<Face> faces);

    /**
     * Method to mark a face as recognized, so we don't attempt to recognize it again
     */
//     void                markFaceAsRecognized(qlonglong imageid, const QRect& rect, const QString& name);
//     void                markFaceAsRecognized(qlonglong imageid, const Face& face);

    /**
     * Method to mark a list of faces as recognized, so that we don't attempt to identify them again
     */
//     void                markFacesAsRecognized(qlonglong imageid, QList<Face> faces);

    /**
     * Tells if the face has been marked as Trained
     */
//     bool                isFaceTrained(qlonglong imageid, const QRect& rect, const QString& name);

    /**
     * Tells if the face has been marked as recognized
     */
//     bool                isFaceRecognized( qlonglong imageid, const QRect& rect, const QString& name);
/*
void FaceIface::markFaceAsTrained ( qlonglong imageid, const QRect& rect, const QString& name )
{
    if(name.isNull() || name == "Unknown")
        return;

    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );
    pair.addProperty("trained", TagRegion(rect).toXml());
    pair.assignTag();

    kDebug() << "Marked face of " << name << " as trained";
}

void FaceIface::markFaceAsTrained ( qlonglong imageid, const KFaceIface::Face& face )
{
    this->markFaceAsTrained(imageid, face.toRect(), face.name());
}

void FaceIface::markFacesAsTrained ( qlonglong imageid, QList< Face > faces )
{
    foreach(Face f, faces)
    {
        this->markFaceAsTrained(imageid, f.toRect(), f.name());
    }
}

void FaceIface::markFaceAsRecognized ( qlonglong imageid, const QRect& rect, const QString& name )
{
    if(name == QString("Unknown") || name == QString())
        return;

    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );
    pair.addProperty("recognized", TagRegion(rect).toXml());
    pair.assignTag();
    kDebug() << "Marked face of " << name << " as recognized";
}

void FaceIface::markFaceAsRecognized ( qlonglong imageid, const KFaceIface::Face& face )
{
    this->markFaceAsRecognized(imageid, face.toRect(), face.name());
}

void FaceIface::markFacesAsRecognized ( qlonglong imageid, QList< Face > faces )
{
    foreach(Face f, faces)
    {
        this->markFaceAsRecognized(imageid, f.toRect(), f.name());
    }
}

bool FaceIface::isFaceTrained ( qlonglong imageid, const QRect& rect, const QString& name )
{
    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );

    if(pair.values("trained").contains(TagRegion(rect).toXml()))
    {
        if(name == QString("Unknown") || name == QString())
        {
            DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "trained", TagRegion(rect).toXml());
            return false;
        }
        else
        {
            return true;
	}
    }

    return false;
}

bool FaceIface::isFaceRecognized ( qlonglong imageid, const QRect& rect, const QString& name )
{
    int nameTagId = tagForPerson ( name );
    ImageTagPair pair ( imageid, nameTagId );

    if(pair.values("recognized").contains(TagRegion(rect).toXml()))
    {
        if(name == QString("Unknown") || name == QString())
        {
            DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "recognized", TagRegion(rect).toXml());
            return false;
        }
        else
        {
            return true;
	}
    }

    return false;
}
*/

} // Namespace Digikam
