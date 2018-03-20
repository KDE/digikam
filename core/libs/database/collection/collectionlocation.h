/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location abstraction
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONLOCATION_H
#define COLLECTIONLOCATION_H

// Qt includes

#include <QString>
#include <QHash>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CollectionLocation
{

public:

    enum Status
    {
        /** An invalid status. A location has this status if it is not valid,
         *  and it had this status before its creation (for oldStatus information)
         */
        LocationNull,

        /** The location if available. This is the most common status.
         */
        LocationAvailable,

        /** The location is explicitly hidden. This gives no information if
         *  the location was available were it not hidden.
         */
        LocationHidden,

        /** The location is currently not available. (Harddisk unplugged, CD not in drive,
         *  network fs not mounted etc.) It may become available any time.
         */
        LocationUnavailable,

        /** An invalid status. A location object acquires this status if it has been deleted.
         *  The object then does no longer point to an existing location.
         */
        LocationDeleted
    };

public:

    enum Type
    {
        /** The location is located on a storage device that is built-in
         *  without frequent removal: Hard-disk inside the machine
         */
        TypeVolumeHardWired = AlbumRoot::VolumeHardWired,

        /** The location is located on a storage device that can be removed
         *  from the local machine, and is expected to be removed.
         *  USB stick, USB hard-disk, CD, DVD
         */
        TypeVolumeRemovable = AlbumRoot::VolumeRemovable,

        /** The location is available via a network file system.
         *  The availability depends on the network connection.
         */
        TypeNetwork         = AlbumRoot::Network
    };

public:

    CollectionLocation();

    /** The id uniquely identifying this collection
     */
    int     id() const;

    /** The current status. See above for possible values.
     */
    Status  status() const;

    /** The type of location. See above for possible values.
     */
    Type    type() const;

    /** The current file system path leading to this album root.
     *  Only guaranteed to be valid for location with status Available.
     */
    QString albumRootPath() const;

    /** A user-visible, optional label.
     */
    QString label() const;

    bool isAvailable() const
    {
        return m_status == LocationAvailable;
    }

    bool isNull() const
    {
        return m_status == LocationNull;
    }

    uint hash() const
    {
        return ::qHash(m_id);
    }

protected:

    int     m_id;
    QString m_path;
    Status  m_status;
    Type    m_type;
    QString m_label;
};

}  // namespace Digikam

inline uint qHash(const Digikam::CollectionLocation& loc)
{
    return loc.hash();
}

#endif // COLLECTIONLOCATION_H
