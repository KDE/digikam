//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMPLUGINMANAGER.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//                            Richard Groult <Richard dot Groult at jalix.org>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt include.

#include <qfile.h>
#include <qvariant.h>

// KDE include.

#include <ktrader.h>
#include <kxmlguifactory.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kaction.h>

// Local includes.

#ifdef HAVE_KIPI                         // libKIPI support.
   #include <libkipi/plugin.h>
   #include <libkipi/pluginloader.h>
#else                                    // DigikamPlugins support.
   #include "plugin.h"
#endif

#include "digikamapp.h"
#include "digikampluginmanager.h"


DigikamPluginManager::DigikamPluginManager(QObject *parent)
                    : QObject(parent)
{
    instance_ = this;
    
    initAvailablePluginList();
    pluginList_.setAutoDelete(true);
}

DigikamPluginManager::~DigikamPluginManager()
{
    instance_ = 0;
}

void DigikamPluginManager::loadPlugins()
{
    #ifdef HAVE_KIPI                         // libKIPI support.
       KTrader::OfferList offers = KTrader::self()->query("KIPI/Plugin");
    #else                                    // DigikamPlugins support.
       KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
    #endif

    KTrader::OfferList::ConstIterator iter;
    
    for (iter = offers.begin() ; iter != offers.end() ; ++iter) 
        {
        KService::Ptr service = *iter;

        #ifdef HAVE_KIPI                         // libKIPI support.
           KIPI::Plugin *plugin = KParts::ComponentFactory
                                           ::createInstanceFromService<KIPI::Plugin>
                                           (service, this, 0, 0);
        #else                                    // DigikamPlugins support.
           Digikam::Plugin *plugin = KParts::ComponentFactory
                                           ::createInstanceFromService<Digikam::Plugin>
                                           (service, this, 0, 0);
        #endif
                                           

        if (plugin) 
            {
            pluginList_.append(plugin);
            ((DigikamApp *)parent())->guiFactory()->addClient(plugin);
            
            #ifdef HAVE_KIPI                         // libKIPI support.
               QVariant variant(service->property("X-KIPI-MergeMenu"));
            #else                                    // DigikamPlugins support.
               QVariant variant(service->property("X-Digikam-MergeMenu"));
            #endif

            if (!variant.isNull()) 
                {
                bool merge = variant.toBool();
                
                if (merge) 
                    {
                    KActionCollection *actions = plugin->actionCollection();
                    
                    for (unsigned int i=0 ; i < actions->count() ; ++i) 
                        {
                        menuMergeActions_.append(actions->action(i));
                        }
                    }
                }

            kdDebug() << "DigikamPluginManager: Loaded plugin "
                      << plugin->name() << endl;
            }
        }
}

#ifdef HAVE_KIPI                         // libKIPI support.
   KIPI::Plugin* DigikamPluginManager::pluginIsLoaded(QString pluginName)
#else                                    // DigikamPlugins support.
   Digikam::Plugin* DigikamPluginManager::pluginIsLoaded(QString pluginName)
#endif
{
	if (pluginList_.isEmpty()) return NULL;

        #ifdef HAVE_KIPI                         // libKIPI support.
	   KIPI::Plugin *plugin;
        #else                                    // DigikamPlugins support.
	   Digikam::Plugin *plugin;
        #endif
        
	for ( plugin = pluginList_.first(); plugin ; plugin = pluginList_.next() )
	   {
	   if(plugin->name() == pluginName)
	   return plugin;
	   }
           
	return NULL;
}


void DigikamPluginManager::loadPlugins(QStringList list)
{
        #ifdef HAVE_KIPI                         // libKIPI support.
	   KTrader::OfferList offers = KTrader::self()->query("KIPI/Plugin");
        #else                                    // DigikamPlugins support.
           KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
        #endif
	
	KTrader::OfferList::ConstIterator iter;
	
        for (iter = offers.begin() ; iter != offers.end() ; ++iter)
	    {
		KService::Ptr service = *iter;
		
                #ifdef HAVE_KIPI                         // libKIPI support.                
                   KIPI::Plugin *plugin;
                #else                                    // DigikamPlugins support.
                   Digikam::Plugin *plugin;
                #endif

		if (!list.contains(service->name()))
		{
			if((plugin=pluginIsLoaded(service->name())) != NULL)
			{
				((DigikamApp *)parent())->guiFactory()->removeClient(plugin);
				pluginList_.remove(plugin);
			}

		}
		else
		if(pluginIsLoaded(service->name()))
			continue;
		else
		{
			 plugin =
				 KParts::ComponentFactory
				 ::createInstanceFromService<Digikam::Plugin>(
				     service, this, 0, 0);

			if (plugin)
			{
			 	pluginList_.append(plugin);
				((DigikamApp *)parent())->guiFactory()->addClient(plugin);
                                
				#ifdef HAVE_KIPI                         // libKIPI support.
				   QVariant variant(service->property("X-KIPI-MergeMenu"));
				#else                                    // DigikamPlugins support.
				   QVariant variant(service->property("X-Digikam-MergeMenu"));
				#endif
				
			 	if (!variant.isNull())
			 	{
			 		bool merge = variant.toBool();
			 		if (merge)
			 		{
			 			KActionCollection *actions = plugin->actionCollection();
			 			for (unsigned int i=0 ; i < actions->count() ; ++i)
			 			{
			 				menuMergeActions_.append(actions->action(i));
			 			}
			 		}
			 	}
				
			 	kdDebug() << "DigikamPluginManager: Loaded plugin "
			 		<< plugin->name() << endl;
			}
		}
	}
}

void DigikamPluginManager::initAvailablePluginList()
{
    #ifdef HAVE_KIPI                         // libKIPI support.    
       KTrader::OfferList offers = KTrader::self()->query("KIPI/Plugin");
    #else                                    // DigikamPlugins support.
       KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
    #endif
    
    KTrader::OfferList::ConstIterator iter;
    
    for(iter = offers.begin() ; iter != offers.end() ; ++iter)
        {
	KService::Ptr service = *iter;
	availablePluginList_.append(service->name());
	availablePluginList_.append(service->comment());
        }
}

const QStringList DigikamPluginManager::availablePluginList()
{
    return availablePluginList_;
}

const QPtrList<Digikam::Plugin>& DigikamPluginManager::pluginList()
{
    return pluginList_;    
}

const QPtrList<KAction>& DigikamPluginManager::menuMergeActions()
{
    return menuMergeActions_;
}

DigikamPluginManager* DigikamPluginManager::instance()
{
    return instance_;
}

const QStringList DigikamPluginManager::loadedPluginList()
{
    #ifdef HAVE_KIPI                         // libKIPI support.   
       KIPI::Plugin *plugin;
    #else                                    // DigikamPlugins support.  
       Digikam::Plugin *plugin;
    #endif
	          
	QStringList list;
        
	for ( plugin = pluginList_.first() ; plugin ; plugin = pluginList_.next() )
	    list.append(plugin->name());
                
	return list;
}

DigikamPluginManager* DigikamPluginManager::instance_ = 0;
