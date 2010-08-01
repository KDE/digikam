/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "faceiface.h"

// Qt includes

#include <QImage>

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

#include <libkface/database.h>
#include <libkface/kface.h>

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
    }
    
    ~FaceIfacePriv()
    {
        delete libkface;
    }
    
    int                   scannedForFacesTagId;
    int                   peopleTagId;
    int                   unknownPeopleTagId;
    
    KFaceIface::Database* libkface;
    TagsCache*            tagsCache;
};   

FaceIface::FaceIface()
    : d( new FaceIfacePriv() )
{

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
    QImage qimg = image.copyQImage();
    
    KFaceIface::Image fimg(qimg);
    QList<KFaceIface::Face> faceList = d->libkface->detectFaces(fimg);
    
    // Mark the image as scanned.
    ImageTagPair pairScanned(imageid, d->scannedForFacesTagId);
    pairScanned.addProperty("scannedForFaces", "");
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
        QString s = QString("<rect x=\"")+QString::number(faceRect.x())+
                    QString("\" y=\"")+QString::number(faceRect.y())+
                    QString("\" width=\"")+QString::number(faceRect.width())+
                    QString("\" height=\"")+QString::number(faceRect.height())+ QString("\" />");

        kDebug()<<s;
        pair.addProperty("faceRegion", s);
        pair.assignTag();
        
        kDebug()<<"Applied tag.";
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

    // Populate the peopleTagIds list with all people tags. This includes the "/People" tag and all other subtags of it.
    AlbumManager *man = AlbumManager::instance();
    QList <int> peopleTagIds = man->subTags(d->peopleTagId, true);
    peopleTagIds += d->peopleTagId;

    // Now unassign all these tags from the image. This removes the properties for the Image-tag pair too, which stored the rect
    QListIterator<int> it(peopleTagIds);
    while (it.hasNext())
    {
        int currentTag = it.next();
        ImageTagPair peopleTags(imageid, currentTag );
        peopleTags.removeProperties("faceRegion");
        peopleTags.unAssignTag();
        kDebug()<<" Removed tag "<< d->tagsCache->tagName(currentTag);
    }
}


} // Namespace Digikam