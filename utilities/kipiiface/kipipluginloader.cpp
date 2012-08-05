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
 * Copyright (C) 2012      by Victor Dodon <dodonvictor at gmail dot com>
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
#include <kxmlguifactory.h>
#include <kxmlguiwindow.h>

// LibKIPI includes

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

// local includes

#include "kipiinterface.h"
#include "digikamapp.h"
#include "splashscreen.h"

namespace Digikam
{

class KipiPluginLoader::Private
{
public:

    Private()
    {
        app                         = DigikamApp::instance();
        kipipluginsActionCollection = 0;
        kipiPluginLoader            = 0;
        kipiInterface               = 0;
        splashScreen                = 0;
    }

    PluginLoader*               kipiPluginLoader;
    KipiInterface*              kipiInterface;
    SplashScreen*               splashScreen;
    DigikamApp*                 app;

    KActionCollection*          kipipluginsActionCollection; // Collection used to host all plugin actions for KDE shortcuts editor
    QMap<int, KActionCategory*> kipiCategoryMap;             // KActionCategory map shorted by Category
};

// -- Static values -------------------------------

KipiPluginLoader* KipiPluginLoader::m_instance = 0;

// -----------------------------------------------------------------------------------------------------------------------

KipiPluginLoader::KipiPluginLoader(QObject* const parent, SplashScreen* const splash)
    : QObject(parent), d(new Private)
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

QList<QAction*> KipiPluginLoader::kipiActionsByCategory(Category cat) const
{
    KActionCategory* category = d->kipiCategoryMap[cat];

    if (category)
    {
        return category->actions();
    }

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

    ignores.append("HelloWorld");
    ignores.append("KameraKlient");

    // These plugins have been renamed with 0.2.0-rc1
    ignores.append("Facebook Exporter");
    ignores.append("SmugMug Exporter");
    ignores.append("SlideShow");
    ignores.append("PrintWizard");
    ignores.append("SimpleViewer");
    ignores.append("KioExport");

    // These plugins have been replaced by digiKam core solution with 2.6.0
    ignores.append("JPEGLossless");

#if KIPI_VERSION >= 0x020000
    d->kipiPluginLoader = new PluginLoader(d->app);
    d->kipiPluginLoader->setInterface(d->kipiInterface);
    d->kipiPluginLoader->setIgnoreList(ignores);
    d->kipiPluginLoader->init();
#else
    d->kipiPluginLoader = new PluginLoader(ignores, d->kipiInterface);
#endif

    connect(d->kipiPluginLoader, SIGNAL(replug()),
            this, SLOT(slotKipiPluginPlug()));

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(AlbumManager::instance()->currentAlbum());

    connect(AlbumManager::instance(), SIGNAL(signalAlbumCurrentChanged(Album*)),
            d->kipiInterface, SLOT(slotCurrentAlbumChanged(Album*)));
}

void KipiPluginLoader::slotKipiPluginPlug()
{
    // Delete all action categories
    for (QMap<int, KActionCategory*>::iterator it = d->kipiCategoryMap.begin();
         it != d->kipiCategoryMap.end();
         ++it)
    {
        if (it.value())
        {
            delete it.value();
        }
    }

    d->kipipluginsActionCollection->clear();
    d->kipiCategoryMap.clear();

    PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    int cpt                       = 0;

    // List of obsolete tool actions to not load

    QStringList pluginActionsDisabled;
    pluginActionsDisabled << QString("gpssync2");                       // Experimental plugin renamed gpssync during GoSC2010.
    pluginActionsDisabled << QString("raw_converter_single");           // Obsolete since 0.9.5 and new Raw Import tool.
    pluginActionsDisabled << QString("batch_rename_images");            // Obsolete since 1.0.0, replaced by AdvancedRename.
    pluginActionsDisabled << QString("batch_border_images");            // Obsolete since 1.2.0, replaced by BQM border tool.
    pluginActionsDisabled << QString("batch_convert_images");           // Obsolete since 1.2.0, replaced by BQM convert tool.
    pluginActionsDisabled << QString("batch_color_images");             // Obsolete since 1.2.0, replaced by BQM color tool.
    pluginActionsDisabled << QString("batch_filter_images");            // Obsolete since 1.2.0, replaced by BQM enhance tool.

    // First we remove all plugins from the gui
    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* plugin = (*it)->plugin();

        if (!plugin || !(*it)->shouldLoad() || !dynamic_cast<KXMLGUIClient*>(plugin))
        {
            continue;
        }
        d->app->guiFactory()->removeClient(plugin);
    }

    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* plugin = (*it)->plugin();

        if (!plugin || !(*it)->shouldLoad())
        {
            continue;
        }

        ++cpt;
        plugin->setup(d->app);
        plugin->rebuild();

        foreach(KAction* const action, plugin->actions())
        {
            QString actionName(action->objectName());
            Category cat = plugin->category(action);

            if (!pluginActionsDisabled.contains(actionName))
            {
                KActionCategory* category = d->kipiCategoryMap[cat];

                if (!category)
                {
                    category = new KActionCategory(categoryName(cat), d->kipipluginsActionCollection);
                    d->kipiCategoryMap.insert(cat, category);
                }

                category->addAction(actionName, qobject_cast<QAction*>(action));
            }
            else
            {
                kDebug() << "Plugin '" << actionName << "' is disabled.";
            }
        }
    }

//    d->app->rebuild();

    // We add them back
    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* plugin = (*it)->plugin();

        if (!plugin || !(*it)->shouldLoad() || !dynamic_cast<KXMLGUIClient*>(plugin))
        {
            continue;
        }
        d->app->guiFactory()->addClient(plugin);
    }

    // load KIPI actions settings
    d->kipipluginsActionCollection->readSettings();
}

QString KipiPluginLoader::categoryName(Category cat) const
{
    QString res;

    switch (cat)
    {
        case ExportPlugin:
            res = i18n("Export Tools");
            break;

        case ImportPlugin:
            res = i18n("Import Tools");
            break;

        case ImagesPlugin:
            res = i18n("Images Tools");
            break;

        case ToolsPlugin:
            res = i18n("Miscellaneous Tools");
            break;

        case BatchPlugin:
            res = i18n("Batch Tools");
            break;

        case CollectionsPlugin:
            res = i18n("Albums Tools");
            break;

        default:
            res = i18n("Unknown Tools");
            break;
    }

    return res;
}

} // namespace Digikam
