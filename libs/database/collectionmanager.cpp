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

#include <QDir>

// KDE includes

#include <kglobal.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/predicate.h>

// Local includes

#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionmanager.moc"

namespace Digikam
{

#define OLDCODE
#ifdef OLDCODE
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

CollectionManager::CollectionManager()
{
    d = new CollectionManagerPrivate;
}

CollectionManager::~CollectionManager()
{
    delete d;
}


// !! FAKE Implementation !!


QList<CollectionLocation *> CollectionManager::allLocations()
{
    return QList<CollectionLocation *>();
}

QList<CollectionLocation *> CollectionManager::allAvailableLocations()
{
    return QList<CollectionLocation *>();
}

void CollectionManager::updateLocations()
{
}

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    return QStringList() << DatabaseAccess::albumRoot();
}

CollectionLocation *CollectionManager::locationForAlbumRoot(const KUrl &fileUrl)
{
    Q_UNUSED(fileUrl);
    if (!d->location)
        d->location = new SimpleLocation(DatabaseAccess::albumRoot());
    return d->location;
}

CollectionLocation *CollectionManager::locationForAlbumRootPath(const QString &albumRootPath)
{
    Q_UNUSED(albumRootPath);
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
    QString path = fileUrl.path(KUrl::RemoveTrailingSlash);
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








#else









class AlbumRootLocation : public CollectionLocation
{
public:

    AlbumRootLocation()
    {
    }

    AlbumRootLocation(const AlbumRootInfo &info)
    {
        id           = info.id;
        type         = info.type;
        absolutePath = info.absolutePath;
        relativePath = info.relativePath;
        uuid         = info.relativePath;

        setStatus((CollectionLocation::Status)info.status);
    }

    void setStatusFromFlags()
    {
        if (hidden)
        {
            status = CollectionLocation::LocationHidden;
        }
        else
        {
            if (available)
                status = CollectionLocation::LocationAvailable;
            else
                status = CollectionLocation::LocationUnavailable;
        }
    }

    void setStatus(CollectionLocation::Status s)
    {
        status = s;
        // status is exclusive, and Hidden wins
        // but really both states are independent
        // - a hidden location might or might not be available
        if (status == CollectionLocation::LocationAvailable)
        {
            available = true;
            hidden    = false;
        }
        else if (status == CollectionLocation::LocationHidden)
        {
            available = false;
            hidden    = true;
        }
        else // Unavailable
        {
            available = false;
            hidden    = false;
        }
    }

    int id;
    CollectionLocation::Status status;
    int type;
    QString absolutePath;
    QString uuid;
    QString relativePath;
    bool available;
    bool hidden;
};


class SolidVolumeInfo
{
public:
    QString path; // mount path of volume
    QString uuid; // UUID as from Solid
    bool removableOrPluggable; // may be removed
};

// -------------------------------------------------

class CollectionManagerPrivate
{
public:

    CollectionManagerPrivate()
    {
    }

    QMap<int, AlbumRootLocation *> locations;
    QList<SolidVolumeInfo> listVolumes();
};

QList<SolidVolumeInfo> CollectionManagerPrivate::listVolumes()
{
    QList<SolidVolumeInfo> volumes;

    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    foreach(Solid::Device accessDevice, devices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
            continue;

        Solid::StorageAccess *access = accessDevice.as<Solid::StorageAccess>();

        if (!access->isAccessible())
            continue;

        // check for StorageDrive
        Solid::Device driveDevice;
        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }
        if (!driveDevice.isValid())
            continue;

        Solid::StorageDrive *drive = driveDevice.as<Solid::StorageDrive>();

        // check for StorageVolume
        Solid::Device volumeDevice;
        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }
        if (!volumeDevice.isValid())
            continue;

        Solid::StorageVolume *volume = volumeDevice.as<Solid::StorageVolume>();

        SolidVolumeInfo info;
        info.path = access->filePath();
        if (!info.path.endsWith("/"))
            info.path += "/";
        info.uuid = volume->uuid();
        info.removableOrPluggable = drive->isRemovable() || drive->isHotpluggable();
        volumes << info;
    }

    return volumes;
}

// -------------------------------------------------

CollectionManager *CollectionManager::m_instance = 0;

CollectionManager *CollectionManager::instance()
{
    if (!m_instance)
        m_instance = new CollectionManager;
    return m_instance;
}

