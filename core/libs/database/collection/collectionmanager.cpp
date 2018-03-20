/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionmanager.h"

// Qt includes

#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QCryptographicHash>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/predicate.h>

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbtransaction.h"
#include "coredb.h"
#include "collectionscanner.h"
#include "collectionlocation.h"

namespace Digikam
{

class AlbumRootLocation : public CollectionLocation
{

public:

    AlbumRootLocation() :
        available(false),
        hidden(false)
    {
    }

    explicit AlbumRootLocation(const AlbumRootInfo& info)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Creating new Location " << info.specificPath << " uuid " << info.identifier;
        m_id         = info.id;
        m_type       = (Type)info.type;
        specificPath = info.specificPath;
        identifier   = info.identifier;
        m_label      = info.label;

        m_path.clear();

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
            {
                m_status = CollectionLocation::LocationAvailable;
            }
            else
            {
                m_status = CollectionLocation::LocationUnavailable;
            }
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

    void setAbsolutePath(const QString& path)
    {
        m_path = path;
    }

    void setType(Type type)
    {
        m_type = type;
    }

    void setLabel(const QString& label)
    {
        m_label = label;
    }

public:

    QString identifier;
    QString specificPath;
    bool    available;
    bool    hidden;
};

// -------------------------------------------------

class SolidVolumeInfo
{

public:

    SolidVolumeInfo()
        : isRemovable(false),
          isOpticalDisc(false),
          isMounted(false)
    {
    }

    bool isNull() const
    {
        return path.isNull();
    }

public:

    QString udi;            // Solid device UDI of the StorageAccess device
    QString path;           // mount path of volume, with trailing slash
    QString uuid;           // UUID as from Solid
    QString label;          // volume label (think of CDs)
    bool    isRemovable;    // may be removed
    bool    isOpticalDisc;  // is an optical disk device as CD/DVD/BR
    bool    isMounted;      // is mounted on File System.
};

// -------------------------------------------------

class CollectionManagerPrivate
{

public:

    explicit CollectionManagerPrivate(CollectionManager* s);

    // hack for Solid's threading problems
    QList<SolidVolumeInfo> actuallyListVolumes();
    void                   slotTriggerUpdateVolumesList();
    QList<SolidVolumeInfo> volumesListCache;


    /// Access Solid and return a list of storage volumes
    QList<SolidVolumeInfo> listVolumes();

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  corresponding to the location
     */
    SolidVolumeInfo findVolumeForLocation(const AlbumRootLocation* location, const QList<SolidVolumeInfo> volumes);

    /**
     *  Find from a given list (usually the result of listVolumes) the volume
     *  on which the file path specified by the url is located.
     */
    SolidVolumeInfo findVolumeForUrl(const QUrl& fileUrl, const QList<SolidVolumeInfo> volumes);

    /// Create the volume identifier for the given volume info
    static QString volumeIdentifier(const SolidVolumeInfo& info);

    /// Create a volume identifier based on the path only
    QString volumeIdentifier(const QString& path);

    /// Create a network share identifier based on the mountpath
    QString networkShareIdentifier(const QString& path);

    /// Return the path, if location has a path-only identifier. Else returns a null string.
    QString pathFromIdentifier(const AlbumRootLocation* location);

    /// Return the path, if location has a path-only identifier. Else returns a null string.
    QStringList networkShareMountPathsFromIdentifier(const AlbumRootLocation* location);

    /// Create an MD5 hash of the top-level entries (file names, not file content) of the given path
    static QString directoryHash(const QString& path);

    /// Check if a location for specified path exists, assuming the given list of locations was deleted
    bool checkIfExists(const QString& path, QList<CollectionLocation> assumeDeleted);

    /// Make a user presentable description, regardless of current location status
    QString technicalDescription(const AlbumRootLocation* location);

public:

    QMap<int, AlbumRootLocation*> locations;
    bool                          changingDB;
    QStringList                   udisToWatch;
    bool                          watchEnabled;
    CollectionManager*            s;
};

// -------------------------------------------------

class ChangingDB
{

public:

    explicit ChangingDB(CollectionManagerPrivate* d) : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

public:

    CollectionManagerPrivate* const d;
};

} // namespace Digikam

// This is because of the private slot; we'd want a collectionmanager_p.h
#include "collectionmanager.h"

namespace Digikam
{

CollectionManagerPrivate::CollectionManagerPrivate(CollectionManager* s)
    : changingDB(false),
      watchEnabled(false),
      s(s)
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
        emit(s->triggerUpdateVolumesList());
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

