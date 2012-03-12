/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-03
 * Description : kipi Loader Implementation
 *
 * Copyright (C) 2012      by Supreet Pal Singh <supreetpal@gmail.com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipipluginloader.moc"

// Qt includes

#include <QSignalMapper>
#include <QStringList>
#include <QAction>

// KDE includes

#include <kactioncategory.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kactionmenu.h>
#include <kdebug.h>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// local includes

#include "kipiinterface.h"
#include "digikamapp.h"
#include "splashscreen.h"

namespace Digikam
{

class KipiPluginLoaderCreator
{
public:

    KipiPluginLoader object;
};

K_GLOBAL_STATIC(KipiPluginLoaderCreator, kipiPluginLoaderCreator)

// -----------------------------------------------------------------------------------------------

class KipiPluginLoader::KipiPluginLoaderPriv
{
public:

    KipiPluginLoaderPriv()
    {
        app                         = DigikamApp::instance();
        kipipluginsActionCollection = 0;
        kipiPluginLoader            = 0;
        kipiInterface               = 0;
        splashScreen                = 0;
    }

    KActionCollection*  kipipluginsActionCollection;
    KIPI::PluginLoader* kipiPluginLoader;
    KipiInterface*      kipiInterface;
    SplashScreen*       splashScreen;
    DigikamApp*         app;

    QList<QAction*>     kipiFileActionsExport;
    QList<QAction*>     kipiFileActionsImport;
    QList<QAction*>     kipiPrintActions;
    QList<QAction*>     kipiMetadataActions;
    QList<QAction*>     kipiImageActions;
    QList<QAction*>     kipiToolsActions;
    QList<QAction*>     kipiBatchActions;
    QList<QAction*>     kipiAlbumActions;
};

KipiPluginLoader::KipiPluginLoader()
    : QObject(DigikamApp::instance()), d(new KipiPluginLoaderPriv)
{
}

KipiPluginLoader::~KipiPluginLoader()
{
    delete d;
}

KipiPluginLoader* KipiPluginLoader::instance()
{
    return &kipiPluginLoaderCreator->object;
}

KActionCollection* KipiPluginLoader::pluginsActionCollection() const
{
    return d->kipipluginsActionCollection;
}

const QList<QAction*>& KipiPluginLoader::menuExportActions()
{
    return d->kipiFileActionsExport;
}

const QList<QAction*>& KipiPluginLoader::menuImportActions()
{
    return d->kipiFileActionsImport;
}

const QList<QAction*>& KipiPluginLoader::menuMetadataActions()
{
    return d->kipiMetadataActions;
}

const QList<QAction*>& KipiPluginLoader::menuPrintActions()
{
    return d->kipiPrintActions;
}

const QList<QAction*>& KipiPluginLoader::menuImageActions()
{
    return d->kipiImageActions;
}

const QList<QAction*>& KipiPluginLoader::menuToolsActions()
{
    return d->kipiToolsActions;
}

const QList<QAction*>& KipiPluginLoader::menuBatchActions()
{
    return d->kipiBatchActions;
}

const QList<QAction*>& KipiPluginLoader::menuAlbumActions()
{
    return d->kipiAlbumActions;
}

void KipiPluginLoader::setSplashScreen(SplashScreen* const splash)
{
    d->splashScreen = splash;
}

void KipiPluginLoader::loadPlugins()
{
    d->kipipluginsActionCollection = new KActionCollection(d->app, KGlobal::mainComponent());

    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Loading KIPI Plugins..."));
    }

    QStringList ignores;
    d->kipiInterface = new KipiInterface(d->app, "Digikam_KIPI_interface");

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    // These plugins have been renamed with 0.2.0-rc1
    ignores.append( "Facebook Exporter" );
    ignores.append( "SmugMug Exporter" );
    ignores.append( "SlideShow" );
    ignores.append( "PrintWizard" );
    ignores.append( "SimpleViewer" );
    ignores.append( "KioExport" );

