/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins loader for image editor.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepluginloader.h"

// Qt includes

#include <QMap>
#include <QList>

// KDE includes

#include <kservicetypetrader.h>
#include <klocalizedstring.h>
#include <kxmlguiclient.h>

// Local includes

#include "digikam_debug.h"
#include "dsplashscreen.h"

namespace Digikam
{

// List of obsolete image plugins name.

static const char* ObsoleteImagePluginsList[] =
{
    "digikamimageplugin_blowup",                   // Merged with "Resize" tool since 0.9.2.
    "digikamimageplugin_solarize",                 // Renamed "ColorFx" since 0.9.2.
    "digikamimageplugin_unsharp",                  // Merged with "Sharpen" tool since 0.9.2.
    "digikamimageplugin_refocus",                  // Merged with "Sharpen" tool since 0.9.2.
    "digikamimageplugin_despeckle",                // Renamed "Noise Reduction" since 0.9.2.
    "digikamimageplugin_antivignetting",           // Merged with "Lens Correction" since 0.10.0.
    "digikamimageplugin_lensdistortion",           // Merged with "Lens Correction" since 0.10.0.
    "digikamimageplugin_noisereduction",           // Merged to core plugin since 1.0.0-rc.
    "digikamimageplugin_infrared",                 // Merged to core plugin with B&W converter since 1.2.0.
    "digikamimageplugin_whitebalance",             // Merged to core plugin since 1.2.0.
    "digikamimageplugin_channelmixer",             // Merged to core plugin since 1.2.0.
    "digikamimageplugin_adjustcurves",             // Merged to core plugin since 1.2.0.
    "digikamimageplugin_adjustlevels",             // Merged to core plugin since 1.2.0.
    "digikamimageplugin_localcontrast",            // Merged to core plugin since 1.2.0.
    "digikamimageplugin_colorfx",                  // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_charcoal",                 // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_emboss",                   // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_oilpaint",                 // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_blurfx",                   // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_distortionfx",             // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_raindrop",                 // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_filmgrain",                // Merged to "FxFilters" plugin since 1.2.0.
    "digikamimageplugin_perspective",              // Merged to "Transform" plugin since 1.2.0.
    "digikamimageplugin_freerotation",             // Merged to "Transform" plugin since 1.2.0.
    "digikamimageplugin_sheartool",                // Merged to "Transform" plugin since 1.2.0.
    "digikamimageplugin_contentawareresizing",     // Merged to "Transform" plugin since 1.2.0.
    "digikamimageplugin_inserttext",               // Merged to "Decorate" plugin since 1.2.0.
    "digikamimageplugin_border",                   // Merged to "Decorate" plugin since 1.2.0.
    "digikamimageplugin_texture",                  // Merged to "Decorate" plugin since 1.2.0.
    "digikamimageplugin_superimpose",              // Merged to "Decorate" plugin since 1.2.0.
    "digikamimageplugin_restoration",              // Merged to "Enhance" plugin since 1.2.0.
    "digikamimageplugin_inpainting",               // Merged to "Enhance" plugin since 1.2.0.
    "digikamimageplugin_lenscorrection",           // Merged to "Enhance" plugin since 1.2.0.
    "digikamimageplugin_hotpixels",                // Merged to "Enhance" plugin since 1.2.0.
    "digikamimageplugin_core",                     // Renamed "Color" plugin since 1.2.0.
    "digikamimageplugin_decorate",
    "digikamimageplugin_fxfilters",
    "digikamimageplugin_color",
    "digikamimageplugin_enhance",
    "digikamimageplugin_transform",
    "-1"
};

class ImagePluginLoader::Private
{

public:

    Private()
    {
        splash = 0;

        for (int i = 0 ; QLatin1String(ObsoleteImagePluginsList[i]) != QLatin1String("-1") ; ++i)
        {
            obsoleteImagePluginsList << QLatin1String(ObsoleteImagePluginsList[i]);
        }
    }

    ImagePlugin* pluginIsLoaded(const QString& name) const
    {
        return pluginMap.value(name);
    }

public:

    QStringList                  obsoleteImagePluginsList;
    DSplashScreen*               splash;

    // a map of _loaded_ plugins
    QMap<QString, ImagePlugin*>  pluginMap;
    // a map of _available_ plugins
    QMap<QString, KService::Ptr> pluginServiceMap;
};

ImagePluginLoader* ImagePluginLoader::m_instance = 0;

ImagePluginLoader* ImagePluginLoader::instance()
{
    return m_instance;
}

ImagePluginLoader::ImagePluginLoader(QObject* const parent, DSplashScreen* const splash)
    : QObject(parent), d(new Private)
{
    m_instance                  = this;
    d->splash                   = splash;
    const KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("Digikam/ImagePlugin"));

    foreach(const KService::Ptr& service, offers)
    {
        if (service)
        {
            d->pluginServiceMap[service->name()] = service;
        }
    }

    QStringList imagePluginsList2Load;

    foreach(const KService::Ptr& service, d->pluginServiceMap)
    {
        if (!d->obsoleteImagePluginsList.contains(service->library()))
        {
            imagePluginsList2Load.append(service->name());
        }
    }

    loadPluginsFromList(imagePluginsList2Load);
}

ImagePluginLoader::~ImagePluginLoader()
{
    qDeleteAll(d->pluginMap.values());

    delete d;
    m_instance = 0;
}

void ImagePluginLoader::loadPluginsFromList(const QStringList& pluginsToLoad)
{
    if (d->splash)
    {
        d->splash->setMessage(i18n("Loading Image Plugins..."));
    }

    int cpt = 0;

    foreach(const QString& name, pluginsToLoad)
    {
        KService::Ptr service = d->pluginServiceMap.value(name);
        ImagePlugin* plugin   = 0;

        if (d->pluginIsLoaded(name))
        {
            continue;
        }
        else
        {
            QString error;

            plugin = service->createInstance<ImagePlugin>(this, QVariantList(), &error);

            if (plugin && (dynamic_cast<KXMLGUIClient*>(plugin) != 0))
            {
                d->pluginMap[name] = plugin;

                qCDebug(DIGIKAM_GENERAL_LOG) << "ImagePluginLoader: Loaded plugin " << service->name();

                ++cpt;
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "ImagePluginLoader: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error;
            }
        }
    }

    d->splash = 0; // Splashcreen is only displayed at the first time.
}

ImagePlugin* ImagePluginLoader::pluginInstance(const QString& libraryName) const
{
    foreach(const KService::Ptr& service, d->pluginServiceMap)
    {
        if (service->library() == libraryName)
        {
            return (d->pluginIsLoaded(service->name()));
        }
    }

    return 0;
}

bool ImagePluginLoader::pluginLibraryIsLoaded(const QString& libraryName) const
{
    foreach(const KService::Ptr& service, d->pluginServiceMap)
    {
        if (service->library() == libraryName)
        {
            if (d->pluginIsLoaded(service->name()))
            {
                return true;
            }
        }
    }

    return false;
}

QList<ImagePlugin*> ImagePluginLoader::pluginList() const
{
    return d->pluginMap.values();
}

}  // namespace Digikam
