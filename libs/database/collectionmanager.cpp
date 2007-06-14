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

CollectionManager *CollectionManager::m_componentData = 0;
CollectionManager *CollectionManager::componentData()
{
    if (!m_componentData)
        m_componentData = new CollectionManager;
    return m_componentData;
}

void CollectionManager::cleanUp()
{
    delete m_componentData;
    m_componentData = 0;
}

//QList<CollectionLocation *> allLocations();
//QList<CollectionLocation *> allAvailableLocations();

// !! FAKE Implementation !!

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    return QStringList() << DatabaseAccess::albumRoot();
}

CollectionLocation *CollectionManager::locationForAlbumRoot(const KUrl &fileUrl)
{
    if (!d->location)
        d->location = new SimpleLocation(DatabaseAccess::albumRoot());
    return d->location;
}

CollectionLocation *CollectionManager::locationForAlbumRootPath(const QString &albumRootPath)
{
    if (!d->location)
        d->location = new SimpleLocation(DatabaseAccess::albumRoot());
    return d->location;
}

KUrl CollectionManager::albumRoot(const KUrl &fileUrl)
{
    QString path = fileUrl.path();
    if (path.startsWith(DatabaseAccess::albumRoot()))
    {
        KUrl url;
        url.setPath(DatabaseAccess::albumRoot());
        return url;
    }
    return KUrl();
}

QString CollectionManager::albumRootPath(const KUrl &fileUrl)
{
    QString path = fileUrl.path();
    if (path.startsWith(DatabaseAccess::albumRoot()))
    {
        return DatabaseAccess::albumRoot();
    }
    return QString();
}

bool CollectionManager::isAlbumRoot(const KUrl &fileUrl)
{
    return album(fileUrl) == "/";
}

QString CollectionManager::album(const KUrl &fileUrl)
{
    QString path = fileUrl.path(-1);
    path.remove(DatabaseAccess::albumRoot());
    path = QDir::cleanPath(path);
    if (path.isEmpty())
        path = "/";
    return path;
}

KUrl CollectionManager::oneAlbumRoot()
{
    KUrl url;
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

