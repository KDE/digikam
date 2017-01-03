/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Internal part of the Marble-backend for GeoIface
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

#ifndef BACKEND_MAP_MARBLE_LAYER_H
#define BACKEND_MAP_MARBLE_LAYER_H

// Qt includes

#include <QtCore/QPointer>

// Marble includes

#include <marble/LayerInterface.h>

/// @cond false
namespace Marble
{
    class GeoPainter;
    class ViewportParams;
    class GeoSceneLayer;
}
/// @endcond

namespace GeoIface
{

class BackendMarble;

class BackendMarbleLayer : public Marble::LayerInterface
{
public:

    explicit BackendMarbleLayer(BackendMarble* const pMarbleBackend);
    virtual ~BackendMarbleLayer();

    virtual bool render(Marble::GeoPainter* painter, Marble::ViewportParams* viewport,
                        const QString& renderPos = QLatin1String( "NONE"), Marble::GeoSceneLayer* layer = 0);
    virtual QStringList renderPosition () const;

    void setBackend(BackendMarble* const pMarbleBackend);

private:

    QPointer<BackendMarble> marbleBackend;
};

} /* namespace GeoIface */

#endif /* BACKEND_MAP_MARBLE_LAYER_H */
