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

#ifndef EXTERNALDRAW_H
#define EXTERNALDRAW_H

// Qt includes

#include <QtCore/QObject>

// Marble includes

#include <marble/RenderPlugin.h>
#include <marble/MarbleWidget.h>
#include <marble/MarbleMap.h>

#define EXTERNALDRAWPLUGIN_IDENTIFIER "externaldraw"

namespace Marble
{

class ExternalDrawPlugin : public RenderPlugin
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( ExternalDrawPlugin )

public:
    ExternalDrawPlugin();
    
    QStringList backendTypes() const;

    QString renderPolicy() const;

    QStringList renderPosition() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString description() const;

    QIcon icon () const;


    void initialize ();

    bool isInitialized () const;


    bool render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer = 0 );
    
    typedef void (*RenderCallbackFunction)(GeoPainter *painter, void* yourdata);
    RenderCallbackFunction renderCallbackFunction;
    void* renderCallbackFunctionData;
    
    /**
     * @brief Install a callback function
     *
     * Use this function to install your callback function which draws on the
     * MarbleWidget. Set yourFunction to zero to delete the callback function.
     *
     * @param yourFunction Pointer to callback function
     * @param yourdata Pointer to user data of the callback function
     */
    void setRenderCallback(RenderCallbackFunction yourFunction, void* yourdata)
    {
        renderCallbackFunction = yourFunction;
        renderCallbackFunctionData = yourdata;
    }
    
    /**
     * @brief Find the ExternalDrawPlugin instance for a given MarbleWidget
     *
     * Use this function in your client application to find an instance of
     * the plugin, then call setRenderCallback to set up your callback
     * function.
     *
     * @param marbleWidget Instance of Marble::MarbleWidget to search for a plugin instance
     * @return Pointer to an instance of the plugin or 0 if the plugin was not found
     */
    static ExternalDrawPlugin* findPluginInstance(MarbleWidget* const marbleWidget)
    {
        Marble::MarbleMap* const marbleMap = marbleWidget->map();
        
        // find the ExternalDraw plugin:
        const QList<Marble::RenderPlugin*> renderPlugins = marbleMap->renderPlugins();
        Marble::ExternalDrawPlugin* myPlugin = 0;
        for (QList<Marble::RenderPlugin*>::const_iterator it = renderPlugins.constBegin(); it!=renderPlugins.constEnd(); ++it)
        {
            // TODO: find a stricter way to verify that it is the right plugin
            // tried qobject_cast, but that did not work because we do not link to the plugin
            if ((*it)->nameId()==EXTERNALDRAWPLUGIN_IDENTIFIER)
            {
                myPlugin = reinterpret_cast<Marble::ExternalDrawPlugin*>(*it);
                break;
            }
        }
        
        return myPlugin;
    }
    
    
private:
    Q_DISABLE_COPY(ExternalDrawPlugin)
    
};

}

#endif // EXTERNALDRAW_H
