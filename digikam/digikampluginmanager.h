/* ============================================================
 * File  : digikampluginmanager.h
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

#ifndef DIGIKAMPLUGINMANAGER_H
#define DIGIKAMPLUGINMANAGER_H

#include <qobject.h>
#include <qptrlist.h>

class KAction;

namespace Digikam
{
class Plugin;
}

class DigikamPluginManager : public QObject
{
public:

    DigikamPluginManager(QObject *parent);
    ~DigikamPluginManager();

    void loadPlugins();

    static DigikamPluginManager* instance();
    const  QPtrList<Digikam::Plugin>& pluginList();
    const  QPtrList<KAction>&         menuMergeActions();
    
private:

    static DigikamPluginManager* instance_;
    QPtrList<Digikam::Plugin>    pluginList_;
    QPtrList<KAction>            menuMergeActions_;
    
};

#endif /* DIGIKAMPLUGINMANAGER_H */
