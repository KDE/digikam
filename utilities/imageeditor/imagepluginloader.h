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

// Local includes.

#include "imageplugin.h"

class ImagePluginLoader : public QObject
{
    
public:

    ImagePluginLoader(QObject *parent);
    ~ImagePluginLoader();

    static ImagePluginLoader* instance();

    QPtrList<Digikam::ImagePlugin>& pluginList();
    void loadPluginsFromList(QStringList list);
    
private:

    static ImagePluginLoader*      m_instance;
    QPtrList<Digikam::ImagePlugin> m_pluginList;
    
    Digikam::ImagePlugin* pluginIsLoaded(QString pluginName);
};

#endif /* IMAGEPLUGINLOADER_H */
