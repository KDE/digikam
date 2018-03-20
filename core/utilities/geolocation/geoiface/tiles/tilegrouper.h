/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Merges tiles into groups
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef TILE_GROUPER_H
#define TILE_GROUPER_H

// Local includes

#include "digikam_export.h"
#include "geoifacecommon.h"

namespace Digikam
{
class MapBackend;

class DIGIKAM_EXPORT TileGrouper : public QObject
{
    Q_OBJECT

public:

    TileGrouper(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData, QObject* const parent);
    ~TileGrouper();

    void setClustersDirty();
    bool getClustersDirty() const;
    void updateClusters();
    void setCurrentBackend(MapBackend* const backend);

private:

    bool currentBackendReady();

private:

    class Private;
    const QScopedPointer<Private> d;

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;
};

} // namespace Digikam

#endif // TILE_GROUPER_H
