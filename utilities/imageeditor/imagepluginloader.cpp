/* ============================================================
 * File  : imagepluginloader.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>

#include "imagepluginloader.h"

ImagePluginLoader* ImagePluginLoader::m_instance=0;

ImagePluginLoader::ImagePluginLoader(QObject *parent)
    : QObject(parent)
{
    m_instance = this;

    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;
    
    for (iter = offers.begin() ; iter != offers.end() ; ++iter) {

        KService::Ptr service = *iter;

        Digikam::ImagePlugin *plugin =
            KParts::ComponentFactory
            ::createInstanceFromService<Digikam::ImagePlugin>(service, this,
                                                              0, 0);

        if (plugin) {

            m_pluginList.append(plugin);

            kdDebug() << "ImagePluginLoader: Loaded plugin "
                      << plugin->name() << endl;
        }
    }
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
