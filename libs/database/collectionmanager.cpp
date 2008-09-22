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

// Qt includes.

#include <QCoreApplication>
#include <QDir>
#include <QThread>

// KDE includes.

#include <kglobal.h>
#include <kcodecs.h>
#include <klocale.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/predicate.h>

// Local includes.

#include "ddebug.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"

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
        kDebug(50003) << "Creating new Location " << info.specificPath << " uuid " << info.identifier << endl;
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

    void setLabel(const QString &label)
    {
        m_label = label;
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
    QString label; // volume label (think of CDs)
    bool isRemovable; // may be removed
    bool isOpticalDisc;

    bool isNull() const { return path.isNull(); }
};

// -------------------------------------------------

class CollectionManagerPrivate
{
public:

    CollectionManagerPrivate(CollectionManager *s);

    QMap<int, AlbumRootLocation *> locations;

    // hack for Solid's threading problems
    QList<SolidVolumeInfo> actuallyListVolumes();
    void slotTriggerUpdateVolumesList();
    QList<SolidVolumeInfo> volumesListCache;


    /// Access Solid and return a list of storage volumes
    QList<SolidVolumeInfo> listVolumes();

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  corresponding to the location
     */
    SolidVolumeInfo findVolumeForLocation(const AlbumRootLocation *location, const QList<SolidVolumeInfo> volumes);

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  on which the file path specified by the url is located.
     */
    SolidVolumeInfo findVolumeForUrl(const KUrl &url, const QList<SolidVolumeInfo> volumes);

    /// Create the volume identifier for the given volume info
    static QString volumeIdentifier(const SolidVolumeInfo &info);

    /// Create a volume identifier based on the path only
    QString volumeIdentifier(const QString &path);

    /// Return the path, if location has a path-only identifier. Else returns a null string.
    QString pathIdentifier(const AlbumRootLocation *location);

    /// Create an MD5 hash of the top-level entries (file names, not file content) of the given path
    static QString directoryHash(const QString &path);

    CollectionManager *s;
};

}

// This is because of the private slot; we'd want a collectionmanager_p.h
#include "collectionmanager.moc"

namespace Digikam
{

CollectionManagerPrivate::CollectionManagerPrivate(CollectionManager *s)
    : s(s)
{
    QObject::connect(s, SIGNAL(triggerUpdateVolumesList()),
                     s, SLOT(slotTriggerUpdateVolumesList()),
                     Qt::BlockingQueuedConnection);
}

QList<SolidVolumeInfo> CollectionManagerPrivate::listVolumes()
{
    // Move calls to Solid to the main thread.
    // Solid was meant to be thread-safe, but it is not (KDE4.0),
    // calling from a non-UI thread leads to a reversible
    // lock-up of variable length.
    if (QThread::currentThread() == QCoreApplication::instance()->thread())
    {
        return actuallyListVolumes();
    }
    else
    {
        // emit a blocking queued signal to move call to main thread
        emit s->triggerUpdateVolumesList();
        return volumesListCache;
    }
}

void CollectionManagerPrivate::slotTriggerUpdateVolumesList()
{
    volumesListCache = actuallyListVolumes();
}

QList<SolidVolumeInfo> CollectionManagerPrivate::actuallyListVolumes()
{
    QList<SolidVolumeInfo> volumes;

    kDebug(50003) << "listFromType" << endl;
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);
    kDebug(50003) << "got listFromType" << endl;

    foreach(const Solid::Device &accessDevice, devices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
            continue;

        const Solid::StorageAccess *access = accessDevice.as<Solid::StorageAccess>();

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
        if (!info.path.endsWith('/'))
            info.path += '/';
        info.uuid = volume->uuid();
        info.label = volume->label();
        info.isRemovable = drive->isRemovable();
        info.isOpticalDisc = volumeDevice.is<Solid::OpticalDisc>();

        volumes << info;
    }

    return volumes;
}