    //qCDebug(DIGIKAM_DATABASE_LOG) << "listFromType";
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);
    //qCDebug(DIGIKAM_DATABASE_LOG) << "got listFromType";

    udisToWatch.clear();

    foreach(const Solid::Device& accessDevice, devices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        // mark as a device of principal interest
        udisToWatch << accessDevice.udi();

        const Solid::StorageAccess* access = accessDevice.as<Solid::StorageAccess>();

        // watch mount status (remove previous connections)
        QObject::disconnect(access, SIGNAL(accessibilityChanged(bool,QString)),
                            s, SLOT(accessibilityChanged(bool,QString)));
        QObject::connect(access, SIGNAL(accessibilityChanged(bool,QString)),
                         s, SLOT(accessibilityChanged(bool,QString)));

        if (!access->isAccessible())
        {
            continue;
        }

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

        /*
         * We cannot require a drive, some logical volumes may not have "one" drive as parent
         * See bug 273369
        if (!driveDevice.isValid())
        {
            continue;
        }
        */

        Solid::StorageDrive* drive = driveDevice.as<Solid::StorageDrive>();

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
        {
            continue;
        }

        Solid::StorageVolume* const volume = volumeDevice.as<Solid::StorageVolume>();

        SolidVolumeInfo info;
        info.udi       = accessDevice.udi();
        info.path      = QDir::fromNativeSeparators(access->filePath());
        info.isMounted = access->isAccessible();

        if (!info.path.isEmpty() && !info.path.endsWith(QLatin1Char('/')))
        {
            info.path += QLatin1Char('/');
        }

        info.uuid  = volume->uuid();
        info.label = volume->label();

        if (drive)
        {
            info.isRemovable = drive->isHotpluggable() || drive->isRemovable();
        }
        else
        {
            // impossible to know, but probably not hotpluggable (see comment above)
            info.isRemovable = false;
        }

        info.isOpticalDisc = volumeDevice.is<Solid::OpticalDisc>();

        volumes << info;
    }

    // This is the central place where the watch is enabled
    watchEnabled = true;

    return volumes;
}

QString CollectionManagerPrivate::volumeIdentifier(const SolidVolumeInfo& volume)
{
    QUrl url;
    url.setScheme(QLatin1String("volumeid"));

    // On changing these, please update the checkLocation() code
    bool identifyByUUID      = !volume.uuid.isEmpty();
    bool identifyByLabel     = !identifyByUUID && !volume.label.isEmpty() && (volume.isOpticalDisc || volume.isRemovable);
    bool addDirectoryHash    = identifyByLabel && volume.isOpticalDisc;
    bool identifyByMountPath = !identifyByUUID && !identifyByLabel;

    if (identifyByUUID)
    {
        QUrlQuery q(url);
        q.addQueryItem(QLatin1String("uuid"), volume.uuid);
        url.setQuery(q);
    }

    if (identifyByLabel)
    {
        QUrlQuery q(url);
        q.addQueryItem(QLatin1String("label"), volume.label);
        url.setQuery(q);
    }

    if (addDirectoryHash)
    {
        // for CDs, we store a hash of the root directory. May be useful.
        QString dirHash = directoryHash(volume.path);

        if (!dirHash.isNull())
        {
            QUrlQuery q(url);
            q.addQueryItem(QLatin1String("directoryhash"), dirHash);
            url.setQuery(q);
        }
    }

    if (identifyByMountPath)
    {
        QUrlQuery q(url);
        q.addQueryItem(QLatin1String("mountpath"), volume.path);
        url.setQuery(q);
    }

    return url.url();
}

