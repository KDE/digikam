/* ============================================================
 * File  : imagepluginloader.h
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

#ifndef IMAGEPLUGINLOADER_H
#define IMAGEPLUGINLOADER_H

#include <qobject.h>
#include <qptrlist.h>

#include "imageplugin.h"

class ImagePluginLoader : public QObject
{
    
public:

    ImagePluginLoader(QObject *parent);
    ~ImagePluginLoader();

    static ImagePluginLoader* instance();

    QPtrList<Digikam::ImagePlugin>& pluginList();
    
private:

    static ImagePluginLoader*      m_instance;
    QPtrList<Digikam::ImagePlugin> m_pluginList;
};

#endif /* IMAGEPLUGINLOADER_H */