QString CollectionManagerPrivate::volumeIdentifier(const SolidVolumeInfo &volume)
{
    KUrl url;
    url.setProtocol("volumeid");

    // On changing these, please update the checkLocation() code
    bool identifyByUUID      = !volume.uuid.isEmpty();
    bool identifyByLabel     = !identifyByUUID && !volume.label.isEmpty() && (volume.isOpticalDisc || volume.isRemovable);
    bool addDirectoryHash    = identifyByLabel && volume.isOpticalDisc;
    bool identifyByMountPath = !identifyByUUID && !identifyByLabel;

    if (identifyByUUID)
        url.addQueryItem("uuid", volume.uuid);
    if (identifyByLabel)
        url.addQueryItem("label", volume.label);
    if (addDirectoryHash)
    {
        // for CDs, we store a hash of the root directory. May be useful.
        QString dirHash = directoryHash(volume.path);
        if (!dirHash.isNull())
            url.addQueryItem("directoryhash", dirHash);
    }
    if (identifyByMountPath)
        url.addQueryItem("mountpath", volume.path);

    return url.url();
}

QString CollectionManagerPrivate::volumeIdentifier(const QString &path)
{
    KUrl url;
    url.setProtocol("volumeid");
    url.addQueryItem("path", path);
    return url.url();
}

QString CollectionManagerPrivate::pathIdentifier(const AlbumRootLocation *location)
{
    KUrl url(location->identifier);

    if (url.protocol() != "volumeid")
        return QString();

    return url.queryItem("path");
}

QString CollectionManagerPrivate::directoryHash(const QString &path)
{
    QDir dir(path);
    if (dir.isReadable())
    {
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        KMD5 hash;
        foreach (const QString &entry, entries)
        {
            hash.update(entry.toUtf8());
        }
        return hash.hexDigest();
    }
    return QString();
}

SolidVolumeInfo CollectionManagerPrivate::findVolumeForLocation(const AlbumRootLocation *location, const QList<SolidVolumeInfo> volumes)
{
    KUrl url(location->identifier);
    QString queryItem;

    if (url.protocol() != "volumeid")
        return SolidVolumeInfo();

    if (!(queryItem = url.queryItem("uuid")).isNull())
    {
        foreach (const SolidVolumeInfo &volume, volumes)
        {
            if (volume.uuid == queryItem)
                return volume;
        }
        return SolidVolumeInfo();
    }
    else if (!(queryItem = url.queryItem("label")).isNull())
    {
        // This one is a bit more difficult, as we take into account the possibility
        // that the label is not unique, and we take some care to make it work anyway.

        // find all available volumes with the given label (usually one)
        QList<SolidVolumeInfo> candidateVolumes;
        foreach (const SolidVolumeInfo &volume, volumes)
        {
            if (volume.label == queryItem)
                candidateVolumes << volume;
        }

        if (candidateVolumes.isEmpty())
            return SolidVolumeInfo();

        // find out of there is another location with the same label (usually not)
        bool hasOtherLocation = false;
        foreach (AlbumRootLocation *otherLocation, locations)
        {
            if (otherLocation == location)
                continue;

            KUrl otherUrl(otherLocation->identifier);
            if (otherUrl.protocol() == "volumeid"
                && otherUrl.queryItem("label") == queryItem)
            {
                hasOtherLocation = true;
                break;
            }
        }

        // the usual, easy case
        if (candidateVolumes.size() == 1 && !hasOtherLocation)
            return candidateVolumes.first();
        else
        {
            // not unique: try to use the directoryhash
            QString dirHash = url.queryItem("directoryhash");

            // bail out if not provided
            if (dirHash.isNull())
            {
                kDebug(50003) << "No directory hash specified for the non-unique Label"
                         << queryItem << "Resorting to returning the first match." << endl;
                return candidateVolumes.first();
            }

            // match against directory hash
            foreach (const SolidVolumeInfo &volume, candidateVolumes)
            {
                QString volumeDirHash = directoryHash(volume.path);
                if (volumeDirHash == dirHash)
                    return volume;
            }
        }
        return SolidVolumeInfo();
    }
    else if (!(queryItem = url.queryItem("mountpath")).isNull())
    {
        foreach (const SolidVolumeInfo &volume, volumes)
        {
            if (volume.path == queryItem)
                return volume;
        }
        return SolidVolumeInfo();
    }

    return SolidVolumeInfo();
}

