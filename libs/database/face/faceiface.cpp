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

namespace Digikam
{
    
class FaceIfacePriv
{
    FaceIfacePriv()
    {
        libkface             = new KFaceIface::Database(KFaceIface::Database::InitAll, KStandardDirs::locateLocal("data", "libkface"));
        tagsCache            = TagsCache::instance();
        scannedForFacesTagId = tagsCache->createTag("/Scanned/Scanned for Faces");
        peopleTagId          = tagsCache->createTag("/People");
    }
    
    ~FaceIfacePriv()
    {
        delete libkface;
        delete tagsCache;
    }
    int                   scannedForFacesTagId;
    int                   peopleTagId;
    
    KFaceIface::Database* libkface;
    TagsCache*            tagsCache;
};   

FaceIface::FaceIface()
{

}

FaceIface::~FaceIface()
{
    delete d;
}


} // Namespace Digikam