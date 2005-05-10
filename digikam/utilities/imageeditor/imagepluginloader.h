/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju and Gilles Caulier
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

#ifndef IMAGEPLUGINLOADER_H
#define IMAGEPLUGINLOADER_H

// Qt includes.

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qpair.h>

// Local includes.

#include "imageplugin.h"

class SplashScreen;

class ImagePluginLoader : public QObject
{
    
public:

    ImagePluginLoader(QObject *parent, SplashScreen *splash=0);
    ~ImagePluginLoader();

    static ImagePluginLoader* instance();

    QPtrList<Digikam::ImagePlugin> pluginList();
    void loadPluginsFromList(const QStringList& list);
    
    // Return true if plugin library is loaded in memory.
    // 'libraryName' is internal plugin library name not i18n.
    bool pluginLibraryIsLoaded(const QString& libraryName);

private:

    typedef QPair<QString,Digikam::ImagePlugin*> PluginType;
    typedef QValueList< PluginType >             PluginList;
    
    SplashScreen                         *m_splash;
    static ImagePluginLoader             *m_instance;
    PluginList                            m_pluginList;
    
    Digikam::ImagePlugin* pluginIsLoaded(const QString& name);
};

#endif /* IMAGEPLUGINLOADER_H */
