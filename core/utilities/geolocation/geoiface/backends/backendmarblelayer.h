/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-08
 * Description : Internal part of the Marble-backend for geolocation interface
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BACKEND_MARBLE_LAYER_H
#define DIGIKAM_BACKEND_MARBLE_LAYER_H

// Qt includes

#include <QPointer>

// Marble includes

#include <marble/LayerInterface.h>

// Local includes

#include "digikam_export.h"

/// @cond false
namespace Marble
{
    class GeoPainter;
    class ViewportParams;
    class GeoSceneLayer;
}
/// @endcond

namespace Digikam
{

class BackendMarble;

class DIGIKAM_EXPORT BackendMarbleLayer : public Marble::LayerInterface
{
public:

    explicit BackendMarbleLayer(BackendMarble* const pMarbleBackend);
    virtual ~BackendMarbleLayer();

    virtual bool render(Marble::GeoPainter* painter,
                        Marble::ViewportParams* viewport,
                        const QString& renderPos = QLatin1String("NONE"),
                        Marble::GeoSceneLayer* layer = 0);
    virtual QStringList renderPosition () const;

    void setBackend(BackendMarble* const pMarbleBackend);

private:

    QPointer<BackendMarble> marbleBackend;
};

} // namespace Digikam

#endif // DIGIKAM_BACKEND_MARBLE_LAYER_H
