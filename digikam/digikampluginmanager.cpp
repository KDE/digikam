//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMPLUGINMANAGER.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
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

#include "digikamapp.h"
#include "digikampluginmanager.h"
#include "plugin.h"

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
    KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
    KTrader::OfferList::ConstIterator iter;
    
    for (iter = offers.begin() ; iter != offers.end() ; ++iter) 
        {
        KService::Ptr service = *iter;

        Digikam::Plugin *plugin = KParts::ComponentFactory
                                           ::createInstanceFromService<Digikam::Plugin>
                                           (service, this, 0, 0);

        if (plugin) 
            {
            pluginList_.append(plugin);
            ((DigikamApp *)parent())->guiFactory()->addClient(plugin);
            
            QVariant variant(service->property("X-Digikam-MergeMenu"));

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

Digikam::Plugin* DigikamPluginManager::pluginIsLoaded(QString pluginName)
{
	if (pluginList_.isEmpty()) return NULL;

        Digikam::Plugin *plugin;
        
	for ( plugin = pluginList_.first(); plugin ; plugin = pluginList_.next() )
	   {
	   if(plugin->name() == pluginName)
	   return plugin;
	   }
           
	return NULL;
}


void DigikamPluginManager::loadPlugins(QStringList list)
{
        KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
	KTrader::OfferList::ConstIterator iter;
	
        for (iter = offers.begin() ; iter != offers.end() ; ++iter)
	    {
		KService::Ptr service = *iter;
                Digikam::Plugin *plugin;

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
                                
  			        QVariant variant(service->property("X-Digikam-MergeMenu"));
				
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
    KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");
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
    Digikam::Plugin *plugin;
	          
	QStringList list;
        
	for ( plugin = pluginList_.first() ; plugin ; plugin = pluginList_.next() )
	    list.append(plugin->name());
                
	return list;
}

DigikamPluginManager* DigikamPluginManager::instance_ = 0;

