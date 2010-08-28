/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : libkface interface, also allowing easy manipulation of face tags
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

// Libkface Includes

#include <libkface/database.h>
#include <libkface/kface.h>

namespace Digikam
{

class FaceIface::FaceIfacePriv
{
public:

    FaceIfacePriv()
    {
        QString dbDir        = KStandardDirs::locateLocal("data", "libkface", true);
        libkface             = new KFaceIface::Database(KFaceIface::Database::InitAll, dbDir);
        tagsCache            = TagsCache::instance();

        setupTags();
    }

    ~FaceIfacePriv()
    {
        delete libkface;
    }

    bool                  suggestionsAllowed;

    double                recognitionThreshold;
    int                   detectionAccuracy;

    int                   scannedForFacesTagId;
    int                   peopleTagId;
    int                   unknownPeopleTagId;

    KFaceIface::Database* libkface;
    TagsCache*            tagsCache;
    TagProperties*        tagProps;

    void setupTags();
    QString faceTagPath(const QString& name) const;
    int  makeFaceTag(const QString& tagPath, const QString& fullName, const QString& kfaceId);
    int  findFirstTagWithProperty(const QString& property);
    int  findFirstTagWithProperty(const QString& property, const QString& value);
};

void FaceIface::FaceIfacePriv::setupTags()
{
    // Internal tag
    scannedForFacesTagId = TagsCache::instance()->getOrCreateInternalTag("Scanned for Faces"); // no i18n

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

FaceIface::FaceIface()
         : d( new FaceIfacePriv() )
{
    readConfigSettings();
}

FaceIface::~FaceIface()
{
    delete d;
}

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

QList< Face > FaceIface::findAndTagFaces(const DImg& image, qlonglong imageid, FaceRecognitionSteps todo)
{
    readConfigSettings(); //FIXME: do by signal
    // Find faces
    QImage qimg = image.copyQImage(); // FIXME: memcpy necessary?

    kDebug() << "Image dimensions : " << qimg.rect();

    KFaceIface::Image fimg(qimg);
    QList<KFaceIface::Face> faceList = d->libkface->detectFaces(fimg);

    // Mark the image as scanned.
    /*
    ImageTagPair pairScanned(imageid, d->scannedForFacesTagId);
    pairScanned.addProperty(d->imageTagPropertyNumberOfFaces, QString::number(faceList.size()));
    pairScanned.assignTag();
    */
    markAsScanned(imageid);

    // Apply region tags to the image for each face that has been detected.
    QListIterator<KFaceIface::Face> it(faceList);

    //TODO: Recognition!

    while (it.hasNext())
    {
        const KFaceIface::Face& face = it.next();

        // Assign the "/People/Unknown" tag. The property for this tag is "faceRegion", which has an SVG rect associated with it.
        // When we assign a name for a person in the image, we assign a new tag "/People/<Person Name>" to this image, and move the
        // "faceRegion" property to this tag, and delete it from the unknown tag."
        // See README.FACE for a nicer explanation.

        int tagId = tagForFaceName(face.name());

        ImageTagPair pair(imageid, tagId);
        QRect faceRect = face.toRect();
        if (!faceRect.isValid())
            continue;
        QString region = TagRegion(faceRect).toXml();

        kDebug() << "Applying face tag" << tagId << face.name() << faceRect << region;
        pair.addProperty(ImageTagPropertyName::autodetectedFace(), region);
    }

    return faceList;
}

QList< Face > FaceIface::findFacesFromTags(const DImg& image, qlonglong imageid)
{
    if (!hasBeenScanned(imageid))
    {
        kDebug() << "Image has not been scanned yet.";
        return QList<Face>();
    }

    QList<Face> faceList;

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    kDebug() << imageid << pairs.size();
    foreach (const ImageTagPair& pair, pairs)
    {
        kDebug() << pair.tagId() << pair.properties();
        if (!isPerson(pair.tagId()))
            continue;

        if (!pair.hasProperty(ImageTagPropertyName::tagRegion()) && !pair.hasProperty(ImageTagPropertyName::autodetectedFace()) )
            continue;

        foreach (const QString& rectString, pair.values(ImageTagPropertyName::tagRegion()) + pair.values(ImageTagPropertyName::autodetectedFace()))
        {
            QRect rect = TagRegion(rectString).toRect();
            kDebug()<<"rect found as "<< rect;

            if (!rect.isValid())
                continue;

            Face f;
            f.setRect(rect);
            // FIXME: Later, need support for putting the cropped image in Face f too. I tried it, but I get weird crashes. Will see.
            //f.setImage(DImg.copyQImage(rect);

            if (pair.tagId() != d->unknownPeopleTagId)
                f.setName(faceNameForTag(pair.tagId()));

            faceList += f;
        }
    }

    return faceList;
}

void FaceIface::forgetFaceTags(qlonglong imageid)
{
    ImageInfo info(imageid);

    if (!hasBeenScanned(info))
    {
        kDebug() << "Image has not been scanned yet.";
        return;
    }


    // Remove the "scanned for faces" tag.
    markAsScanned(info, false);
    /*
    ImageTagPair pair1(imageid, d->scannedForFacesTagId);
    pair1.removeProperties(d->imageTagPropertyNumberOfFaces);
    pair1.unAssignTag();
    */

    QList<ImageTagPair> pairs = ImageTagPair::availablePairs(imageid);
    QList<int> tagsToRemove;

    for (int i=0; i<pairs.size(); i++)
    {
        ImageTagPair& pair = pairs[i];
        if (!pair.hasProperty(ImageTagPropertyName::tagRegion()) || !isPerson(pair.tagId()))
            continue;
        pair.removeProperties(ImageTagPropertyName::tagRegion());
        pair.removeProperties(ImageTagPropertyName::autodetectedFace());
        tagsToRemove << pair.tagId();
    }

    // When we unassign a tag, this can include changing metadata!
    // FIXME: Check use case. Do we really need this?
    MetadataManager::instance()->removeTags(info, tagsToRemove);
}

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

            QString nameString = d->tagsCache->tagName(tagId);
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


QList< QRect > FaceIface::getTagRects(qlonglong imageid)
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

QString FaceIface::getNameForRect(qlonglong imageid, const QRect &faceRect) const
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

int FaceIface::numberOfFaces(qlonglong imageid)
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

QList< int > FaceIface::allPersonTags()
{
    AlbumList candidates = AlbumManager::instance()->findTagsWithProperty(TagPropertyName::person());
    QList <int> peopleTagIds;
    foreach (Album *a, candidates)
        peopleTagIds << a->id();
    //peopleTagIds += d->peopleTagId;

    return peopleTagIds;
}

QList< QString > FaceIface::allPersonNames()
{
    return d->tagsCache->tagNames(allPersonTags());
}

QList< QString > FaceIface::allPersonPaths()
{
    return d->tagsCache->tagPaths(allPersonTags());
}

int FaceIface::tagForPerson(const QString& name, const QString &fullName)
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

int FaceIface::getOrCreateTagForPerson(const QString& name, const QString &givenFullName)
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

bool FaceIface::isPerson ( int tagId )
{
    TAlbum *talbum = AlbumManager::instance()->findTAlbum(tagId);
    return talbum && talbum->hasProperty(TagPropertyName::person());
}

int FaceIface::setName ( qlonglong imageid, const QRect& rect, const QString& name )
{
    // First look in all people tags. If the name is already there, then put it there. For ex, if the name of
    // your "dad" is a subtag of "Family" in the People tags, then assigning a tag "dad" to a person should assign the
    // tag under "Family", and not create a new tag just below "People".

    int nameTagId = getOrCreateTagForPerson(name);
    QString region = TagRegion(rect).toXml();

    ImageTagPair pairUnknown ( imageid, d->unknownPeopleTagId);

    ImageTagPair pairNamed   ( imageid, nameTagId );

    pairNamed.removeProperty(ImageTagPropertyName::autodetectedFace(), region);
    pairNamed.setProperty(ImageTagPropertyName::tagRegion(), region);
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

void FaceIface::removeRect ( qlonglong imageid, const QRect& rect , const QString& name)
{
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

int FaceIface::faceCountForPersonInImage ( qlonglong imageid, int tagId )
{
    ImageTagPair pair(imageid, tagId);
    return pair.values(ImageTagPropertyName::tagRegion()).size();
}

int FaceIface::tagForFaceName(const QString& kfaceId)
{
    if (kfaceId.isEmpty())
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
    TAlbum *album = AlbumManager::instance()->findTAlbum(tagId);
    if (!album)
        return QString();

    QString id = album->property(TagPropertyName::kfaceId());
    if (id.isNull())
        id = album->property(TagPropertyName::person());
    if (id.isNull())
        id = album->title();
    return id;
}

Face FaceIface::faceForRectInImage ( qlonglong imageid, const QRect& rect, const QString& name )
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

void FaceIface::trainWithFace(const Face& face)
{
    d->libkface->updateFaces(QList<Face>()+=face);
}

void FaceIface::trainWithFaces (const QList< Face >& givenFaceList )
{
    readConfigSettings(); //TODO: do by signal

    QList<Face> faceList = givenFaceList;
    for (int i=0; i<faceList.size(); i++)
    {
        const Face& face = faceList[i];
        if (face.name().isEmpty() || face.getImage().isNull())
            faceList.removeAt(i);
    }

    d->libkface->updateFaces(faceList);
    kDebug() << "DB file is : " << d->libkface->configPath();
    d->libkface->saveConfig();
}

QString FaceIface::recognizedName (const KFaceIface::Face& face )
{
    if(d->libkface->peopleCount() == 0)
        return QString();

    QList<Face> f;
    f.append(face);

    if( face.getImage().isNull() )
        return QString();
    else 
    {
        // TODO: in libkface, store the distance in the Face object, if the name is stored there?
        double distance = d->libkface->recognizeFaces(f).at(0)/100000;
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

void FaceIface::readConfigSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Face Tags Settings");

    d->suggestionsAllowed     = group.readEntry("FaceSuggestion", false);
    d->detectionAccuracy      = group.readEntry("DetectionAccuracy", 3);
    d->recognitionThreshold   = 1 - group.readEntry("SuggestionThreshold", 0.2);

    d->libkface->setDetectionAccuracy(d->detectionAccuracy);
}

} // Namespace Digikam
