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
        m_id         = info.id;
        m_type       = (CollectionLocation::Type)info.type;
        m_path       = info.absolutePath;
        specificPath = info.specificPath;
        uuid         = info.uuid;

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
        else // Unavailable
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

    QString uuid;
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

CollectionLocation *CollectionManager::addLocation(const KUrl &fileUrl)
{
    QString path = fileUrl.path(KUrl::RemoveTrailingSlash);

    if (locationForPath(path))
        return 0;

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume;
    bool haveVolume = false;

    foreach (volume, volumes)
    {
        if (path.startsWith(volume.path))
        {
            haveVolume = true;
            break;
        }
    }

    if (!haveVolume)
    {
        DError() << "Failed to detect a storage volume for path " << path << " with Solid" << endl;
        return 0;
    }

    DatabaseAccess access;
    // volume.path has a trailing slash. We want to split in front of this.
    QString specificPath = path.mid(volume.path.length() - 1);
    CollectionLocation::Type type;
    if (volume.removableOrPluggable)
        type = CollectionLocation::TypeRemovable;
    else
        type = CollectionLocation::TypeHardWired;

    access.db()->addAlbumRoot(type, volume.path, volume.uuid, specificPath);

    updateLocations();

    return locationForPath(path);
}

void CollectionManager::removeLocation(CollectionLocation *location)
{
    DatabaseAccess().db()->deleteAlbumRoot(((AlbumRootLocation *)location)->id());
    updateLocations();
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
        if (location->status() == CollectionLocation::LocationAvailable)
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
        if (location->status() == CollectionLocation::LocationAvailable)
            list << location->albumRootPath();
    }
    return list;
}

CollectionLocation *CollectionManager::locationForAlbumRootId(int id)
{
    return d->locations.value(id);
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
        if (location->albumRootPath() == path)
            return location;
    }
    return 0;
}

CollectionLocation *CollectionManager::locationForUrl(const KUrl &fileUrl)
{
    return locationForPath(fileUrl.path());
}

CollectionLocation *CollectionManager::locationForPath(const QString &filePath)
{
    DatabaseAccess access;
    foreach (AlbumRootLocation *location, d->locations)
    {
        if (filePath.startsWith(location->albumRootPath()))
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
            location->setAbsolutePath(volumePath + location->specificPath);
            location->setStatusFromFlags();
            // set the volatile values in db
            access.db()->setAlbumRootStatus(location->id(), location->status(), location->albumRootPath());
        }
    }
}

}  // namespace Digikam