SolidVolumeInfo CollectionManagerPrivate::findVolumeForUrl(const KUrl &url, const QList<SolidVolumeInfo> volumes)
{
    SolidVolumeInfo volume;
    QString path = url.path(KUrl::RemoveTrailingSlash);
    int volumeMatch = 0;

    //FIXME: Network shares! Here we get only the volume of the mount path...
    // This is probably not really clean. But Solid does not help us.
    foreach (const SolidVolumeInfo &v, volumes)
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
        kError(50003) << "Failed to detect a storage volume for path " << path << " with Solid" << endl;
    }

    return volume;
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
    d = new CollectionManagerPrivate(this);

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
    }

    updateLocations();
}

CollectionLocation CollectionManager::addLocation(const KUrl &fileUrl, const QString &label)
{
    kDebug(50003) << "addLocation " << fileUrl << endl;
    QString path = fileUrl.path(KUrl::RemoveTrailingSlash);

    if (!locationForPath(path).isNull())
        return CollectionLocation();

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull())
    {
        DatabaseAccess access;
        // volume.path has a trailing slash. We want to split in front of this.
        QString specificPath = path.mid(volume.path.length() - 1);
        AlbumRoot::Type type;
        if (volume.isRemovable)
            type = AlbumRoot::VolumeRemovable;
        else
            type = AlbumRoot::VolumeHardWired;

        access.db()->addAlbumRoot(type, d->volumeIdentifier(volume), specificPath, label);
    }
    else
    {
        kWarning(50003) << "Unable to identify a path with Solid. Adding the location with path only." << endl;
        DatabaseAccess().db()->addAlbumRoot(AlbumRoot::VolumeHardWired,
                                            d->volumeIdentifier(path), "/", label);
    }

    // Do not emit the locationAdded signal here, it is done in updateLocations()
    updateLocations();

    return locationForPath(path);
}

CollectionManager::LocationCheckResult CollectionManager::checkLocation(const KUrl &fileUrl, QString *message)
{
    QString path = fileUrl.path(KUrl::RemoveTrailingSlash);

    if (!locationForPath(path).isNull())
    {
        if (message)
            *message = i18n("There is already an entry with the same path");
        return LocationNotAllowed;
    }

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull())
    {
        if (!volume.uuid.isEmpty())
        {
            if (volume.isRemovable)
            {
                if (message)
                    *message = i18n("The storage media can be uniquely identified.");
            }
            else
            {
                if (message)
                    *message = QString();
            }
            return LocationAllRight;
        }
        else if (!volume.label.isEmpty() && (volume.isOpticalDisc || volume.isRemovable))
        {
            if (volume.isOpticalDisc)
            {
                bool hasOtherLocation = false;
                foreach (AlbumRootLocation *otherLocation, d->locations)
                {
                    KUrl otherUrl(otherLocation->identifier);
                    if (otherUrl.protocol() == "volumeid"
                        && otherUrl.queryItem("label") == volume.label)
                    {
                        hasOtherLocation = true;
                        break;
                    }
                }

                if (hasOtherLocation)
                {
                    if (message)
                        *message = i18n("This is a CD/DVD, which is identified by the label "
                                        "that you can set in your CD burning application. "
                                        "There is already another entry with the same label. "
                                        "The two will be distinguished by the files in the top directory, "
                                        "so please do not append files to the CD, or it will not be recognized. "
                                        "In the future, please set a unique label on your CDs and DVDs "
                                        "if you intend to use them with digiKam.");
                    return LocationHasProblems;
                }
                else
                {
                    if (message)
                        *message = i18n("This is a CD/DVD. It will be identified by the label (\"%1\")"
                                        "that you have set in your CD burning application. "
                                        "If you create further CDs for use with digikam in the future, "
                                        "please remember to give them a unique label as well.",
                                        volume.label);
                    return LocationAllRight;
                }
            }
            else
            {
                // Which situation? HasProblems or AllRight?
                if (message)
                    *message = i18n("This is a removable storage media that will be identified by its label (\"%1\")",
                                    volume.label);
                return LocationAllRight;
            }
        }
        else
        {
            if (message)
                *message = i18n("This entry will only be identified by the path where it is found on your system (\"%1\"). "
                                "No more specific means of identification (UUID, label) is available.",
                                volume.path);
            return LocationHasProblems;
        }
    }
    else
    {
        if (message)
            *message = i18n("There is a problem identifying the storage media of this path. "
                            "It will be added using the file path as the only identifier");
        return LocationHasProblems;
    }
}

