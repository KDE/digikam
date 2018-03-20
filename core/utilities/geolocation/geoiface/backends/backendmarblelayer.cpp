/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-08
 * Description : Internal part of the Marble-backend for geolocation interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "backendmarblelayer.h"

// Marble includes

#include <marble/GeoPainter.h>

// Local includes

#include "backendmarble.h"

namespace Digikam
{

BackendMarbleLayer::BackendMarbleLayer(BackendMarble* const pMarbleBackend)
    : marbleBackend(pMarbleBackend)
{
}

BackendMarbleLayer::~BackendMarbleLayer()
{
}

bool BackendMarbleLayer::render(Marble::GeoPainter* painter,
                                Marble::ViewportParams* /*viewport*/,
                                const QString& renderPos,
                                Marble::GeoSceneLayer* /*layer*/)
{
    if (marbleBackend && (renderPos == QLatin1String("HOVERS_ABOVE_SURFACE")))
    {
        marbleBackend->marbleCustomPaint(painter);
        return true;
    }

    return false;
}

QStringList BackendMarbleLayer::renderPosition () const
{
    QStringList layerNames;
    layerNames << QLatin1String("HOVERS_ABOVE_SURFACE" );
    return layerNames;
}

void BackendMarbleLayer::setBackend(BackendMarble* const pMarbleBackend)
{
    marbleBackend = pMarbleBackend;
}

} // namespace Digikam
