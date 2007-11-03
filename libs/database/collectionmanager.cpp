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

#include "ddebug.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionmanager.moc"

namespace Digikam
{

class AlbumRootLocation : public CollectionLocation
{
public:

    AlbumRootLocation()
    {
    }

    AlbumRootLocation(const AlbumRootInfo &info)
    {
        DDebug() << "Creating new Location " << info.specificPath << " uuid " << info.identifier << endl;
        m_id         = info.id;
        m_type       = (Type)info.type;
        specificPath = info.specificPath;
        identifier   = info.identifier;

        m_path       = QString();

        setStatus((CollectionLocation::Status)info.status);
    }

    void setStatusFromFlags()
    {
        if (hidden)
        {
            m_status = CollectionLocation::LocationHidden;
        }
        else
        {
            if (available)
                m_status = CollectionLocation::LocationAvailable;
            else
                m_status = CollectionLocation::LocationUnavailable;
        }
    }

    void setStatus(CollectionLocation::Status s)
    {
        m_status = s;
        // status is exclusive, and Hidden wins
        // but really both states are independent
        // - a hidden location might or might not be available
        if (m_status == CollectionLocation::LocationAvailable)
        {
            available = true;
            hidden    = false;
        }
        else if (m_status == CollectionLocation::LocationHidden)
        {
            available = false;
            hidden    = true;
        }
        else // Unavailable, Null, Deleted
        {
            available = false;
            hidden    = false;
        }
    }

    void setId(int id)
    {
        m_id = id;
    }

    void setAbsolutePath(const QString &path)
    {
        m_path = path;
    }

    void setType(Type type)
    {
        m_type = type;
    }

    QString identifier;
    QString specificPath;
    bool available;
    bool hidden;
};


class SolidVolumeInfo
{
public:
    QString path; // mount path of volume, with trailing slash
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

    qRegisterMetaType<CollectionLocation>("CollectionLocation");

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceAdded(const QString &)),
            this,
            SLOT(deviceChange(const QString &)));

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceRemoved(const QString &)),
            this,
            SLOT(deviceChange(const QString &)));
}

CollectionManager::~CollectionManager()
{
    delete d;
}

void CollectionManager::refresh()
{
    DatabaseAccess access;

    // clear list
    foreach (AlbumRootLocation *location, d->locations)
    {
        CollectionLocation::Status oldStatus = location->status();
        location->setStatus(CollectionLocation::LocationDeleted);
        emit locationStatusChanged(*location, oldStatus);
        delete location;
    }
    d->locations.clear();

    updateLocations();
}

CollectionLocation CollectionManager::addLocation(const KUrl &fileUrl)
{
    DDebug() << "addLocation " << fileUrl << endl;
    QString path = fileUrl.path(KUrl::RemoveTrailingSlash);

    if (!locationForPath(path).isNull())
        return CollectionLocation();

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume;
    int volumeMatch = 0;

    // This is probably not really clean. But Solid does not help us.
    foreach (SolidVolumeInfo v, volumes)
    {
        if (path.startsWith(v.path))
        {
            int length = v.path.length();
            if (length > volumeMatch)
            {
                volumeMatch = v.path.length();
                volume = v;
            }
        }
    }

    if (!volumeMatch)
    {
        DError() << "Failed to detect a storage volume for path " << path << " with Solid" << endl;
        return CollectionLocation();
    }

    DatabaseAccess access;
    // volume.path has a trailing slash. We want to split in front of this.
    QString specificPath = path.mid(volume.path.length() - 1);
    CollectionLocation::Type type;
    if (volume.removableOrPluggable)
        type = CollectionLocation::TypeVolumeRemovable;
    else
        type = CollectionLocation::TypeVolumeHardWired;

    access.db()->addAlbumRoot(type, volume.uuid, specificPath);

    // Do not emit the locationAdded signal here, it is done in updateLocations()
    updateLocations();

    return locationForPath(path);
}

void CollectionManager::removeLocation(const CollectionLocation &location)
{
    DatabaseAccess access;

    AlbumRootLocation *albumLoc = d->locations.value(location.id());
    if (!albumLoc)
        return;

    access.db()->deleteAlbumRoot(albumLoc->id());

    // Do not emit the locationRemoved signal here, it is done in updateLocations()

    updateLocations();
}

QList<CollectionLocation> CollectionManager::allLocations()
{
    DatabaseAccess access;
    QList<CollectionLocation> list;
    foreach (AlbumRootLocation *location, d->locations)
        list << *location;
    return list;
}

