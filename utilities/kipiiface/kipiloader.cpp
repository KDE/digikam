/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-03
 * Description : kipi Loader Implementation
 *
 * Copyright (C) 2012 by Supreet Pal Singh <supreetpal@gmail.com>
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

//Check if headers are complete

#include "digikamapp.h"

// Qt includes

#include <QSignalMapper>
#include <QStringList>

// KDE includes

#include <kactioncollection.h>
#include <kactionmenu.h>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/plugin.h>

namespace Digikam
{

kipiLoader::kipiLoader()
{
   // Load KIPI Plugins.
    loadPlugins();
}

// Need to pass d-pointer

void kipiLoader::loadPlugins()
{
    d->kipipluginsActionCollection = new KActionCollection(this, KGlobal::mainComponent());

//NEEDS REVIEW: Splashscreen can be called from here?
    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Loading Kipi Plugins"));
    }

    QStringList ignores;
    d->kipiInterface = new KipiInterface( this, "Digikam_KIPI_interface" );

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    // These plugins have been renamed with 0.2.0-rc1
    ignores.append( "Facebook Exporter" );
    ignores.append( "SmugMug Exporter" );
    ignores.append( "SlideShow" );
    ignores.append( "PrintWizard" );
    ignores.append( "SimpleViewer" );
    ignores.append( "KioExport" );

    d->kipiPluginLoader = new KIPI::PluginLoader( ignores, d->kipiInterface );

