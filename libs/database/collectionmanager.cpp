/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <qdir.h>

// Local includes

#include "databaseaccess.h"
#include "collectionlocation.h"
#include "collectionmanager.h"


namespace Digikam
{

class SimpleLocation : public CollectionLocation
{
public:
    SimpleLocation(const QString &albumRoot)
    {
        m_albumRoot = albumRoot;
        m_status = LocationAvailable;
    }
};

class CollectionManagerPrivate
{
public:
    CollectionManagerPrivate()
    {
        location = 0;
    }
    SimpleLocation *location;
};

CollectionManager *CollectionManager::m_instance = 0;
CollectionManager *CollectionManager::instance()
{
    if (!m_instance)
        m_instance = new CollectionManager;
    return m_instance;
}

void CollectionManager::cleanUp()
{
    delete m_instance;
    m_instance = 0;
}

//QList<CollectionLocation *> allLocations();
//QList<CollectionLocation *> allAvailableLocations();

// !! FAKE Implementation !!

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    return QStringList() << DatabaseAccess::albumRoot();
}

CollectionLocation *CollectionManager::locationForAlbumRoot(KURL fileUrl)
{
    if (!d->location)
        d->location = new SimpleLocation(DatabaseAccess::albumRoot());
    return d->location;
}

CollectionLocation *CollectionManager::locationForAlbumRootPath(QString albumRootPath)
{
    if (!d->location)
        d->location = new SimpleLocation(DatabaseAccess::albumRoot());
    return d->location;
}

KURL CollectionManager::albumRoot(KURL fileUrl)
{
    QString path = fileUrl.path();
    if (path.startsWith(DatabaseAccess::albumRoot()))
    {
        KURL url;
        url.setPath(DatabaseAccess::albumRoot());
        return url;
    }
    return KURL();
}

QString CollectionManager::albumRootPath(KURL fileUrl)
{
    QString path = fileUrl.path();
    if (path.startsWith(DatabaseAccess::albumRoot()))
    {
        return DatabaseAccess::albumRoot();
    }
    return QString();
}

QString CollectionManager::album(KURL fileUrl)
{
    QString path = fileUrl.path();
    path.remove(DatabaseAccess::albumRoot());
    return QDir::cleanDirPath(path);
}

KURL CollectionManager::oneAlbumRoot()
{
    KURL url;
    url.setPath(DatabaseAccess::albumRoot());
    return url;
}

QString CollectionManager::oneAlbumRootPath()
{
    return DatabaseAccess::albumRoot();
}

CollectionManager::CollectionManager()
{
    d = new CollectionManagerPrivate;
}

CollectionManager::~CollectionManager()
{
    delete d;;
}


}