    // These plugins have been replaced by digiKam core solution with 2.6.0
    ignores.append( "JPEGLossless" );

    d->kipiPluginLoader = new KIPI::PluginLoader(ignores, d->kipiInterface);

    connect( d->kipiPluginLoader, SIGNAL(replug()),
             this, SLOT(slotKipiPluginPlug()) );

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(AlbumManager::instance()->currentAlbum());
}

void KipiPluginLoader::slotKipiPluginPlug()
{
    d->app->unplugActionList(QString::fromLatin1("file_actions_export"));
    d->app->unplugActionList(QString::fromLatin1("file_actions_import"));
    d->app->unplugActionList(QString::fromLatin1("image_print_actions"));
    d->app->unplugActionList(QString::fromLatin1("image_metadata_actions"));
    d->app->unplugActionList(QString::fromLatin1("image_actions"));
    d->app->unplugActionList(QString::fromLatin1("tool_actions"));
    d->app->unplugActionList(QString::fromLatin1("batch_actions"));
    d->app->unplugActionList(QString::fromLatin1("album_actions"));

    d->kipiImageActions.clear();
    d->kipiFileActionsExport.clear();
    d->kipiFileActionsImport.clear();
    d->kipiToolsActions.clear();
    d->kipiBatchActions.clear();
    d->kipiAlbumActions.clear();
    d->kipiPrintActions.clear();
    d->kipiMetadataActions.clear();

    // Remove Advanced slideshow kipi-plugin action from View/Slideshow menu.
    foreach(QAction* const action, d->app->slideShowMenu()->menu()->actions())
    {
        if (action->objectName() == QString("advancedslideshow"))
        {
            d->app->slideShowMenu()->removeAction(action);
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

    for ( KIPI::PluginLoader::PluginList::ConstIterator it = list.constBegin() ;
          it != list.constEnd() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
        {
            continue;
        }

        ++cpt;

        plugin->setup(d->app);

        // Add actions to kipipluginsActionCollection
        QList<QAction*> allPluginActions = plugin->actionCollection()->actions();

        if (allPluginActions.count() > 3)
        {
            KActionCategory* category = new KActionCategory(plugin->objectName(), d->kipipluginsActionCollection);

            foreach(QAction* const action, allPluginActions)
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
            foreach(QAction* const action, allPluginActions)
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

        foreach(KAction* const action, actions)
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
                        if (plugin->objectName() == "PrintImages")
                        {
                            d->kipiPrintActions.append(action);
                        }
                        else if (plugin->objectName() == "GPSSync"      ||
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
                             d->app->slideShowMenu()->addAction(action);
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

    // Check if the kipiFileActionsExport are empty, if so, add an empty action which tells the user that no export plugins are
    // available. It is more user-friendly to present some menu entry, instead of leaving it completely empty.
    if (d->kipiFileActionsExport.empty())
    {
        QAction* noPluginsLoaded = new QAction(i18n("No export plugins available"), d->app);
        noPluginsLoaded->setEnabled(false);
        d->kipiFileActionsExport << noPluginsLoaded;
    }

    // Create GUI menu in according with plugins.
    d->app->plugActionList(QString::fromLatin1("file_actions_export"),    d->kipiFileActionsExport);
    d->app->plugActionList(QString::fromLatin1("file_actions_import"),    d->kipiFileActionsImport);
    d->app->plugActionList(QString::fromLatin1("image_print_actions"),    d->kipiPrintActions);
    d->app->plugActionList(QString::fromLatin1("image_metadata_actions"), d->kipiMetadataActions);
    d->app->plugActionList(QString::fromLatin1("image_actions"),          d->kipiImageActions);
    d->app->plugActionList(QString::fromLatin1("tool_actions"),           d->kipiToolsActions);
    d->app->plugActionList(QString::fromLatin1("batch_actions"),          d->kipiBatchActions);
    d->app->plugActionList(QString::fromLatin1("album_actions"),          d->kipiAlbumActions);
}

} //namespace Digikam
