/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Hint data containers for the collection scanner
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONSCANNERHINTS_H
#define COLLECTIONSCANNERHINTS_H

#include "digikam_config.h"

// Qt includes

#include <QDateTime>
#include <QList>
#include <QStringList>

#ifdef HAVE_DBUS
#include <QDBusArgument>
#include "dbenginedbusutils.h"
#endif

// Local includes

#include "digikam_export.h"

namespace Digikam
{

namespace CollectionScannerHints
{

class DIGIKAM_DATABASE_EXPORT Album
{
public:

    Album();
    Album(int albumRootId, int albumId);

    bool isNull()                       const;
    uint qHash()                        const;
    bool operator==(const Album& other) const;

public:

    int albumRootId;
    int albumId;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT DstPath
{
public:

    DstPath();
    DstPath(int albumRootId, const QString& relativePath);

    bool isNull()                         const;
    uint qHash()                          const;
    bool operator==(const DstPath& other) const;

public:

    int     albumRootId;
    QString relativePath;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT Item
{
public:

    Item();
    explicit Item(qlonglong id);

    bool isNull()                      const;
    uint qHash()                       const;
    bool operator==(const Item& other) const;

public:

    qlonglong id;
};

inline uint qHash(const Album& src)
{
    return src.qHash();
}

inline uint qHash(const DstPath& dst)
{
    return dst.qHash();
}

inline uint qHash(const Item& item)
{
    return item.qHash();
}

} // namespace CollectionScannerHints

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumCopyMoveHint
{
public:

    /** An AlbumCopyMoveHint describes an existing album
     *  and a destination to which this album is expected to be
     *  copied, moved or renamed.
     */

    AlbumCopyMoveHint();
    AlbumCopyMoveHint(int srcAlbumRootId, int srcAlbum,
                      int dstAlbumRootId, const QString& dstRelativePath);

    int albumRootIdSrc()                          const;
    int albumIdSrc()                              const;
    bool isSrcAlbum(int albumRootId, int albumId) const;

    CollectionScannerHints::Album src() const
    {
        return m_src;
    }

    int albumRootIdDst()                                          const;
    QString relativePathDst()                                     const;
    bool isDstAlbum(int albumRootId, const QString& relativePath) const;

    CollectionScannerHints::DstPath dst() const
    {
        return m_dst;
    }

    uint qHash() const;

    bool operator==(const CollectionScannerHints::Album& src) const
    {
        return src == m_src;
    }

    bool operator==(const CollectionScannerHints::DstPath& dst) const
    {
        return dst == m_dst;
    }

#ifdef HAVE_DBUS
    AlbumCopyMoveHint& operator<<(const QDBusArgument& argument);
    const AlbumCopyMoveHint& operator>>(QDBusArgument& argument) const;
#endif

    operator const CollectionScannerHints::Album& () const
    {
        return m_src;
    }

    operator const CollectionScannerHints::DstPath& () const
    {
        return m_dst;
    }

protected:

    CollectionScannerHints::Album   m_src;
    CollectionScannerHints::DstPath m_dst;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemCopyMoveHint
{
public:

    /** An ItemCopyMoveHint describes a list of existing items that will
     *  be copied, moved or renamed to an album given by album root id and album id.
     *  In the new album, the items will have the filenames given in dstNames.
     */

    ItemCopyMoveHint();
    ItemCopyMoveHint(const QList<qlonglong>& srcIds, int dstAlbumRootId, int albumId, const QStringList& dstNames);

    QList<qlonglong> srcIds()                     const;
    bool isSrcId(qlonglong id)                    const;
    int albumRootIdDst()                          const;
    int albumIdDst()                              const;
    bool isDstAlbum(int albumRootId, int albumId) const;

    CollectionScannerHints::Album dst() const
    {
        return m_dst;
    }

    QStringList dstNames()        const;
    QString dstName(qlonglong id) const;

    bool operator==(const CollectionScannerHints::Album& dst) const
    {
        return dst == m_dst;
    }

#ifdef HAVE_DBUS
    ItemCopyMoveHint& operator<<(const QDBusArgument& argument);
    const ItemCopyMoveHint& operator>>(QDBusArgument& argument) const;
#endif

    operator const CollectionScannerHints::Album& () const
    {
        return m_dst;
    }

protected:

    QList<qlonglong>              m_srcIds;
    CollectionScannerHints::Album m_dst;
    QStringList                   m_dstNames;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemChangeHint
{
public:

    /** An ItemCopyMoveHint describes a list of existing items that
     *  should be updated although the modification date may not have changed.
     */

    enum ChangeType
    {
        ItemModified, /// treat as if modification date changed
        ItemRescan    /// reread metadata
    };

public:

    ItemChangeHint();
    explicit ItemChangeHint(QList<qlonglong> srcIds, ChangeType type = ItemModified);

    QList<qlonglong> ids()  const;
    bool isId(qlonglong id) const;
    ChangeType changeType() const;

    bool isModified() const
    {
        return changeType() == ItemModified;
    }

    bool needsRescan() const
    {
        return changeType() == ItemRescan;
    }

#ifdef HAVE_DBUS
    ItemChangeHint& operator<<(const QDBusArgument& argument);
    const ItemChangeHint& operator>>(QDBusArgument& argument) const;
#endif

protected:

    QList<qlonglong>  m_ids;
    ChangeType        m_type;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemMetadataAdjustmentHint
{
public:

    /** The file's has been edited writing out information from
      *  the database, i.e., the db is already guaranteed to contain
      *  all changed information in the file's metadata.
      *  There is no need for a full rescan, optimizations are possible.
      */

    enum AdjustmentStatus
    {
        AboutToEditMetadata,       /// The file is about to be edited. Suspends scanning. The Finished hint must follow.
        MetadataEditingFinished,   /// The file's metadata has been edited as described above.
        MetadataEditingAborted     /// The file's metadata has not been edited, despite sending AboutToEditMedata
    };

public:

    ItemMetadataAdjustmentHint();
    explicit ItemMetadataAdjustmentHint(qlonglong id, AdjustmentStatus status,
                                        const QDateTime& modificationDateOnDisk, qlonglong fileSize);

    qlonglong id()                      const;
    AdjustmentStatus adjustmentStatus() const;
    QDateTime modificationDate()        const;
    qlonglong fileSize()                const;

    bool isAboutToEdit() const
    {
        return adjustmentStatus() == AboutToEditMetadata;
    }

    bool isEditingFinished() const
    {
        return adjustmentStatus() == MetadataEditingFinished;
    }

    bool isEditingFinishedAborted() const
    {
        return adjustmentStatus() == MetadataEditingAborted;
    }

#ifdef HAVE_DBUS
    ItemMetadataAdjustmentHint& operator<<(const QDBusArgument& argument);
    const ItemMetadataAdjustmentHint& operator>>(QDBusArgument& argument) const;
#endif

protected:

    qlonglong         m_id;
    AdjustmentStatus  m_status;
    QDateTime         m_modificationDate;
    qlonglong         m_fileSize;
};

inline uint qHash(const Digikam::AlbumCopyMoveHint& hint)
{
    return hint.qHash();
}

} // namespace Digikam

#ifdef HAVE_DBUS
DECLARE_METATYPE_FOR_DBUS(Digikam::AlbumCopyMoveHint)
DECLARE_METATYPE_FOR_DBUS(Digikam::ItemCopyMoveHint)
DECLARE_METATYPE_FOR_DBUS(Digikam::ItemChangeHint)
#endif

#endif // COLLECTIONSCANNERHINTS_H
