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

#include <QStringList>
#include <QAction>
#include <QMap>

// KDE includes

#include <kactioncategory.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kactionmenu.h>
#include <kdebug.h>

// LibKIPI includes

#include <libkipi/pluginloader.h>

// local includes

#include "kipiinterface.h"
#include "digikamapp.h"
#include "splashscreen.h"

namespace Digikam
{

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

    static const QString        imagesActionName;
    static const QString        toolsActionName;
    static const QString        importActionName;
    static const QString        exportActionName;
    static const QString        batchActionName;
    static const QString        albumsActionName;

    KIPI::PluginLoader*         kipiPluginLoader;
    KipiInterface*              kipiInterface;
    SplashScreen*               splashScreen;
    DigikamApp*                 app;

    KActionCollection*          kipipluginsActionCollection; // Collection used to host all plugin actions for KDE shortcuts editor
    QMap<int, KActionCategory*> kipiCategoryMap;             // KActionCategory map shorted by KIPI::Category
};

// -- Static values -------------------------------

const QString KipiPluginLoader::KipiPluginLoaderPriv::imagesActionName(QString::fromLatin1("image_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::toolsActionName(QString::fromLatin1("tool_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::importActionName(QString::fromLatin1("import_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::exportActionName(QString::fromLatin1("export_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::batchActionName(QString::fromLatin1("batch_kipi_actions"));
const QString KipiPluginLoader::KipiPluginLoaderPriv::albumsActionName(QString::fromLatin1("album_kipi_actions"));

KipiPluginLoader* KipiPluginLoader::m_instance = 0;

// -----------------------------------------------------------------------------------------------------------------------

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

QList<QAction*> KipiPluginLoader::kipiActionsByCategory(KIPI::Category cat) const
{
    KActionCategory* category = d->kipiCategoryMap[cat];
    if (category)
        return category->actions();

    return QList<QAction*>();
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

    // List of obsoletes plugins to not load

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

    d->kipiCategoryMap.clear();

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

    // List of obsolete tool actions to not load

    QStringList pluginActionsDisabled;
    pluginActionsDisabled << QString("gpssync2");                       // Experimental plugin renamed gpssync during GoSC2010.
    pluginActionsDisabled << QString("raw_converter_single");           // Obsolete since 0.9.5 and new Raw Import tool.
    pluginActionsDisabled << QString("batch_rename_images");            // Obsolete since 1.0.0, replaced by AdvancedRename.
    pluginActionsDisabled << QString("batch_border_images");            // Obsolete since 1.2.0, replaced by BQM border tool.
    pluginActionsDisabled << QString("batch_convert_images");           // Obsolete since 1.2.0, replaced by BQM convert tool.
    pluginActionsDisabled << QString("batch_color_images");             // Obsolete since 1.2.0, replaced by BQM color tool.
    pluginActionsDisabled << QString("batch_filter_images");            // Obsolete since 1.2.0, replaced by BQM enhance tool.

    for (KIPI::PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
        {
            continue;
        }

        ++cpt;
        plugin->setup(d->app);

        foreach(KAction* const action, plugin->actions())
        {
            QString actionName(action->objectName());
            KIPI::Category cat = plugin->category(action);

            if (!pluginActionsDisabled.contains(actionName))
            {
                KActionCategory* category = d->kipiCategoryMap[cat];
                if (!category)
                {
                    category = new KActionCategory(categoryName(cat), d->kipipluginsActionCollection);
                    d->kipiCategoryMap.insert(cat, category);
                }

                if (cat == KIPI::ToolsPlugin && actionName == QString("advancedslideshow"))
                {
                    // Special wrap for Advanced Slideshow plugin action which need to be pluged to View/Slideshow menu.
                    d->app->slideShowMenu()->addAction(action);
                }

                category->addAction(actionName, qobject_cast<QAction*>(action));
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
    checkEmptyCategory(KIPI::ExportPlugin);
    checkEmptyCategory(KIPI::ImportPlugin);
    checkEmptyCategory(KIPI::ToolsPlugin);

    // Create plugin GUI menus in application.
    kipiPlugActions();
}

void KipiPluginLoader::checkEmptyCategory(KIPI::Category cat)
{
    KActionCategory* category = d->kipiCategoryMap[cat];
    if (!category)
    {
        QAction* action = new QAction(i18n("No tool available"), d->app);
        action->setEnabled(false);
        category        = new KActionCategory(categoryName(cat), d->kipipluginsActionCollection);
        d->kipiCategoryMap.insert(cat, category);
    }
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
        d->app->unplugActionList(d->albumsActionName);
    }
    else
    {
        d->app->plugActionList(d->exportActionName, kipiActionsByCategory(KIPI::ExportPlugin));
        d->app->plugActionList(d->importActionName, kipiActionsByCategory(KIPI::ImportPlugin));
        d->app->plugActionList(d->imagesActionName, kipiActionsByCategory(KIPI::ImagesPlugin));
        d->app->plugActionList(d->toolsActionName,  kipiActionsByCategory(KIPI::ToolsPlugin));
        d->app->plugActionList(d->batchActionName,  kipiActionsByCategory(KIPI::BatchPlugin));
        d->app->plugActionList(d->albumsActionName, kipiActionsByCategory(KIPI::CollectionsPlugin));
    }
}

QString KipiPluginLoader::categoryName(KIPI::Category cat) const
{
    switch (cat)
    {
        case KIPI::ExportPlugin:
            return i18n("Export Tools");
            break;
        case KIPI::ImportPlugin:
            return i18n("Import Tools");
            break;
        case KIPI::ImagesPlugin:
            return i18n("Images Tools");
            break;
        case KIPI::ToolsPlugin:
            return i18n("Miscs Tools");
            break;
        case KIPI::BatchPlugin:
            return i18n("Batch Tools");
            break;
        case KIPI::CollectionsPlugin:
            return i18n("Albums Tools");
            break;
        default:
            return i18n("Unknown Tools");
            break;
    }
}

} //namespace Digikam
