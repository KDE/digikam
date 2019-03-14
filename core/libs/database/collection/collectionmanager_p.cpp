/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management - private containers.
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

#include "collectionmanager_p.h"

namespace Digikam
{

CollectionManager::Private::Private(CollectionManager* const s)
    : changingDB(false),
      watchEnabled(false),
      s(s)
{
    QObject::connect(s, SIGNAL(triggerUpdateVolumesList()),
                     s, SLOT(slotTriggerUpdateVolumesList()),
                     Qt::BlockingQueuedConnection);
}

QList<SolidVolumeInfo> CollectionManager::Private::listVolumes()
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

void CollectionManager::Private::slotTriggerUpdateVolumesList()
{
    volumesListCache = actuallyListVolumes();
}

QList<SolidVolumeInfo> CollectionManager::Private::actuallyListVolumes()
{
    QList<SolidVolumeInfo> volumes;

    //qCDebug(DIGIKAM_DATABASE_LOG) << "listFromType";
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);
    //qCDebug(DIGIKAM_DATABASE_LOG) << "got listFromType";

    udisToWatch.clear();

    foreach (const Solid::Device& accessDevice, devices)
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

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid() ; currentDevice = currentDevice.parent())
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

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid() ; currentDevice = currentDevice.parent())
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

QString CollectionManager::Private::volumeIdentifier(const SolidVolumeInfo& volume)
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

QString CollectionManager::Private::volumeIdentifier(const QString& path)
{
    QUrl url;
    url.setScheme(QLatin1String("volumeid"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("path"), path);
    url.setQuery(q);

    return url.url();
}

QString CollectionManager::Private::networkShareIdentifier(const QString& path)
{
    QUrl url;
    url.setScheme(QLatin1String("networkshareid"));

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("mountpath"), path);
    url.setQuery(q);

    return url.url();
}

QString CollectionManager::Private::pathFromIdentifier(const AlbumRootLocation* location)
{
    QUrl url(location->identifier);

    if (url.scheme() != QLatin1String("volumeid"))
    {
        return QString();
    }

    return QUrlQuery(url).queryItemValue(QLatin1String("path"));
}

QStringList CollectionManager::Private::networkShareMountPathsFromIdentifier(const AlbumRootLocation* location)
{
    // using a QUrl because QUrl cannot handle duplicate query items
    QUrl url(location->identifier);

    if (url.scheme() != QLatin1String("networkshareid"))
    {
        return QStringList();
    }

    return QUrlQuery(url).allQueryItemValues(QLatin1String("mountpath"));
}

QString CollectionManager::Private::directoryHash(const QString& path)
{
    QDir dir(path);

    if (dir.isReadable())
    {
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        QCryptographicHash md5(QCryptographicHash::Md5);

        foreach (const QString& entry, entries)
        {
            md5.addData(entry.toUtf8());
        }

        return QString::fromUtf8(md5.result().toHex());
    }

    return QString();
}

SolidVolumeInfo CollectionManager::Private::findVolumeForLocation(const AlbumRootLocation* location,
                                                                  const QList<SolidVolumeInfo> volumes)
{
    QUrl url(location->identifier);
    QString queryItem;

    if (url.scheme() != QLatin1String("volumeid"))
    {
        return SolidVolumeInfo();
    }

    if (!(queryItem = QUrlQuery(url).queryItemValue(QLatin1String("uuid"))).isNull())
    {
        foreach (const SolidVolumeInfo& volume, volumes)
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

        foreach (const SolidVolumeInfo& volume, volumes)
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

        foreach (AlbumRootLocation* const otherLocation, locations)
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
            foreach (const SolidVolumeInfo& volume, candidateVolumes)
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
        foreach (const SolidVolumeInfo& volume, volumes)
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

QString CollectionManager::Private::technicalDescription(const AlbumRootLocation* albumLoc)
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

SolidVolumeInfo CollectionManager::Private::findVolumeForUrl(const QUrl& fileUrl,
                                                             const QList<SolidVolumeInfo> volumes)
{
    SolidVolumeInfo volume;
    // v.path is specified to have a trailing slash. path needs one as well.
    QString path    = fileUrl.toLocalFile() + QLatin1Char('/');
    int volumeMatch = 0;

    //FIXME: Network shares! Here we get only the volume of the mount path...
    // This is probably not really clean. But Solid does not help us.
    foreach (const SolidVolumeInfo& v, volumes)
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

bool CollectionManager::Private::checkIfExists(const QString& filePath, QList<CollectionLocation> assumeDeleted)
{
    const QUrl filePathUrl = QUrl::fromLocalFile(filePath);

    CoreDbAccess access;

    foreach (AlbumRootLocation* const location, locations)
    {
        const QUrl locationPathUrl = QUrl::fromLocalFile(location->albumRootPath());

        //qCDebug(DIGIKAM_DATABASE_LOG) << filePathUrl << locationPathUrl;
        // make sure filePathUrl is neither a child nor a parent
        // of an existing collection
        if (!locationPathUrl.isEmpty() &&
            (filePathUrl.isParentOf(locationPathUrl) ||
             locationPathUrl.isParentOf(filePathUrl)))
        {
            bool isDeleted = false;

            foreach (const CollectionLocation& deletedLoc, assumeDeleted)
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

} // namespace Digikam