QString CollectionManagerPrivate::volumeIdentifier(const QString& path)
{
    QUrl url;
    url.setScheme(QLatin1String("volumeid"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("path"), path);
    url.setQuery(q);

    return url.url();
}

QString CollectionManagerPrivate::networkShareIdentifier(const QString& path)
{
    QUrl url;
    url.setScheme(QLatin1String("networkshareid"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("mountpath"), path);
    url.setQuery(q);

    return url.url();
}

QString CollectionManagerPrivate::pathFromIdentifier(const AlbumRootLocation* location)
{
    QUrl url(location->identifier);

    if (url.scheme() != QLatin1String("volumeid"))
    {
        return QString();
    }

    return QUrlQuery(url).queryItemValue(QLatin1String("path"));
}

QStringList CollectionManagerPrivate::networkShareMountPathsFromIdentifier(const AlbumRootLocation* location)
{
    // using a QUrl because QUrl cannot handle duplicate query items
    QUrl url(location->identifier);

    if (url.scheme() != QLatin1String("networkshareid"))
    {
        return QStringList();
    }

    return QUrlQuery(url).allQueryItemValues(QLatin1String("mountpath"));
}

QString CollectionManagerPrivate::directoryHash(const QString& path)
{
    QDir dir(path);

    if (dir.isReadable())
    {
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        QCryptographicHash md5(QCryptographicHash::Md5);

        foreach(const QString& entry, entries)
        {
            md5.addData(entry.toUtf8());
        }

        return QString::fromUtf8(md5.result().toHex());
    }

    return QString();
}

SolidVolumeInfo CollectionManagerPrivate::findVolumeForLocation(const AlbumRootLocation* location, const QList<SolidVolumeInfo> volumes)
{
    QUrl url(location->identifier);
    QString queryItem;

    if (url.scheme() != QLatin1String("volumeid"))
    {
        return SolidVolumeInfo();
    }

    if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("uuid"))).isNull())
    {
        foreach(const SolidVolumeInfo& volume, volumes)
        {
            if (volume.uuid.compare(queryItem, Qt::CaseInsensitive) == 0)
            {
                return volume;
            }
        }
        return SolidVolumeInfo();
    }
    else if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("label"))).isNull())
    {
        // This one is a bit more difficult, as we take into account the possibility
        // that the label is not unique, and we take some care to make it work anyway.

        // find all available volumes with the given label (usually one)
        QList<SolidVolumeInfo> candidateVolumes;

        foreach(const SolidVolumeInfo& volume, volumes)
        {
            if (volume.label == queryItem)
            {
                candidateVolumes << volume;
            }
        }

        if (candidateVolumes.isEmpty())
        {
            return SolidVolumeInfo();
        }

        // find out of there is another location with the same label (usually not)
        bool hasOtherLocation = false;

        foreach(AlbumRootLocation* const otherLocation, locations)
        {
            if (otherLocation == location)
            {
                continue;
            }

            QUrl otherUrl(otherLocation->identifier);

            if (otherUrl.scheme() == QLatin1String("volumeid")
                && QUrlQuery(otherUrl).queryItemValue(QLatin1String("label")) == queryItem)
            {
                hasOtherLocation = true;
                break;
            }
        }

        // the usual, easy case
        if (candidateVolumes.size() == 1 && !hasOtherLocation)
        {
            return candidateVolumes.first();
        }
        else
        {
            // not unique: try to use the directoryhash
            QString dirHash = QUrlQuery(url).queryItemValue(QLatin1String("directoryhash"));

            // bail out if not provided
            if (dirHash.isNull())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "No directory hash specified for the non-unique Label"
                                             << queryItem << "Resorting to returning the first match.";
                return candidateVolumes.first();
            }

            // match against directory hash
            foreach(const SolidVolumeInfo& volume, candidateVolumes)
            {
                QString volumeDirHash = directoryHash(volume.path);

                if (volumeDirHash == dirHash)
                {
                    return volume;
                }
            }
        }

        return SolidVolumeInfo();
    }
    else if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("mountpath"))).isNull())
    {
        foreach(const SolidVolumeInfo& volume, volumes)
        {
            if (volume.isMounted && volume.path == queryItem)
            {
                return volume;
            }
        }

        return SolidVolumeInfo();
    }

    return SolidVolumeInfo();
}

QString CollectionManagerPrivate::technicalDescription(const AlbumRootLocation* albumLoc)
{
    QUrl url(albumLoc->identifier);
    QString queryItem;

    if (url.scheme() == QLatin1String("volumeid"))
    {
        if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("uuid"))).isNull())
        {
            return i18nc("\"relative path\" on harddisk partition with \"UUID\"",
                         "Folder \"%1\" on the volume with the id \"%2\"",
                         QDir::toNativeSeparators(albumLoc->specificPath), queryItem);
        }
        else if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("label"))).isNull())
        {
            return i18nc("\"relative path\" on harddisk partition with \"label\"",
                         "Folder \"%1\" on the volume labeled \"%2\"",
                         QDir::toNativeSeparators(albumLoc->specificPath), queryItem);
        }
        else if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("mountpath"))).isNull())
        {
            return QString::fromUtf8("\"%1\"").arg(queryItem);
        }
    }
    else if (url.scheme() == QLatin1String("networkshareid"))
    {
        if (!(queryItem =  QUrlQuery(url).queryItemValue(QLatin1String("mountpath"))).isNull())
        {
            return i18nc("@info", "Shared directory mounted at <b>%1</b>", QDir::toNativeSeparators(queryItem));
        }
    }

    return QString();
}

