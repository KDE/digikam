/* ============================================================
 * File  : digikampluginmanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-31
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qfile.h>
#include <qvariant.h>

#include <ktrader.h>
#include <kxmlguifactory.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kaction.h>

#include "plugin.h"

#include "digikamapp.h"
#include "digikampluginmanager.h"

DigikamPluginManager::DigikamPluginManager(QObject *parent)
    : QObject(parent)
{
    instance_ = this;
    loadPlugins();
}

DigikamPluginManager::~DigikamPluginManager()
{
    instance_ = 0;
}

void DigikamPluginManager::loadPlugins()
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/Plugin");

    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter) {

        KService::Ptr service = *iter;

        Digikam::Plugin *plugin =
            KParts::ComponentFactory
            ::createInstanceFromService<Digikam::Plugin>(
                service, this, 0, 0);

        if (plugin) {
            pluginList_.append(plugin);
            ((DigikamApp *)parent())->guiFactory()->addClient(plugin);

            QVariant variant(service->property("X-Digikam-MergeMenu"));
            if (!variant.isNull()) {
                bool merge = variant.toBool();
                if (merge) {
                    KActionCollection *actions = plugin->actionCollection();
                    for (unsigned int i=0; i < actions->count(); i++) {
                        menuMergeActions_.append(actions->action(i));
                    }
                }
            }

            kdDebug() << "DigikamPluginManager: Loaded plugin "
                      << plugin->name() << endl;
        }

    }
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

DigikamPluginManager* DigikamPluginManager::instance_ = 0;
