//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMPLUGINMANAGER.H
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


#ifndef DIGIKAMPLUGINMANAGER_H
#define DIGIKAMPLUGINMANAGER_H

// Qt includes.

#include <qobject.h>
#include <qptrlist.h>

class KAction;

#ifdef HAVE_KIPI                         // libKIPI support.
   namespace KIPI
   {
   class Plugin;
   }
#else                                    // DigikamPlugins support.
   namespace Digikam
   {
   class Plugin;
   }
#endif
   
class DigikamPluginManager : public QObject
{
public:

    DigikamPluginManager(QObject *parent);
    ~DigikamPluginManager();

    void loadPlugins();
    void loadPlugins(QStringList list);
    
    static DigikamPluginManager* instance();
    
    #ifdef HAVE_KIPI                         // libKIPI support.
       const  QPtrList<KIPI::Plugin>& pluginList();
    #else                                    // DigikamPlugins support.   
       const  QPtrList<Digikam::Plugin>& pluginList();
    #endif
    
    const  QPtrList<KAction>&         menuMergeActions();
    
    const  QStringList availablePluginList();
    const  QStringList loadedPluginList();
    
private:

    static DigikamPluginManager* instance_;
    
    #ifdef HAVE_KIPI                         // libKIPI support.
       QPtrList<KIPI::Plugin>    pluginList_;
    #else                                    // DigikamPlugins support.   
       QPtrList<Digikam::Plugin>    pluginList_;
    #endif

    QPtrList<KAction>            menuMergeActions_;
    QStringList                  availablePluginList_;
    
    void initAvailablePluginList();
    
    #ifdef HAVE_KIPI                         // libKIPI support.
       KIPI::Plugin* pluginIsLoaded(QString pluginName);
    #else                                    // DigikamPlugins support.   
       Digikam::Plugin* pluginIsLoaded(QString pluginName);
    #endif
};

#endif // DIGIKAMPLUGINMANAGER_H 