SolidVolumeInfo CollectionManagerPrivate::findVolumeForUrl(const QUrl& fileUrl, const QList<SolidVolumeInfo> volumes)
{
    SolidVolumeInfo volume;
    // v.path is specified to have a trailing slash. path needs one as well.
    QString path    = fileUrl.toLocalFile() + QLatin1String("/");
    int volumeMatch = 0;

    //FIXME: Network shares! Here we get only the volume of the mount path...
    // This is probably not really clean. But Solid does not help us.
    foreach(const SolidVolumeInfo& v, volumes)
    {
        if (v.isMounted && !v.path.isEmpty() && path.startsWith(v.path))
        {
            int length = v.path.length();

            if (length > volumeMatch)
            {
                volumeMatch = v.path.length();
                volume      = v;
            }
        }
    }

    if (!volumeMatch)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Failed to detect a storage volume for path " << path << " with Solid";
    }

    return volume;
}

bool CollectionManagerPrivate::checkIfExists(const QString& filePath, QList<CollectionLocation> assumeDeleted)
{
    const QUrl filePathUrl = QUrl::fromLocalFile(filePath);

    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, locations)
    {
        const QUrl locationPathUrl = QUrl::fromLocalFile(location->albumRootPath());

        //qCDebug(DIGIKAM_DATABASE_LOG) << filePathUrl << locationPathUrl;
        // make sure filePathUrl is neither a child nor a parent
        // of an existing collection
        if (!locationPathUrl.isEmpty() &&
            (filePathUrl.isParentOf(locationPathUrl) ||
             locationPathUrl.isParentOf(filePathUrl))
           )
        {
            bool isDeleted = false;

            foreach(const CollectionLocation& deletedLoc, assumeDeleted)
            {
                if (deletedLoc.id() == location->id())
                {
                    isDeleted = true;
                    break;
                }
            }

            if (!isDeleted)
            {
                return true;
            }
        }
    }

    return false;
}

// -------------------------------------------------

CollectionManager* CollectionManager::m_instance = 0;

CollectionManager* CollectionManager::instance()
{
    if (!m_instance)
    {
        m_instance = new CollectionManager;
    }

    return m_instance;
}

void CollectionManager::cleanUp()
{
    delete m_instance;
    m_instance = 0;
}

CollectionManager::CollectionManager()
    : d(new CollectionManagerPrivate(this))
{
    qRegisterMetaType<CollectionLocation>("CollectionLocation");

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceAdded(QString)),
            this,
            SLOT(deviceAdded(QString)));

    connect(Solid::DeviceNotifier::instance(),
            SIGNAL(deviceRemoved(QString)),
            this,
            SLOT(deviceRemoved(QString)));

    // CoreDbWatch slot is connected at construction of CoreDbWatch, which may be later.
}

CollectionManager::~CollectionManager()
{
    qDeleteAll(d->locations.values());
    delete d;
}

void CollectionManager::refresh()
{
    {
        // if called from the CoreDbAccess constructor itself, it will
        // hold a flag to prevent endless recursion
        CoreDbAccess access;
        clear_locked();
    }

    updateLocations();
}

void CollectionManager::setWatchDisabled()
{
    d->watchEnabled = false;
}

CollectionLocation CollectionManager::addLocation(const QUrl& fileUrl, const QString& label)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "addLocation " << fileUrl;
    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    if (!locationForPath(path).isNull())
    {
        return CollectionLocation();
    }

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume         = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull())
    {
        CoreDbAccess access;
        // volume.path has a trailing slash. We want to split in front of this.
        QString specificPath = path.mid(volume.path.length() - 1);
        AlbumRoot::Type type;

        if (volume.isRemovable)
        {
            type = AlbumRoot::VolumeRemovable;
        }
        else
        {
            type = AlbumRoot::VolumeHardWired;
        }

        ChangingDB changing(d);
        access.db()->addAlbumRoot(type, d->volumeIdentifier(volume), specificPath, label);
    }
    else
    {
        // Empty volumes indicates that Solid is not working correctly.
        if (volumes.isEmpty())
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Solid did not return any storage volumes on your system.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "This indicates a missing implementation or a problem with your installation";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Linux, check that Solid and HAL are working correctly."
                                            "Problems with RAID partitions have been reported, if you have RAID this error may be normal.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Windows, Solid may not be fully implemented, if you are running Windows this error may be normal.";
        }

        // fall back
        qCWarning(DIGIKAM_DATABASE_LOG) << "Unable to identify a path with Solid. Adding the location with path only.";
        ChangingDB changing(d);
        CoreDbAccess().db()->addAlbumRoot(AlbumRoot::VolumeHardWired,
                                            d->volumeIdentifier(path), QLatin1String("/"), label);
    }

    // Do not emit the locationAdded signal here, it is done in updateLocations()
    updateLocations();

    return locationForPath(path);
}

