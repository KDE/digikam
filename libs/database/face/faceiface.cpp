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
#include <QtSvg>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "imageinfo.h"
#include "databaseaccess.h"
#include "databasetransaction.h"
#include "albumdb.h"
#include "databasebackend.h"
#include "searchxml.h"
#include "sqlquery.h"
#include "tagscache.h"
#include "imagetagpair.h"
#include "albummanager.h"
#include "dimg.h"
#include "album.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "tagproperties.h"

// Libkface Includes

#include <libkface/database.h>
#include <libkface/kface.h>
#include <imageinfocache.h>

namespace Digikam
{

class FaceIfacePriv
{
public:
    
    FaceIfacePriv()
    {
        libkface             = new KFaceIface::Database(KFaceIface::Database::InitAll, KStandardDirs::locateLocal("data", "libkface"));
        tagsCache            = TagsCache::instance();
        scannedForFacesTagId = tagsCache->createTag("/Scanned/Scanned for Faces");
        peopleTagId          = tagsCache->createTag("/People");
        unknownPeopleTagId   = tagsCache->createTag("/People/Unknown");
        tagProps = new TagProperties(unknownPeopleTagId);
        tagProps->setProperty("person", "Unknown");
    }
    
    ~FaceIfacePriv()
    {
        delete libkface;
    }

    double                recognitionThreshold;
    int                   detectionAccuracy;
    
    int                   scannedForFacesTagId;
    int                   peopleTagId;
    int                   unknownPeopleTagId;
    
    KFaceIface::Database* libkface;
    TagsCache*            tagsCache;
    TagProperties*        tagProps;
};   

FaceIface::FaceIface()
    : d( new FaceIfacePriv() )
{
    readConfigSettings();
}


FaceIface::~FaceIface()
{
    delete d;
}

bool FaceIface::hasBeenScanned(qlonglong imageid)
{
    ImageTagPair pair(imageid, d->scannedForFacesTagId);
    return pair.hasProperty("scannedForFaces");
}

QList< Face > FaceIface::findAndTagFaces(DImg& image, qlonglong imageid)
{
    readConfigSettings();
    // Find faces
    QImage qimg = image.copyQImage();
    
    kDebug()<<"Image dimensions : "<<qimg.rect();
    
    KFaceIface::Image fimg(qimg);
    QList<KFaceIface::Face> faceList = d->libkface->detectFaces(fimg);
    
    // Mark the image as scanned.
    ImageTagPair pairScanned(imageid, d->scannedForFacesTagId);
    pairScanned.addProperty("scannedForFaces", QString::number(faceList.size()));
    pairScanned.assignTag();

    // Apply region tags to the image for each face that has been detected.
    QListIterator<KFaceIface::Face> it(faceList);
    
    while (it.hasNext())
    {
        KFaceIface::Face face = it.next();
        
        // Assign the "/People/Unknown" tag. The property for this tag is "faceRegion", which has an SVG rect associated with it.
        // When we assign a name for a person in the image, we assign a new tag "/People/<Person Name>" to this image, and move the
        // "faceRegion" property to this tag, and delete it from the unknown tag."
        // See README.FACE for a nicer explanation.
        ImageTagPair pair(imageid, d->tagsCache->tagForPath("/People/Unknown"));
        QRect faceRect = face.toRect();
        kDebug() << faceRect;
        pair.addProperty("faceRegion", rectToString(faceRect));
        pair.addProperty("face", "Unknown");
        pair.assignTag();
        
        kDebug()<<"Applied tag.";
    }
    
    return faceList;
}

int FaceIface::numberOfFaces(qlonglong imageid)
{
    ImageTagPair pairScanned(imageid, d->scannedForFacesTagId);
    return pairScanned.value("scannedForFaces").toInt();
}


QList< Face > FaceIface::findFacesFromTags(DImg& image, qlonglong imageid)
{
    if (!hasBeenScanned(imageid))
    {
        kDebug()<<"Image has not been scanned yet.";
        return QList<Face>();
    }
    
    QImage qimg = image.copyQImage();
    
    QList<Face> faceList;
    
    QList<int> peopleTagIds = allPersonTags();
    
    QListIterator<int> it(peopleTagIds);
    
    while (it.hasNext())
    {
        int currentTag = it.next();
        
        ImageTagPair peopleTags(imageid, currentTag);
        QString tagName = d->tagsCache->tagName(currentTag);
        
        // The only people tags with a face region property are either "Unknown" or "<Name of Person>". So assign them to the Face name
        if(peopleTags.hasProperty("faceRegion"))
        {
            QList<QString> rectStringList = peopleTags.values("faceRegion");
            
            QListIterator<QString> i(rectStringList);
            
            while(i.hasNext())
            {
                QRect rect = stringToRect(i.next());
                kDebug()<<"rect found as "<<rectToString(rect);
                
                // FIXME: Later, need support for putting the cropped image in Face f too. I tried it, but I get weird crashes. Will see.
                Face f;
                f.setRect(rect);
                        
                if(tagName == "Unknown")
                    f.setName("");
                else
                    f.setName(tagName);
                
                faceList += f;
            }
        }
    }
    
    return faceList;
}


void FaceIface::forgetFaceTags(qlonglong imageid)
{
    if (!hasBeenScanned(imageid))
    {
        kDebug()<<"Image has not been scanned yet.";
        return;
    }

    // Remove the "scanned for faces" tag.
    ImageTagPair pair1(imageid, d->scannedForFacesTagId);
    pair1.removeProperties("scannedForFaces");
    pair1.unAssignTag();

    QList<int> peopleTagIds = allPersonTags();

    // Now unassign all these tags from the image. This removes the properties for the Image-tag pair too, which stored the rect
    QListIterator<int> it(peopleTagIds);
    while (it.hasNext())
    {
        int currentTag = it.next();
        ImageTagPair peopleTags(imageid, currentTag );
        peopleTags.removeProperties("faceRegion");
        peopleTags.removeProperties("face");
        peopleTags.unAssignTag();
        kDebug()<<" Removed tag "<< d->tagsCache->tagName(currentTag);
    }
}

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
            kDebug()<< "Image " << info.filePath() << "has already been scanned.";
            
