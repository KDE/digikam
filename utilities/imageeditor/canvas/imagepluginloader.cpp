/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
 * Date   : 2004-06-04
 * Description : image plugins loader for  digiKam image editor
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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

// KDE includes.

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kapplication.h>
#include <klocale.h>
#include <kxmlguiclient.h>

// Local includes.

#include "ddebug.h"
#include "splashscreen.h"
#include "imagepluginloader.h"

namespace Digikam
{

// List of obsolete image plugins name. 

static const char* ObsoleteImagePluginsList[] =
{
     "digikamimageplugin_blowup",    // Merged with "Resize" tool since 0.9.2.  
     "digikamimageplugin_solarize",  // Renamed "ColorFx" since 0.9.2.  
     "digikamimageplugin_unsharp",   // Merged with "Sharpen" tool since 0.9.2.
     "digikamimageplugin_refocus",   // Merged with "Sharpen" tool since 0.9.2.
     "digikamimageplugin_despeckle", // Renamed "Noise Reduction" since 0.9.2.
     "-1"
};

class ImagePluginLoaderPrivate
{

public:

    typedef QPair<QString, ImagePlugin*> PluginType;
    typedef QValueList< PluginType >     PluginList;

public:

    ImagePluginLoaderPrivate()
    {
        splash = 0;

        for (int i=0 ; QString(ObsoleteImagePluginsList[i]) != QString("-1") ; i++)
            obsoleteImagePluginsList << ObsoleteImagePluginsList[i];
    }
    
    QStringList   obsoleteImagePluginsList;

    SplashScreen *splash;

    PluginList    pluginList;
};

ImagePluginLoader* ImagePluginLoader::m_instance=0;

ImagePluginLoader* ImagePluginLoader::instance()
{
    return m_instance;
}

ImagePluginLoader::ImagePluginLoader(QObject *parent, SplashScreen *splash)
                 : QObject(parent)
{
    m_instance = this;
    d = new ImagePluginLoaderPrivate;
    d->splash = splash;
 
    QStringList imagePluginsList2Load;
    
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    for (iter = offers.begin() ; iter != offers.end() ; ++iter) 
    {
        KService::Ptr service = *iter;
        if (!d->obsoleteImagePluginsList.contains(service->library()))
            imagePluginsList2Load.append(service->library());
    }
        
    loadPluginsFromList(imagePluginsList2Load);
}

ImagePluginLoader::~ImagePluginLoader()
{
    delete d;
    m_instance = 0;
}

void ImagePluginLoader::loadPluginsFromList(const QStringList& list)
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    int cpt = 0;
    
    // Load plugin core at the first time.

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        ImagePlugin *plugin;

        if (service->library() == "digikamimageplugin_core")
        {
            if (!pluginIsLoaded(service->name()) )
            {
                int error;
                plugin = KParts::ComponentFactory::createInstanceFromService<ImagePlugin>(
                                 service, this, service->name().local8Bit(), 0, &error);

                if (plugin && (dynamic_cast<KXMLGUIClient*>(plugin) != 0))
                {
                    d->pluginList.append(ImagePluginLoaderPrivate::PluginType(service->name(), plugin));
                
                    DDebug() << "ImagePluginLoader: Loaded plugin " << service->name() << endl;
                 
                    if (d->splash)      
                        d->splash->message(i18n("Loading: %1").arg(service->name()));
                    
                    ++cpt;
                }
                else
                {
                    DWarning() << "KIPI::PluginLoader:: createInstanceFromLibrary returned 0 for "
                                << service->name()
                                << " (" << service->library() << ")"
                                << " with error number "
                                << error << endl;
                    if (error == KParts::ComponentFactory::ErrNoLibrary)
                        DWarning() << "KLibLoader says: "
                                    << KLibLoader::self()->lastErrorMessage() << endl;
                }
            }
            break;
        }
    }
    
    // Load all other image plugins after (make a coherant menu construction in Image Editor).
      
    for (iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        ImagePlugin *plugin;

        if (!list.contains(service->library()) && service->library() != "digikamimageplugin_core")
        {
            if ((plugin = pluginIsLoaded(service->name())) != NULL)
                d->pluginList.remove(ImagePluginLoaderPrivate::PluginType(service->name(),plugin));
        }
        else 
        {
            if( pluginIsLoaded(service->name()) )
                continue;
            else
            {
                plugin = KParts::ComponentFactory::createInstanceFromService<ImagePlugin>(
                                 service, this, service->name().local8Bit(), 0);

                if (plugin)
                {
                    d->pluginList.append(ImagePluginLoaderPrivate::PluginType(service->name(), plugin));
                
                    DDebug() << "ImagePluginLoader: Loaded plugin " << service->name() << endl;
                
                    if (d->splash)      
                        d->splash->message(i18n("Loading: %1").arg(service->name()));
                
                    ++cpt;
                }
                else
                {
                    DError() << "ImagePluginLoader: Failed to load plugin " << service->name() << endl;
                }
            }
        }
    }

    if (d->splash)      
        d->splash->message(i18n("1 Image Plugin Loaded", "%n Image Plugins Loaded", cpt));    

    d->splash = 0;       // Splashcreen is only lanched at the first time.
                         // If user change plugins list to use in setup, don't try to 
                         // use the old splashscreen instance.
}

ImagePlugin* ImagePluginLoader::pluginIsLoaded(const QString& name)
{
    if ( d->pluginList.isEmpty() )
        return 0;

    for (ImagePluginLoaderPrivate::PluginList::iterator it = d->pluginList.begin(); 
         it != d->pluginList.end(); ++it)
    {
        if ((*it).first == name )
            return (*it).second;
    }
           
    return 0;
}

ImagePlugin* ImagePluginLoader::pluginInstance(const QString& libraryName)
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;

        if(service->library() == libraryName)
        {
            return ( pluginIsLoaded(service->name()) );
        }
    }
    
    return 0;
}

bool ImagePluginLoader::pluginLibraryIsLoaded(const QString& libraryName)
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;

        if(service->library() == libraryName)
        {
            if( pluginIsLoaded(service->name()) )
                return true;
        }
    }
    
    return false;
}

QPtrList<ImagePlugin> ImagePluginLoader::pluginList()
{
    QPtrList<ImagePlugin> list;

    for (ImagePluginLoaderPrivate::PluginList::iterator it = d->pluginList.begin(); 
         it != d->pluginList.end(); ++it)
    {
        list.append((*it).second);
    }
        
    return list;
}

}  // namespace Digikam