void CollectionManager::cleanUp()
{
}

CollectionManager::CollectionManager()
{
    d = new CollectionManagerPrivate;
}

CollectionManager::~CollectionManager()
{
    delete d;
}

QList<CollectionLocation *> CollectionManager::allLocations()
{
    DatabaseAccess access;
    QList<CollectionLocation *> list;
    foreach (AlbumRootLocation *location, d->locations)
        list << location;
    return list;
}

QList<CollectionLocation *> CollectionManager::allAvailableLocations()
{
    DatabaseAccess access;
    QList<CollectionLocation *> list;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->status == CollectionLocation::LocationAvailable)
            list << location;
    }
    return list;
}

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    DatabaseAccess access;
    QStringList list;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->status == CollectionLocation::LocationAvailable)
            list << location->absolutePath;
    }
    return list;
}

CollectionLocation *CollectionManager::locationForAlbumRoot(const KUrl &fileUrl)
{
    return locationForAlbumRootPath(fileUrl.path(KUrl::RemoveTrailingSlash));
}

CollectionLocation *CollectionManager::locationForAlbumRootPath(const QString &albumRootPath)
{
    DatabaseAccess access;
    QString path = albumRootPath;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->absolutePath == path)
            return location;
    }
    return 0;
}

KUrl CollectionManager::albumRoot(const KUrl &fileUrl)
{
    return KUrl::fromPath(albumRootPath(fileUrl.path(KUrl::LeaveTrailingSlash)));
}

QString CollectionManager::albumRootPath(const KUrl &fileUrl)
{
    return albumRootPath(fileUrl.path(KUrl::LeaveTrailingSlash));
}

QString CollectionManager::albumRootPath(const QString &filePath)
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (filePath.startsWith(location->absolutePath))
            return location->absolutePath;
    }
    return 0;
}

bool CollectionManager::isAlbumRoot(const KUrl &fileUrl)
{
    return isAlbumRoot(fileUrl.path(KUrl::RemoveTrailingSlash));
}

bool CollectionManager::isAlbumRoot(const QString &filePath)
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (filePath == location->absolutePath)
            return true;
    }
    return false;
}

QString CollectionManager::album(const KUrl &fileUrl)
{
    return album(fileUrl.path(KUrl::LeaveTrailingSlash));
}

QString CollectionManager::album(const QString &filePath)
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        QString firstPart = filePath.left(location->absolutePath.length());
        if (firstPart == location->absolutePath)
        {
            if (filePath == location->absolutePath)
                return "/";
            else
                return filePath.mid(location->absolutePath.length());
        }
    }
    return QString();
}

KUrl CollectionManager::oneAlbumRoot()
{
    return KUrl::fromPath(oneAlbumRootPath());
}

QString CollectionManager::oneAlbumRootPath()
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->status == CollectionLocation::LocationAvailable)
            return location->absolutePath;
    }
    return QString();
}

void CollectionManager::updateLocations()
{
    // get information from Solid
    QList<SolidVolumeInfo> volumes = d->listVolumes();

    {
        DatabaseAccess access;
        // read information from database
        QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

        // synchronize map with database
        QMap<int, AlbumRootLocation *> locs;
        foreach (AlbumRootInfo info, infos)
        {
            if (d->locations.contains(info.id))
            {
                locs[info.id] = d->locations.value(info.id);
                d->locations.remove(info.id);
            }
            else
            {
                locs[info.id] = new AlbumRootLocation(info);
            }
        }
        foreach (AlbumRootLocation *location, d->locations)
            delete location;
        d->locations = locs;

        // update status in db with current access state
        foreach (AlbumRootLocation *location, d->locations)
        {
            QString volumePath;
            bool available = false;
            // if volume is in list, it is accessible
            foreach (SolidVolumeInfo volume, volumes)
            {
                if (volume.uuid == location->uuid)
                {
                    available = true;
                    volumePath = volume.path;
                }
            }
            // set values in location
            // dont touch location->status, do not interfer with "hidden" setting
            location->available = available;
            location->absolutePath = volumePath + location->relativePath;
            location->setStatusFromFlags();
            // set the volatile values in db
            access.db()->setAlbumRootStatus(location->id, location->status, location->absolutePath);
        }
    }
}
#endif

}  // namespace Digikam