    connect( d->kipiPluginLoader, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(AlbumManager::instance()->currentAlbum());

    // Setting the initial menu options after all plugins have been loaded
    d->view->slotAlbumSelected(AlbumManager::instance()->currentAlbum());

    d->imagePluginsLoader = new ImagePluginLoader(this, d->splashScreen);
}

void kipiLoader::slotKipiPluginPlug()
{
    unplugActionList(QString::fromLatin1("file_actions_export"));
    unplugActionList(QString::fromLatin1("file_actions_import"));
    unplugActionList(QString::fromLatin1("image_jpeglossless_actions"));
    unplugActionList(QString::fromLatin1("image_print_actions"));
    unplugActionList(QString::fromLatin1("image_metadata_actions"));
    unplugActionList(QString::fromLatin1("image_actions"));
    unplugActionList(QString::fromLatin1("tool_actions"));
    unplugActionList(QString::fromLatin1("batch_actions"));
    unplugActionList(QString::fromLatin1("album_actions"));

    d->kipiImageActions.clear();
    d->kipiFileActionsExport.clear();
    d->kipiFileActionsImport.clear();
    d->kipiToolsActions.clear();
    d->kipiBatchActions.clear();
    d->kipiAlbumActions.clear();
    d->kipiJpeglosslessActions.clear();
    d->kipiPrintActions.clear();
    d->kipiMetadataActions.clear();

    // Remove Advanced slideshow kipi-plugin action from View/Slideshow menu.
    foreach (QAction* action, d->slideShowAction->menu()->actions())
    {
        if (action->objectName() == QString("advancedslideshow"))
        {
            d->slideShowAction->removeAction(action);
            break;
        }
    }

    d->kipipluginsActionCollection->clear();

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    int cpt                             = 0;

    // List of obsolete kipi-plugins to not load.
    QStringList pluginActionsDisabled;
    pluginActionsDisabled << QString("gpssync2");                       // Experimental plugin renamed gpssync during GoSC2010.
    pluginActionsDisabled << QString("raw_converter_single");           // Obsolete since 0.9.5 and new Raw Import tool.
    pluginActionsDisabled << QString("batch_rename_images");            // Obsolete since 1.0.0, replaced by AdvancedRename.
    pluginActionsDisabled << QString("batch_border_images");            // Obsolete since 1.2.0, replaced by BQM border tool.
    pluginActionsDisabled << QString("batch_convert_images");           // Obsolete since 1.2.0, replaced by BQM convert tool.
    pluginActionsDisabled << QString("batch_color_images");             // Obsolete since 1.2.0, replaced by BQM color tool.
    pluginActionsDisabled << QString("batch_filter_images");            // Obsolete since 1.2.0, replaced by BQM enhance tool.
    pluginActionsDisabled << QString("jpeglossless_convert2grayscale"); // Obsolete since 1.7.0, replaced by BQM B&W tool.

    for ( KIPI::PluginLoader::PluginList::ConstIterator it = list.constBegin() ;
          it != list.constEnd() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
        {
            continue;
        }

        ++cpt;

        plugin->setup( this );

        // Add actions to kipipluginsActionCollection
        QList<QAction*> allPluginActions = plugin->actionCollection()->actions();

        if (allPluginActions.count() > 3)
        {
            KActionCategory* category = new KActionCategory(plugin->objectName(), d->kipipluginsActionCollection);
            foreach (QAction* action, allPluginActions)
            {
                QString actionName(action->objectName());

                if (!pluginActionsDisabled.contains(actionName))
                {
                    category->addAction(actionName, action);
                }
            }
        }
        else
        {
            foreach (QAction* action, allPluginActions)
            {
                QString actionName(action->objectName());

                if (!pluginActionsDisabled.contains(actionName))
                {
                    d->kipipluginsActionCollection->addAction(actionName, action);
                }
            }
        }

        // Plugin category identification using KAction method based.

        QList<KAction*> actions = plugin->actions();
        foreach (KAction* action, actions)
        {
            QString actionName(action->objectName());

            if (!pluginActionsDisabled.contains(actionName))
            {
                switch (plugin->category(action))
                {
                    case KIPI::BatchPlugin:
                    {
                        d->kipiBatchActions.append(action);
                        break;
                    }
                    case KIPI::CollectionsPlugin:
                    {
                        d->kipiAlbumActions.append(action);
                        break;
                    }
                    case KIPI::ImportPlugin:
                    {
                        d->kipiFileActionsImport.append(action);
                        break;
                    }
                    case KIPI::ExportPlugin:
                    {
                        d->kipiFileActionsExport.append(action);
                        break;
                    }
                    case KIPI::ImagesPlugin:
                    {
                        if (plugin->objectName() == "JPEGLossless")
                        {
                            d->kipiJpeglosslessActions.append(action);
                        }
                        else if (plugin->objectName() == "PrintImages")
                        {
                            d->kipiPrintActions.append(action);
                        }
                        else if (plugin->objectName() == "GPSSync" ||
                                 plugin->objectName() == "MetadataEdit" ||
                                 plugin->objectName() == "TimeAdjust")
                        {
                            d->kipiMetadataActions.append(action);
                        }
                        else
                        {
                            d->kipiImageActions.append(action);
                        }

                        break;
                    }
                    case KIPI::ToolsPlugin:
                    {
                        if (actionName == QString("advancedslideshow"))
                        {
                            // Add Advanced slideshow kipi-plugin action to View/Slideshow menu.
                            d->slideShowAction->addAction(action);
                        }
                        else
                        {
                            d->kipiToolsActions.append(action);
                        }

                        break;
                    }
                    default:
                    {
                        kDebug() << "No menu found for a plugin!";
                        break;
                    }
                }
            }
            else
            {
                kDebug() << "Plugin '" << actionName << "' disabled.";
            }
        }
    }

    // load KIPI actions settings
    d->kipipluginsActionCollection->readSettings();

    // Create GUI menu in according with plugins.
    plugActionList(QString::fromLatin1("file_actions_export"),        d->kipiFileActionsExport);
    plugActionList(QString::fromLatin1("file_actions_import"),        d->kipiFileActionsImport);
    plugActionList(QString::fromLatin1("image_jpeglossless_actions"), d->kipiJpeglosslessActions);
    plugActionList(QString::fromLatin1("image_print_actions"),        d->kipiPrintActions);
    plugActionList(QString::fromLatin1("image_metadata_actions"),     d->kipiMetadataActions);
    plugActionList(QString::fromLatin1("image_actions"),              d->kipiImageActions);
    plugActionList(QString::fromLatin1("tool_actions"),               d->kipiToolsActions);
    plugActionList(QString::fromLatin1("batch_actions"),              d->kipiBatchActions);
    plugActionList(QString::fromLatin1("album_actions"),              d->kipiAlbumActions);
}

} //namespace Digikam