QList<CollectionLocation> CollectionManager::allAvailableLocations()
{
    DatabaseAccess access;
    QList<CollectionLocation> list;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
            list << *location;
    }
    return list;
}

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    DatabaseAccess access;
    QStringList list;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
            list << location->albumRootPath();
    }
    return list;
}

CollectionLocation CollectionManager::locationForAlbumRootId(int id)
{
    DatabaseAccess access;
    return *d->locations.value(id);
}

CollectionLocation CollectionManager::locationForAlbumRoot(const KUrl &fileUrl)
{
    return locationForAlbumRootPath(fileUrl.path(KUrl::RemoveTrailingSlash));
}

CollectionLocation CollectionManager::locationForAlbumRootPath(const QString &albumRootPath)
{
    DatabaseAccess access;
    QString path = albumRootPath;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (location->albumRootPath() == path)
            return *location;
    }
    return CollectionLocation();
}

CollectionLocation CollectionManager::locationForUrl(const KUrl &fileUrl)
{
    return locationForPath(fileUrl.path());
}

CollectionLocation CollectionManager::locationForPath(const QString &filePath)
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        DDebug() << "Testing location " << location->id() << filePath << location->albumRootPath() << endl;
        if (filePath.startsWith(location->albumRootPath()))
            return *location;
    }
    return CollectionLocation();
}

QString CollectionManager::albumRootPath(int id)
{
    DatabaseAccess access;
    CollectionLocation *location = d->locations.value(id);
    if (location)
    {
        return location->albumRootPath();
    }
    return QString();
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
        if (filePath.startsWith(location->albumRootPath()))
            return location->albumRootPath();
    }
    return QString();
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
        if (filePath == location->albumRootPath())
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
        QString absolutePath = location->albumRootPath();
        QString firstPart = filePath.left(absolutePath.length());
        if (firstPart == absolutePath)
        {
            if (filePath == absolutePath)
                return "/";
            else
                return filePath.mid(absolutePath.length());
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
        if (location->status() == CollectionLocation::LocationAvailable)
            return location->albumRootPath();
    }
    return QString();
}

void CollectionManager::deviceChange(const QString &udi)
{
    Solid::Device device(udi);
    if (device.is<Solid::StorageAccess>())
        updateLocations();
}

void CollectionManager::updateLocations()
{
    // get information from Solid
    QList<SolidVolumeInfo> volumes = d->listVolumes();
    //DDebug() << "updateLocations: Have " << volumes.count() << endl;

    {
        DatabaseAccess access;
        // read information from database
        QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

        // synchronize map with database
        QMap<int, AlbumRootLocation *> locs = d->locations;
        d->locations.clear();
        foreach (AlbumRootInfo info, infos)
        {
            if (locs.contains(info.id))
            {
                d->locations[info.id] = locs.value(info.id);
                locs.remove(info.id);
            }
            else
            {
                d->locations[info.id] = new AlbumRootLocation(info);
            }
        }

        // delete old locations
        foreach (AlbumRootLocation *location, locs)
        {
            CollectionLocation::Status oldStatus = location->status();
            location->setStatus(CollectionLocation::LocationDeleted);
            emit locationStatusChanged(*location, oldStatus);
            delete location;
        }

        // update status with current access state, store old status in list
        QList<CollectionLocation::Status> oldStatus;
        foreach (AlbumRootLocation *location, d->locations)
        {
            oldStatus << location->status();

            QString volumePath;
            bool available = false;
            // if volume is in list, it is accessible
            foreach (SolidVolumeInfo volume, volumes)
            {
                if (volume.uuid == location->identifier)
                {
                    available = true;
                    volumePath = volume.path;
                    // volume.path has a trailing slash (and this is good)
                    // but specific path has a leading slash, so remove it
                    volumePath.chop(1);
                }
            }

            //TODO: Network locations (NFS, Samba etc.)

            // set values in location
            // dont touch location->status, do not interfer with "hidden" setting
            location->available = available;
            location->setAbsolutePath(volumePath + location->specificPath);
            DDebug() << "location for " << volumePath + location->specificPath << " is available " << available << endl;
            // set the status depending on "hidden" and "available"
            location->setStatusFromFlags();
        }

        // emit status changes (and new locations)
        int i=0;
        foreach (AlbumRootLocation *location, d->locations)
        {
            if (oldStatus[i] != location->status())
            {
                emit locationStatusChanged(*location, oldStatus[i]);
            }
            i++;
        }
    }
}

}  // namespace Digikam
