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

void ImagePluginLoader::loadPluginsFromList(QStringList list)
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    int cpt = 0;
    
    // Load plugin core at the first time.

    for(iter = offers.begin(); iter != offers.end(); ++iter)
       {
        KService::Ptr service = *iter;
        Digikam::ImagePlugin *plugin;

        if(service->library() == "digikamimageplugin_core")
           {
           if( pluginIsLoaded(service->name()) )
              break;
           else
              {
              plugin = KParts::ComponentFactory
                       ::createInstanceFromService<Digikam::ImagePlugin>(service, this, 0, 0);

              if (plugin)
                 {
                 m_pluginList.append(plugin);
                
                 kdDebug() << "ImagePluginLoader: Loaded plugin " << plugin->name() << endl;
                 
                 if (m_splash)      
                    m_splash->message(i18n("Loading: %1").arg(plugin->name()));
                    
                 ++cpt;
                 }
                                
              break;
              }
           }
       }
        
    // Load all other image plugins after (make a coherant menu construction in Image Editor).
      
    for(iter = offers.begin(); iter != offers.end(); ++iter)
       {
       KService::Ptr service = *iter;
       Digikam::ImagePlugin *plugin;

       if(!list.contains(service->library()) && service->library() != "digikamimageplugin_core")
          {
          if((plugin = pluginIsLoaded(service->name())) != NULL)
              m_pluginList.remove(plugin);
          }
       else 
          {
          if( pluginIsLoaded(service->name()) )
             continue;
          else
             {
             plugin = KParts::ComponentFactory
                      ::createInstanceFromService<Digikam::ImagePlugin>(service, this, 0, 0);

             if (plugin)
                {
                m_pluginList.append(plugin);
                
                kdDebug() << "ImagePluginLoader: Loaded plugin " << plugin->name() << endl;
                
                if (m_splash)      
                   m_splash->message(i18n("Loading: %1").arg(plugin->name()));
                
                ++cpt;
                }
             }
          }
       }

    if (m_splash)      
       m_splash->message(i18n("%1 Image Plugins loaded").arg(cpt));    

    m_splash = 0;        // Splashcreen is only lanched at the first time.
                         // If user change plugins list to use in setup, don't try to 
                         // use the old splashscreen instance.
}

Digikam::ImagePlugin* ImagePluginLoader::pluginIsLoaded(QString pluginName)
{
    if( m_pluginList.isEmpty() ) return NULL;

    Digikam::ImagePlugin *plugin;
        
    for ( plugin = m_pluginList.first() ; plugin ; plugin = m_pluginList.next() )
       {
       if( plugin->name() == pluginName )
          return plugin;
       }
           
    return NULL;
}

ImagePluginLoader::~ImagePluginLoader()
{
    m_instance = 0;
}

ImagePluginLoader* ImagePluginLoader::instance()
{
    return m_instance;
}

QPtrList<Digikam::ImagePlugin>& ImagePluginLoader::pluginList()
{
    return m_pluginList;
}
