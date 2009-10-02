/* ============================================================
 *
 * This file is a part of markerclusterholder, developed
 * for digikam and trippy
 *
 * Date        : 2009-09-03
 * Description : callback plugin for Marble
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
 * Based on test-plugin for Marble, (C) 2009 Torsten Rahn
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


#include "externaldraw.h"
#include "externaldraw.moc"

#include <marble/MarbleDirs.h>
#include <marble/GeoPainter.h>
#include <marble/GeoDataCoordinates.h>

namespace Marble
{
  
ExternalDrawPlugin::ExternalDrawPlugin()
: RenderPlugin(), renderCallbackFunction(0), renderCallbackFunctionData(0)
{
}

QStringList ExternalDrawPlugin::backendTypes() const
{
    return QStringList( "externaldraw" );
}

QString ExternalDrawPlugin::renderPolicy() const
{
    return QString( "ALWAYS" );
}

QStringList ExternalDrawPlugin::renderPosition() const
{
    // this is behind the "crosshair" but in front of the coordinate grid:
    return QStringList( "HOVERS_ABOVE_SURFACE" );
}

QString ExternalDrawPlugin::name() const
{
    return tr( "ExternalDraw" );
}

QString ExternalDrawPlugin::guiString() const
{
    return tr( "ExternalDraw" );
}

QString ExternalDrawPlugin::nameId() const
{
    return QString( EXTERNALDRAWPLUGIN_IDENTIFIER );
}

QString ExternalDrawPlugin::description() const
{
    return tr( "Plugin to allow client applications to draw in layers. Used by Digikam." );
}

QIcon ExternalDrawPlugin::icon () const
{
    return QIcon();
}


void ExternalDrawPlugin::initialize ()
{
}

bool ExternalDrawPlugin::isInitialized () const
{
    return true;
}

bool ExternalDrawPlugin::render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer )
{
    Q_UNUSED(viewport)
    Q_UNUSED(renderPos)
    Q_UNUSED(layer)
    
    if (renderCallbackFunction)
    {
        renderCallbackFunction(painter, renderCallbackFunctionData);
    }
    
    return true;
}

}

Q_EXPORT_PLUGIN2( ExternalDrawPlugin, Marble::ExternalDrawPlugin )

