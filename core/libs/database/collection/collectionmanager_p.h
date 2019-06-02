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

#ifndef DIGIKAM_COLLECTION_MANAGER_P_H
#define DIGIKAM_COLLECTION_MANAGER_P_H

#include "collectionmanager.h"

// Qt includes

#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QReadWriteLock>
#include <QUrlQuery>
#include <QThread>

// KDE includes

#include <klocalizedstring.h>

// Solid includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnonportable-include-path"
#endif

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/predicate.h>

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

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

class Q_DECL_HIDDEN AlbumRootLocation : public CollectionLocation
{

public:

    AlbumRootLocation()
      : available(false),
        hidden(false)
    {
    }

    explicit AlbumRootLocation(const AlbumRootInfo& info)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Creating new Location " << info.specificPath << " uuid " << info.identifier;
        m_id         = info.id;
        m_type       = (Type)info.type;
        QString path = info.specificPath;

        if (path != QLatin1String("/") &&
            path.endsWith(QLatin1Char('/')))
        {
            path.chop(1);
        }

        specificPath = path;
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

class Q_DECL_HIDDEN SolidVolumeInfo
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

class Q_DECL_HIDDEN CollectionManager::Private
{

public:

    explicit Private(CollectionManager* const s);

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

    QReadWriteLock                lock;
    QMap<int, AlbumRootLocation*> locations;
    bool                          changingDB;
    QStringList                   udisToWatch;
    bool                          watchEnabled;
    CollectionManager*            s;
};

// -------------------------------------------------

class Q_DECL_HIDDEN ChangingDB
{

public:

    explicit ChangingDB(CollectionManager::Private* const d)
        : d(d)
    {
        d->changingDB = true;
    }

    ~ChangingDB()
    {
        d->changingDB = false;
    }

public:

    CollectionManager::Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_COLLECTION_MANAGER_P_H