void CollectionManager::removeLocation(const CollectionLocation &location)
{
    {
        DatabaseAccess access;

        AlbumRootLocation *albumLoc = d->locations.value(location.id());
        if (!albumLoc)
            return;

        access.db()->deleteAlbumRoot(albumLoc->id());
    }

    // Do not emit the locationRemoved signal here, it is done in updateLocations()

    updateLocations();
}

void CollectionManager::setLabel(const CollectionLocation &location, const QString &label)
{
    DatabaseAccess access;

    AlbumRootLocation *albumLoc = d->locations.value(location.id());
    if (!albumLoc)
        return;

    // update db
    access.db()->setAlbumRootLabel(albumLoc->id(), label);

    // update local structure
    albumLoc->setLabel(label);
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
    AlbumRootLocation *location = d->locations.value(id);
    if (location)
        return *location;
    else
        return CollectionLocation();
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
        kDebug(50003) << "Testing location " << location->id() << filePath << location->albumRootPath() << endl;
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
            {
                QString album = filePath.mid(absolutePath.length());
                if (album.endsWith('/'))
                    album.chop(1);
                return album;
            }
        }
    }
    return QString();
}

QString CollectionManager::album(const CollectionLocation &location, const KUrl &fileUrl)
{
    return album(location, fileUrl.path(KUrl::LeaveTrailingSlash));
}

QString CollectionManager::album(const CollectionLocation &location, const QString &filePath)
{
    if (location.isNull())
        return QString();
    QString absolutePath = location.albumRootPath();
    if (filePath == absolutePath)
        return "/";
    else
    {
        QString album = filePath.mid(absolutePath.length());
        if (album.endsWith('/'))
            album.chop(1);
        return album;
    }
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
    QList<SolidVolumeInfo> volumes;
    {
        // Absolutely ensure that the db mutex is not held when emitting the blocking queued signal
        DatabaseAccessUnlock unlock;
        DatabaseAccess::assertNoLock(); //TODO: Remove after beta
        volumes = d->listVolumes();
    }

    {
        DatabaseAccess access;
        // read information from database
        QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

        // synchronize map with database
        QMap<int, AlbumRootLocation *> locs = d->locations;
        d->locations.clear();
        foreach (const AlbumRootInfo &info, infos)
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

            SolidVolumeInfo info = d->findVolumeForLocation(location, volumes);

            if (!info.isNull())
            {
                available = true;
                volumePath = info.path;
                // volume.path has a trailing slash (and this is good)
                // but specific path has a leading slash, so remove it
                volumePath.chop(1);
            }
            else
            {
                QString path = d->pathIdentifier(location);
                if (!path.isNull())
                {
                    available = true;
                    volumePath = path;
                }
            }

            //TODO: Network locations (NFS, Samba etc.)

            // set values in location
            // Don't touch location->status, do not interfere with "hidden" setting
            location->available = available;
            location->setAbsolutePath(volumePath + location->specificPath);
            kDebug(50003) << "location for " << volumePath + location->specificPath << " is available " << available << endl;
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