            //dddddd
            DImg img;
            QList<Face> faceList = findFacesFromTags(img, info.id());
            
            QString nameString = d->tagsCache->tagName(tagId);
            if(nameString == "Unknown")
                nameString = "";    // Because we store names for unknown faces this way in the Face objects.
                
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


QList< QRect > FaceIface::getTagRects(qlonglong imageid)
{
    QList< QRect > rectList;
    
    DImg img;
    QList< Face > faceList = this->findFacesFromTags(img, imageid);
    
    foreach (Face f, faceList)
        rectList += f.toRect();
    
    return rectList;
}


QString FaceIface::rectToString(const QRect& rect) const
{
    return ( QString::number(rect.x()) + "," 
           + QString::number(rect.y()) + "," 
           + QString::number(rect.width()) + "," 
           + QString::number(rect.height()) );
}

QRect FaceIface::stringToRect(const QString& string) const
{
    QRect rect;
    
    QStringList list = string.split(",");
    kDebug()<<list;
    
    rect.setX( list[0].toInt() );
    rect.setY( list[1].toInt() );
    rect.setWidth( list[2].toInt() );
    rect.setHeight( list[3].toInt() );
    
    return rect;
}

QList< int > FaceIface::allPersonTags()
{
    AlbumManager *man = AlbumManager::instance();
    QList <int> peopleTagIds = man->subTags(d->peopleTagId, true);
    peopleTagIds += d->peopleTagId;
    
    return peopleTagIds;
}


bool FaceIface::isPerson ( int tagId )
{
    QList<int> peopleTagIds = allPersonTags();
    
    // Now loop through and check if our tagId is in them
    foreach(int id, peopleTagIds)
    {
        if(id == tagId)
            return true;
    }
    
    return false;
}

int FaceIface::setName ( qlonglong imageid, const QRect& rect, const QString& name )
{
    int nameTagId          = d->tagsCache->createTag("/People/" + name);
    ImageTagPair pairNamed   ( imageid, nameTagId );
    
    DatabaseAccess().db()->removeImageTagProperties(imageid, d->unknownPeopleTagId, "faceRegion", rectToString(rect));
    DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "faceRegion", rectToString(rect));
    DatabaseAccess().db()->removeImageTagProperties(imageid, nameTagId, "face", name);
    
    pairNamed.addProperty("faceRegion", rectToString(rect));
    pairNamed.addProperty("face", name);
    pairNamed.assignTag();
   
    d->tagProps = new TagProperties(nameTagId);
    d->tagProps->setProperty("person", name);
    
    if(faceCountForPersonInImage(imageid, d->unknownPeopleTagId) == 0)
    {
            ImageTagPair pair(imageid, d->unknownPeopleTagId);
            pair.removeProperties("faceRegion");
            pair.removeProperties("face");
            pair.unAssignTag();
    }
        
    return nameTagId;
}

void FaceIface::removeRect ( qlonglong imageid, const QRect& rect , const QString& name)
{
    if(name == "")
    {
        DatabaseAccess().db()->removeImageTagProperties(imageid, d->unknownPeopleTagId, "faceRegion", rectToString(rect));
    }
    
    else
    {
        int nameTagId = d->tagsCache->tagForPath("/People/" + name);
        ImageTagPair pair(imageid, nameTagId);
        pair.removeProperties("faceRegion");
        pair.removeProperties("face");
        pair.unAssignTag();
    }
    
    if(faceCountForPersonInImage(imageid, d->unknownPeopleTagId) == 0)
    {
            ImageTagPair pair(imageid, d->unknownPeopleTagId);
            pair.removeProperties("faceRegion");
            pair.removeProperties("face");
            pair.unAssignTag();
    }
}

int FaceIface::faceCountForPersonInImage ( qlonglong imageid, int tagId )
{
    ImageTagPair pair(imageid, tagId);
    return pair.values("faceRegion").size();
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


void FaceIface::trainWithFaces ( QList< Face > faceList )
{
    readConfigSettings();
    
    foreach(Face f, faceList)
    {
        if(f.name() == "" || f.getImage().isNull())
            faceList.remove(f);
    }
    
    d->libkface->updateFaces(faceList);
}

QString FaceIface::recognizedName ( const KFaceIface::Face& face )
{
    QList<Face> f;
    f.append(face);
    
    if( face.getImage().isNull() )
        return "";
    else 
    {
        int distance = d->libkface->recognizeFaces(f).at(0);
        if(distance > d->recognitionThreshold)
            return "";
        else
            return f[0].name();
    }
}

void FaceIface::readConfigSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Face Tags Settings");
    d->detectionAccuracy      = group.readEntry("DetectionAccuracy", 4);
    d->recognitionThreshold   = 1 - group.readEntry("SuggestionThreshold", 0.2);
    
    d->libkface->setDetectionAccuracy(d->detectionAccuracy);
}



} // Namespace Digikam