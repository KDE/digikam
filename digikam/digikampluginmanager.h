//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMPLUGINMANAGER.H
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


#ifndef DIGIKAMPLUGINMANAGER_H
#define DIGIKAMPLUGINMANAGER_H

// Qt includes.

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
    void loadPlugins(QStringList list);
    
    static DigikamPluginManager* instance();
    
    const  QPtrList<Digikam::Plugin>& pluginList();
    
    const  QPtrList<KAction>& menuMergeActions();
    
    const  QStringList availablePluginList();
    const  QStringList loadedPluginList();
    
private:

    static DigikamPluginManager* instance_;
    
    QPtrList<Digikam::Plugin>    pluginList_;

    QPtrList<KAction>            menuMergeActions_;
    QStringList                  availablePluginList_;
    
    void initAvailablePluginList();
    
    Digikam::Plugin* pluginIsLoaded(QString pluginName);
};

#endif // DIGIKAMPLUGINMANAGER_H 
