/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location abstraction
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

#ifndef COLLECTIONLOCATION_H
#define COLLECTIONLOCATION_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "albuminfo.h"

namespace Digikam
{

class DIGIKAM_EXPORT CollectionLocation
{

public:

    enum Status
    {
        LocationNull,
        LocationAvailable,
        LocationHidden,
        LocationUnavailable,
        LocationDeleted
    };

    enum Type
    {
        TypeVolumeHardWired = AlbumRoot::VolumeHardWired,
        TypeVolumeRemovable = AlbumRoot::VolumeRemovable,
        TypeNetwork         = AlbumRoot::Network
    };

    CollectionLocation();

    int     id() const;
    Status  status() const;
    Type    type() const;
    QString albumRootPath() const;

    bool isAvailable() const { return m_status == LocationAvailable; }
    bool isNull() const      { return m_status == LocationNull;      }

protected:


    int     m_id;
    QString m_path;
    Status  m_status;
    Type    m_type;
};

}  // namespace Digikam

#endif // COLLECTIONLOCATION_H
