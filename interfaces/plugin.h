/* ============================================================
 * File  : plugin.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-30
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

#ifndef DIGIKAM_PLUGIN_H
#define DIGIKAM_PLUGIN_H

#include <qobject.h>
#include <qstringlist.h>
#include <kxmlguiclient.h>

namespace Digikam
{

/*! 
  Plugin
 
  Base class for digikam plugins. you need to inherit from this class
  if you plan on writing a plugin. An example is given below:
  \code

  #ifndef PLUGIN_HELLOWORLD_H
  #define PLUGIN_HELLOWORLD_H
  
  #include <digikam/plugin.h>
  
  class Plugin_HelloWorld : public Digikam::Plugin
  {
     Q_OBJECT

   public:

   // Notice the constructor
   // takes three arguments QObject *parent (the parent of this object),
   // const char* name (the name of this object) and
   // const QStringList &args (the arguments passed).
   Plugin_HelloWorld(QObject *parent,
                      const char* name,
                      const QStringList &args);

   private slots:

   //This is an example slot to which your action is connected.
   void slotActivate(); 
                      
  };

  #endif 

  \endcode 

 Now the definition of the class:

 \code

 #include <klocale.h>
 #include <kaction.h>
 #include <kgenericfactory.h>
 #include <klibloader.h>
 #include <kconfig.h>
 #include <kdebug.h>

 #include <digikam/albummanager.h>
 #include <digikam/albuminfo.h>

 #include "plugin_helloworld.h"

 // A macro from KDE KParts to export the symbols for this plugin and
 // create the factory for it. The first argument is the name of the
 // plugin library and the second is the genericfactory templated from
 // the class for your plugin
 K_EXPORT_COMPONENT_FACTORY( digikamplugin_helloworld,
                            KGenericFactory<Plugin_HelloWorld>("digikam"));

 Plugin_HelloWorld::Plugin_HelloWorld(QObject *parent,
                                    const char*,
                                    const QStringList&)
    : Digikam::Plugin(parent, "HelloWorld")
 {
    // Set the instance of this to generic factory instance
    // created using the macro above
    setInstance(KGenericFactory<Plugin_HelloWorld>::instance());

    // Set the xml file which will be merged with the mainwindow's gui
    setXMLFile("plugins/digikamplugin_helloworld.rc");

    // Insert our translations into the global catalogue
    KGlobal::locale()->insertCatalogue("digikamplugin_helloworld");

    kdDebug() << "Plugin_HelloWorld plugin loaded" << endl;

    // this is our action shown in the menubar/toolbar of the mainwindow
    (void) new KAction (i18n("Hello World..."),
                       "misc",
                        0, // or, e.g., CTRL+SHIFT+Key_S,
                        this,
                        SLOT(slotActivate()),
                        actionCollection(),
                        "helloworld");

 }

 void Plugin_HelloWorld::slotActivate()
 {
   kdDebug() << "Plugin_HelloWorld slot activated" << endl;

   // Get the current/selected album
   Digikam::AlbumInfo *album =
        Digikam::AlbumManager::instance()->currentAlbum();

   // Make sure to check that we have a selected album     
   if (!album) return;

   // Now get some properties of the album
   kdDebug() << "The current album title is " << album->getTitle() << endl;
   kdDebug() << "The current album collection is " << album->getCollection() << endl;
   kdDebug() << "The current album date is " << album->getDate().toString() << endl;
   kdDebug() << "The current album path is " << album->getPath() << endl;

   // see the comments in the album

   // First open the album database
   album->openDB();

   // get the comments for this particular item
   kdDebug() << album->getItemComments("IMG_100.JPG") << endl;

   // Close the album database once you are done
   album->closeDB();
   
   // Get all the Albums in the current library path
   for (Digikam::AlbumInfo *a = Digikam::AlbumManager::instance()->firstAlbum();
        a; a = a->nextAlbum()) {
        kdDebug() << "Album title: " << a->getTitle() << endl;
    }
  }

 \endcode

 In addition to the code you need to provide a service file for this
 plugin, \c "digikamplugin_helloworld.desktop"

 \verbatim
 [Desktop Entry]
 Type=Service
 ServiceTypes=Digikam/Plugin
 X-KDE-Library=digikamplugin_helloworld
 Name=HelloWorld
 Comment=Digikam Hello World Plugin
 author=John Doe, johndoe@somewhere.com
 \endverbatim

 And gui xml file for your plugin so that it shows up in the
 mainwindow gui, \c "digikamplugin_helloworld.rc"
 \verbatim
 <!DOCTYPE kpartgui>
<kpartplugin name="helloworld" library="digikamplugin_helloworld" version="1">
<MenuBar>
 <Menu name="Tools"><text>&amp;Tools</text>
    <Action name="helloworld" />
 </Menu>
</MenuBar>
</kpartplugin>
\endverbatim
 
 */
class Plugin : public QObject, public KXMLGUIClient
{
public:

    /*! 
     * Constructor
     */
    Plugin(QObject *parent, const char* name=0);

    /*! 
     * Destructor
     */
    virtual ~Plugin();

    /*! 
     * currently not used
     */
    virtual void enableView();

    /*! 
     * currently not used
     */
    virtual void disableView();

};

}

#endif /* PLUGIN_H */
