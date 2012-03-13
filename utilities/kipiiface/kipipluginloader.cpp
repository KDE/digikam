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
#include <QMultiMap>

// KDE includes

#include <kactioncategory.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kactionmenu.h>
#include <kdebug.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/interface.h>
#include <libkipi/pluginloader.h>

// local includes

#include "kipiinterface.h"
#include "digikamapp.h"
#include "splashscreen.h"

namespace Digikam
{

KipiPluginLoader* KipiPluginLoader::m_instance = 0;

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

    static const QString     imagesActionName;
    static const QString     toolsActionName;
    static const QString     importActionName;
    static const QString     exportActionName;
    static const QString     batchActionName;
    static const QString     collectionsActionName;

    KActionCollection*       kipipluginsActionCollection;
    KIPI::PluginLoader*      kipiPluginLoader;
    KipiInterface*           kipiInterface;
    SplashScreen*            splashScreen;
    DigikamApp*              app;

    QMultiMap<int, QAction*> kipiActionsMap;   // Map <KIPI::Category, KIPI plugin action>
};
const QString KipiPluginLoader::KipiPluginLoaderPriv::imagesActionName(QString::fromLatin1("image_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::toolsActionName(QString::fromLatin1("tool_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::importActionName(QString::fromLatin1("import_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::exportActionName(QString::fromLatin1("export_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::batchActionName(QString::fromLatin1("batch_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::collectionsActionName(QString::fromLatin1("album_kipi_actions"));

KipiPluginLoader::KipiPluginLoader(QObject* const parent, SplashScreen* const splash)
    : QObject(parent), d(new KipiPluginLoaderPriv)
{
    m_instance      = this;
    d->splashScreen = splash;
    loadPlugins();
}

KipiPluginLoader::~KipiPluginLoader()
{
    delete d;
    m_instance = 0;
}

KipiPluginLoader* KipiPluginLoader::instance()
{
    return m_instance;
}

KActionCollection* KipiPluginLoader::pluginsActionCollection() const
{
    return d->kipipluginsActionCollection;
}

QList<QAction*> KipiPluginLoader::kipiActionsByCategory(KIPI::Category category) const
{
    return d->kipiActionsMap.values(category);
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
    // Remove plugin GUI menus in application.
    kipiPlugActions(true);

    d->kipiActionsMap.clear();

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
                int cat = plugin->category(action);

                if (cat == KIPI::ToolsPlugin && actionName == QString("advancedslideshow"))
                {
                    // Wrap Advanced Slideshow kipi-plugin action to View/Slideshow menu.
                    d->app->slideShowMenu()->addAction(action);
                }
                else
                {
                    d->kipiActionsMap.insert(cat, qobject_cast<QAction*>(action));
                }
            }
            else
            {
                kDebug() << "Plugin '" << actionName << "' is disabled.";
            }
        }
    }

    // load KIPI actions settings
    d->kipipluginsActionCollection->readSettings();

    // Check if the Export/Import/tools Plugin lists are empty, if so, add an empty action which tells the user that no 
    // Export/Import/tools plugins are available. It is more user-friendly to present some menu entry, 
    // instead of leaving it completely empty.

    if (kipiActionsByCategory(KIPI::ExportPlugin).empty())
    {
        QAction* action = new QAction(i18n("No tool available"), d->app);
        action->setEnabled(false);
        d->kipiActionsMap.insert(KIPI::ExportPlugin, action);
    }

    if (kipiActionsByCategory(KIPI::ImportPlugin).empty())
    {
        QAction* action = new QAction(i18n("No tool available"), d->app);
        action->setEnabled(false);
        d->kipiActionsMap.insert(KIPI::ImportPlugin, action);
    }

    if (kipiActionsByCategory(KIPI::ToolsPlugin).empty())
    {
        QAction* action = new QAction(i18n("No tool available"), d->app);
        action->setEnabled(false);
        d->kipiActionsMap.insert(KIPI::ToolsPlugin, action);
    }

    // Create plugin GUI menus in application.
    kipiPlugActions();
}

void KipiPluginLoader::kipiPlugActions(bool unplug)
{
    if (unplug)
    {
        d->app->unplugActionList(d->exportActionName);
        d->app->unplugActionList(d->importActionName);
        d->app->unplugActionList(d->imagesActionName);
        d->app->unplugActionList(d->toolsActionName);
        d->app->unplugActionList(d->batchActionName);
        d->app->unplugActionList(d->collectionsActionName);
    }
    else
    {
        d->app->plugActionList(d->exportActionName,      kipiActionsByCategory(KIPI::ExportPlugin));
        d->app->plugActionList(d->importActionName,      kipiActionsByCategory(KIPI::ImportPlugin));
        d->app->plugActionList(d->imagesActionName,      kipiActionsByCategory(KIPI::ImagesPlugin));
        d->app->plugActionList(d->toolsActionName,       kipiActionsByCategory(KIPI::ToolsPlugin));
        d->app->plugActionList(d->batchActionName,       kipiActionsByCategory(KIPI::BatchPlugin));
        d->app->plugActionList(d->collectionsActionName, kipiActionsByCategory(KIPI::CollectionsPlugin));
    }
}

} //namespace Digikam
