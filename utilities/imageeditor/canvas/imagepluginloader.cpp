/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description : load digiKam image editor plugins list 
 *               configured in setup dialog.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
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
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>
#include <kxmlguiclient.h>

// Local includes.

#include "imagepluginloader.h"
#include "splashscreen.h"

ImagePluginLoader* ImagePluginLoader::m_instance=0;

ImagePluginLoader::ImagePluginLoader(QObject *parent, SplashScreen *splash)
                 : QObject(parent)
{
    m_instance = this;
    m_splash   = splash;
    
    QStringList imagePluginsList2Load;
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");  
    
    if ( config->readEntry("ImagePlugins List").isNull() )   
    {
        KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
        KTrader::OfferList::ConstIterator iter;
    
        for (iter = offers.begin() ; iter != offers.end() ; ++iter) 
        {
            KService::Ptr service = *iter;
            imagePluginsList2Load.append(service->library());
        }
    }
    else
        imagePluginsList2Load = config->readListEntry("ImagePlugins List");

    loadPluginsFromList(imagePluginsList2Load);
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
        Digikam::ImagePlugin *plugin;

        if (service->library() == "digikamimageplugin_core")
        {
            if (!pluginIsLoaded(service->name()) )
            {
                int error;
                plugin = KParts::ComponentFactory
                         ::createInstanceFromService<Digikam::ImagePlugin>(service, this,
                                                                           service->name().local8Bit(),
                                                                           0, &error);

                if (plugin && (dynamic_cast<KXMLGUIClient*>(plugin) != 0))
                {
                    m_pluginList.append(PluginType(service->name(), plugin));
                
                    kdDebug() << "ImagePluginLoader: Loaded plugin " << service->name() << endl;
                 
                    if (m_splash)      
                        m_splash->message(i18n("Loading: %1").arg(service->name()));
                    
                    ++cpt;
                }
                else
                {
                    kdWarning(  ) << "KIPI::PluginLoader:: createInstanceFromLibrary returned 0 for "
                                  << service->name()
                                  << " (" << service->library() << ")"
                                  << " with error number "
                                  << error << endl;
                    if (error == KParts::ComponentFactory::ErrNoLibrary)
                        kdWarning(  ) << "KLibLoader says: "
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
        Digikam::ImagePlugin *plugin;

        if (!list.contains(service->library()) && service->library() != "digikamimageplugin_core")
        {
            if ((plugin = pluginIsLoaded(service->name())) != NULL)
                m_pluginList.remove(PluginType(service->name(),plugin));
        }
        else 
        {
            if( pluginIsLoaded(service->name()) )
                continue;
            else
            {
                plugin = KParts::ComponentFactory
                         ::createInstanceFromService<Digikam::ImagePlugin>(service, this,
                                                                           service->name().local8Bit(),
                                                                           0);

                if (plugin)
                {
                    m_pluginList.append(PluginType(service->name(), plugin));
                
                    kdDebug() << "ImagePluginLoader: Loaded plugin " << service->name() << endl;
                
                    if (m_splash)      
                        m_splash->message(i18n("Loading: %1").arg(service->name()));
                
                    ++cpt;
                }
            }
        }
    }

    if (m_splash)      
        m_splash->message(i18n("1 Image Plugin Loaded", "%n Image Plugins Loaded", cpt));    

    m_splash = 0;        // Splashcreen is only lanched at the first time.
                         // If user change plugins list to use in setup, don't try to 
                         // use the old splashscreen instance.
}

Digikam::ImagePlugin* ImagePluginLoader::pluginIsLoaded(const QString& name)
{
    if ( m_pluginList.isEmpty() )
        return 0;

    for (PluginList::iterator it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
    {
        if ((*it).first == name )
            return (*it).second;
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

ImagePluginLoader::~ImagePluginLoader()
{
    m_instance = 0;
}

ImagePluginLoader* ImagePluginLoader::instance()
{
    return m_instance;
}

QPtrList<Digikam::ImagePlugin> ImagePluginLoader::pluginList()
{
    QPtrList<Digikam::ImagePlugin> list;

    for (PluginList::iterator it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
    {
        list.append((*it).second);
    }
        
    return list;
}
