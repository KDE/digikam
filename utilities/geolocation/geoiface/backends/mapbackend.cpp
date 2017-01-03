/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Base-class for backends for GeoIface
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mapbackend.h"

namespace GeoIface
{

MapBackend::MapBackend(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData, QObject* const parent)
    : QObject(parent), s(sharedData)
{
}

MapBackend::~MapBackend()
{
}

void MapBackend::slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)
{
    Q_UNUSED(index)
    Q_UNUSED(pixmap)
}

void MapBackend::slotTrackManagerChanged()
{
}

} /* namespace GeoIface */