CollectionLocation CollectionManager::addNetworkLocation(const QUrl& fileUrl, const QString& label)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "addLocation " << fileUrl;
    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    if (!locationForPath(path).isNull())
    {
        return CollectionLocation();
    }

    ChangingDB changing(d);
    CoreDbAccess().db()->addAlbumRoot(AlbumRoot::Network, d->networkShareIdentifier(path), QLatin1String("/"), label);

    // Do not emit the locationAdded signal here, it is done in updateLocations()
    updateLocations();

    return locationForPath(path);
}

CollectionManager::LocationCheckResult CollectionManager::checkLocation(const QUrl& fileUrl,
        QList<CollectionLocation> assumeDeleted, QString* message, QString* iconName)
{
    if (!fileUrl.isLocalFile())
    {
        if (message)
        {
            *message = i18n("Sorry, digiKam does not support remote URLs as collections.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();
    QDir dir(path);

    if (!dir.isReadable())
    {
        if (message)
        {
            *message = i18n("The selected folder does not exist or is not readable");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (d->checkIfExists(path, assumeDeleted))
    {
        if (message)
        {
            *message = i18n("There is already a collection containing the folder \"%1\"", QDir::toNativeSeparators(path));
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

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
                {
                    *message = i18n("The storage media can be uniquely identified.");
                }

                if (iconName)
                {
                    *iconName = QLatin1String("drive-removable-media");
                }
            }
            else
            {
                if (message)
                {
                    *message = i18n("The collection is located on your harddisk");
                }

                if (iconName)
                {
                    *iconName = QLatin1String("drive-harddisk");
                }
            }

            return LocationAllRight;
        }
        else if (!volume.label.isEmpty() && (volume.isOpticalDisc || volume.isRemovable))
        {
            if (volume.isOpticalDisc)
            {
                bool hasOtherLocation = false;

                foreach(AlbumRootLocation* const otherLocation, d->locations)
                {
                    QUrl otherUrl(otherLocation->identifier);

                    if (otherUrl.scheme() == QLatin1String("volumeid") &&
                        QUrlQuery(otherUrl).queryItemValue(QLatin1String("label")) == volume.label)
                    {
                        hasOtherLocation = true;
                        break;
                    }
                }

                if (iconName)
                {
                    *iconName = QLatin1String("media-optical");
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
                    *message = i18n("This is a removable storage medium that will be identified by its label (\"%1\")",
                                    volume.label);

                if (iconName)
                {
                    *iconName = QLatin1String("drive-removable-media");
                }

                return LocationAllRight;
            }
        }
        else
        {
            if (message)
                *message = i18n("This entry will only be identified by the path where it is found on your system (\"%1\"). "
                                "No more specific means of identification (UUID, label) is available.",
                                QDir::toNativeSeparators(volume.path));

            if (iconName)
            {
                *iconName = QLatin1String("drive-removale-media");
            }

            return LocationHasProblems;
        }
    }
    else
    {
        if (message)
            *message = i18n("It is not possible on your system to identify the storage medium of this path. "
                            "It will be added using the file path as the only identifier. "
                            "This will work well for your local hard disk.");

        if (iconName)
        {
            *iconName = QLatin1String("folder-important");
        }

        return LocationHasProblems;
    }
}

CollectionManager::LocationCheckResult CollectionManager::checkNetworkLocation(const QUrl& fileUrl,
                                                                               QList<CollectionLocation> assumeDeleted,
                                                                               QString* message,
                                                                               QString* iconName)
{
    if (!fileUrl.isLocalFile())
    {
        if (message)
        {
            if (fileUrl.scheme() == QLatin1String("smb"))
                *message = i18n("You need to locally mount your Samba share. "
                                "Sorry, digiKam does currently not support smb:// URLs. ");
            else
                *message = i18n("Your network storage must be set up to be accessible "
                                "as files and folders through the operating system. "
                                "DigiKam does not support remote URLs.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    QDir dir(path);

    if (!dir.isReadable())
    {
        if (message)
        {
            *message = i18n("The selected folder does not exist or is not readable");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (d->checkIfExists(path, assumeDeleted))
    {
        if (message)
        {
            *message = i18n("There is already a collection for a network share with the same path.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (message)
        *message = i18n("The network share will be identified by the path you selected. "
                        "If the path is empty, the share will be considered unavailable.");

    if (iconName)
    {
        *iconName = QLatin1String("network-wired-activated");
    }

    return LocationAllRight;
}

void CollectionManager::removeLocation(const CollectionLocation& location)
{
    {
        CoreDbAccess access;

        AlbumRootLocation* const albumLoc = d->locations.value(location.id());

        if (!albumLoc)
        {
            return;
        }

        // Ensure that all albums are set to orphan and no images will be permanently deleted,
        // as would do only calling deleteAlbumRoot by a Trigger
        QList<int> albumIds = access.db()->getAlbumsOnAlbumRoot(albumLoc->id());
        ChangingDB changing(d);
        CollectionScanner scanner;
        CoreDbTransaction transaction(&access);
        scanner.safelyRemoveAlbums(albumIds);
        access.db()->deleteAlbumRoot(albumLoc->id());
    }

    // Do not emit the locationRemoved signal here, it is done in updateLocations()

    updateLocations();
}

QList<CollectionLocation> CollectionManager::checkHardWiredLocations()
{
    QList<CollectionLocation> disappearedLocations;
    QList<SolidVolumeInfo> volumes = d->listVolumes();

    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        // Hardwired and unavailable?
        if (location->type()   == CollectionLocation::TypeVolumeHardWired &&
            location->status() == CollectionLocation::LocationUnavailable)
        {
            disappearedLocations << *location;
        }
    }

    return disappearedLocations;
}

void CollectionManager::migrationCandidates(const CollectionLocation& location,
                                            QString* const description,
                                            QStringList* const candidateIdentifiers,
                                            QStringList* const candidateDescriptions)
{
    description->clear();
    candidateIdentifiers->clear();
    candidateDescriptions->clear();

    QList<SolidVolumeInfo> volumes = d->listVolumes();

    CoreDbAccess access;

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    *description = d->technicalDescription(albumLoc);

    // Find possible new volumes where the specific path is found.
    foreach(const SolidVolumeInfo& info, volumes)
    {
        if (info.isMounted && !info.path.isEmpty())
        {
            QDir dir(info.path + albumLoc->specificPath);

            if (dir.exists())
            {
                *candidateIdentifiers << d->volumeIdentifier(info);
                *candidateDescriptions << dir.absolutePath();
            }
        }
    }
}

void CollectionManager::migrateToVolume(const CollectionLocation& location, const QString& identifier)
{
    CoreDbAccess access;

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db
    ChangingDB db(d);
    access.db()->migrateAlbumRoot(albumLoc->id(), identifier);

    albumLoc->identifier = identifier;

    updateLocations();
}

void CollectionManager::setLabel(const CollectionLocation& location, const QString& label)
{
    CoreDbAccess access;

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db
    ChangingDB db(d);
    access.db()->setAlbumRootLabel(albumLoc->id(), label);

    // update local structure
    albumLoc->setLabel(label);

    emit locationPropertiesChanged(*albumLoc);
}

void CollectionManager::changeType(const CollectionLocation& location, int type)
{
    CoreDbAccess access;

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db
    ChangingDB db(d);
    access.db()->changeAlbumRootType(albumLoc->id(), (AlbumRoot::Type)type);

    // update local structure
    albumLoc->setType((CollectionLocation::Type)type);

    emit locationPropertiesChanged(*albumLoc);
}

QList<CollectionLocation> CollectionManager::allLocations()
{
    CoreDbAccess access;
    QList<CollectionLocation> list;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        list << *location;
    }

    return list;
}

QList<CollectionLocation> CollectionManager::allAvailableLocations()
{
    CoreDbAccess access;
    QList<CollectionLocation> list;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            list << *location;
        }
    }

    return list;
}

QStringList CollectionManager::allAvailableAlbumRootPaths()
{
    CoreDbAccess access;
    QStringList list;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            list << location->albumRootPath();
        }
    }

    return list;
}

CollectionLocation CollectionManager::locationForAlbumRootId(int id)
{
    CoreDbAccess access;
    AlbumRootLocation* const location = d->locations.value(id);

    if (location)
    {
        return *location;
    }
    else
    {
        return CollectionLocation();
    }
}

CollectionLocation CollectionManager::locationForAlbumRoot(const QUrl& fileUrl)
{
    return locationForAlbumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

CollectionLocation CollectionManager::locationForAlbumRootPath(const QString& albumRootPath)
{
    CoreDbAccess access;
    QString path = albumRootPath;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->albumRootPath() == path)
        {
            return *location;
        }
    }

    return CollectionLocation();
}

CollectionLocation CollectionManager::locationForUrl(const QUrl& fileUrl)
{
    return locationForPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

CollectionLocation CollectionManager::locationForPath(const QString& givenPath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        QString rootPath = location->albumRootPath();
        QString filePath = QDir::fromNativeSeparators(givenPath);

        if (!rootPath.isEmpty() && filePath.startsWith(rootPath))
        {
            // see also bug #221155 for extra checks
            if (filePath == rootPath || filePath.startsWith(rootPath + QLatin1Char('/')))
            {
                return *location;
            }
        }
    }

    return CollectionLocation();
}

QString CollectionManager::albumRootPath(int id)
{
    CoreDbAccess access;
    CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->albumRootPath();
    }

    return QString();
}

QString CollectionManager::albumRootLabel(int id)
{
    CoreDbAccess access;
    CollectionLocation* const location = d->locations.value(id);

    if (location && location->status() == CollectionLocation::LocationAvailable)
    {
        return location->label();
    }

    return QString();
}

QUrl CollectionManager::albumRoot(const QUrl& fileUrl)
{
    return QUrl::fromLocalFile(albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile()));
}

QString CollectionManager::albumRootPath(const QUrl& fileUrl)
{
    return albumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::albumRootPath(const QString& givenPath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        QString rootPath = location->albumRootPath();
        QString filePath = QDir::fromNativeSeparators(givenPath);

        if (!rootPath.isEmpty() && filePath.startsWith(rootPath))
        {
            // see also bug #221155 for extra checks
            if (filePath == rootPath || filePath.startsWith(rootPath + QLatin1Char('/')))
            {
                return location->albumRootPath();
            }
        }
    }

    return QString();
}

bool CollectionManager::isAlbumRoot(const QUrl& fileUrl)
{
    return isAlbumRoot(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

bool CollectionManager::isAlbumRoot(const QString& filePath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (filePath == location->albumRootPath())
        {
            return true;
        }
    }

    return false;
}

QString CollectionManager::album(const QUrl& fileUrl)
{
    return album(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const QString& filePath)
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        QString absolutePath = location->albumRootPath();

        if (absolutePath.isEmpty())
        {
            continue;
        }

        QString firstPart = filePath.left(absolutePath.length());

        if (firstPart == absolutePath)
        {
            if (filePath == absolutePath ||
                (filePath.length() == absolutePath.length() + 1 && filePath.right(1) == QLatin1String("/")))
            {
                return QLatin1String("/");
            }
            else
            {
                QString album = filePath.mid(absolutePath.length());

                if (album.endsWith(QLatin1Char('/')))
                {
                    album.chop(1);
                }

                return album;
            }
        }
    }

    return QString();
}

QString CollectionManager::album(const CollectionLocation& location, const QUrl& fileUrl)
{
    return album(location, fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

QString CollectionManager::album(const CollectionLocation& location, const QString& filePath)
{
    if (location.isNull())
    {
        return QString();
    }

    QString absolutePath = location.albumRootPath();

    if (filePath == absolutePath)
    {
        return QLatin1String("/");
    }
    else
    {
        QString album = filePath.mid(absolutePath.length());

        if (album.endsWith(QLatin1Char('/')))
        {
            album.chop(1);
        }

        return album;
    }
}

QUrl CollectionManager::oneAlbumRoot()
{
    return QUrl::fromLocalFile(oneAlbumRootPath());
}

QString CollectionManager::oneAlbumRootPath()
{
    CoreDbAccess access;

    foreach(AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            return location->albumRootPath();
        }
    }

    return QString();
}

void CollectionManager::deviceAdded(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    Solid::Device device(udi);

    if (device.is<Solid::StorageAccess>())
    {
        updateLocations();
    }
}

void CollectionManager::deviceRemoved(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    // we can't access the Solid::Device to check because it is removed
    CoreDbAccess access;

    if (!d->udisToWatch.contains(udi))
    {
        return;
    }

    updateLocations();
}

void CollectionManager::accessibilityChanged(bool accessible, const QString& udi)
{
    Q_UNUSED(accessible);
    Q_UNUSED(udi);
    updateLocations();
}

void CollectionManager::updateLocations()
{
    // get information from Solid
    QList<SolidVolumeInfo> volumes;
    {
        // Absolutely ensure that the db mutex is not held when emitting the blocking queued signal! Deadlock!
        CoreDbAccessUnlock unlock;
        volumes = d->listVolumes();
    }

    {
        CoreDbAccess access;
        // read information from database
        QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

        // synchronize map with database
        QMap<int, AlbumRootLocation*> locs = d->locations;
        d->locations.clear();

        foreach(const AlbumRootInfo& info, infos)
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
        foreach(AlbumRootLocation* const location, locs)
        {
            CollectionLocation::Status oldStatus = location->status();
            location->setStatus(CollectionLocation::LocationDeleted);
            emit locationStatusChanged(*location, oldStatus);
            delete location;
        }

        // update status with current access state, store old status in list
        QList<CollectionLocation::Status> oldStatus;

        foreach(AlbumRootLocation* const location, d->locations)
        {
            oldStatus << location->status();
            bool available = false;
            QString absolutePath;

            if (location->type() == CollectionLocation::TypeNetwork)
            {
                foreach(const QString& path, d->networkShareMountPathsFromIdentifier(location))
                {
                    QDir dir(path);
                    available    = dir.isReadable() &&
                                   dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).count() > 0;
                    absolutePath = path;

                    if (available)
                    {
                        break;
                    }
                }
            }
            else
            {
                SolidVolumeInfo info = d->findVolumeForLocation(location, volumes);

                if (!info.isNull())
                {
                    available          = info.isMounted;
                    QString volumePath = info.path;
                    // volume.path has a trailing slash (and this is good)
                    // but specific path has a leading slash, so remove it
                    volumePath.chop(1);
                    // volumePath is the mount point of the volume;
                    // specific path is the path on the file system of the volume.
                    absolutePath = volumePath + location->specificPath;
                }
                else
                {
                    QString path = d->pathFromIdentifier(location);

                    if (!path.isNull())
                    {
                        available    = true;
                        // Here we have the absolute path as definition of the volume.
                        // specificPath is "/" as per convention, but ignored,
                        // absolute path shall not have a trailing slash.
                        absolutePath = path;
                    }
                }
            }

            // set values in location
            // Don't touch location->status, do not interfere with "hidden" setting
            location->available = available;
            location->setAbsolutePath(absolutePath);
            qCDebug(DIGIKAM_DATABASE_LOG) << "location for " << absolutePath << " is available " << available;
            // set the status depending on "hidden" and "available"
            location->setStatusFromFlags();
        }

        // emit status changes (and new locations)
        int i = 0;

        foreach(AlbumRootLocation* const location, d->locations)
        {
            if (oldStatus.at(i) != location->status())
            {
                emit locationStatusChanged(*location, oldStatus.at(i));
            }

            ++i;
        }
    }
}

void CollectionManager::clear_locked()
{
    // Internal method: Called with mutex locked
    // Cave: Difficult recursions with CoreDbAccess constructor and setParameters
    foreach(AlbumRootLocation* const location, d->locations)
    {
        CollectionLocation::Status oldStatus = location->status();
        location->setStatus(CollectionLocation::LocationDeleted);
        emit locationStatusChanged(*location, oldStatus);
        delete location;
    }

    d->locations.clear();
}

void CollectionManager::slotAlbumRootChange(const AlbumRootChangeset& changeset)
{
    if (d->changingDB)
    {
        return;
    }

    switch (changeset.operation())
    {
        case AlbumRootChangeset::Added:
        case AlbumRootChangeset::Deleted:
            updateLocations();
            break;

        case AlbumRootChangeset::PropertiesChanged:
            // label has changed
        {
            CollectionLocation toBeEmitted;
            {
                CoreDbAccess access;
                AlbumRootLocation* const location = d->locations.value(changeset.albumRootId());

                if (location)
                {
                    QList<AlbumRootInfo> infos = access.db()->getAlbumRoots();

                    foreach(const AlbumRootInfo& info, infos)
                    {
                        if (info.id == location->id())
                        {
                            location->setLabel(info.label);
                            toBeEmitted = *location;
                            break;
                        }
                    }
                }
            }

            if (!toBeEmitted.isNull())
            {
                emit locationPropertiesChanged(toBeEmitted);
            }
        }
        break;

        case AlbumRootChangeset::Unknown:
            break;
    }
}

}  // namespace Digikam

#include "moc_collectionmanager.cpp"
