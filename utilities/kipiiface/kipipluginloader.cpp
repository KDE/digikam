/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-03
 * Description : kipi Loader Implementation
 *
 * Copyright (C) 2004-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
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

// local includes

#include "kipiinterface.h"
#include "digikamapp.h"
#include "splashscreen.h"

namespace Digikam
{

class KipiPluginLoader::Private
{
public:

    explicit Private(KipiPluginLoader* const loader)
    {
        app                         = DigikamApp::instance();
        kipipluginsActionCollection = 0;
        kipiPluginLoader            = 0;
        kipiInterface               = 0;
        splashScreen                = 0;
        parent                      = loader;
    }

    /** Load all enabled plugins in application accordingly with KIPI setup page.
     */
    void loadPlugins();

    /** Wrapper to return plugin category name for KDE Shortcuts Editor.
     */
    QString categoryName(Category cat) const;

public:

    KipiPluginLoader*           parent;
    PluginLoader*               kipiPluginLoader;
    KipiInterface*              kipiInterface;
    SplashScreen*               splashScreen;
    DigikamApp*                 app;

    KActionCollection*          kipipluginsActionCollection; // Collection used to host all plugin actions for KDE shortcuts editor
    QMap<int, KActionCategory*> kipiCategoryMap;             // KActionCategory map shorted by Category
};

void KipiPluginLoader::Private::loadPlugins()
{
    kipipluginsActionCollection = new KActionCollection(app, KGlobal::mainComponent());

    if (splashScreen)
    {
        splashScreen->message(i18n("Loading KIPI Plugins..."));
    }

    QStringList ignores;
    kipiInterface = new KipiInterface(app, "Digikam_KIPI_interface");

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

    // Test plugin introduced with libkipi 2.0.0
    ignores.append("KXMLHelloWorld");

    // Raw converter single dialog is obsolete since 0.9.5 with new Raw Import tool for Image Editor.
    // Raw converter batch dialog is obsolete since 3.0.0 with new Raw decoding settings include in Batch Queue Manager.
    ignores.append("RawConverter");

    // Disable loading of old PicasaWeb and Google Drive factored in GoogleServices.
    ignores.append("Picasaweb Exporter");
    ignores.append("Google Drive Export");

    // List of obsolete tool actions to not load
    QStringList pluginActionsDisabled;
    pluginActionsDisabled << QString("gpssync2");                       // Experimental plugin renamed gpssync during GoSC2010.
    pluginActionsDisabled << QString("batch_rename_images");            // Obsolete since 1.0.0, replaced by AdvancedRename.
    pluginActionsDisabled << QString("batch_border_images");            // Obsolete since 1.2.0, replaced by BQM border tool.
    pluginActionsDisabled << QString("batch_convert_images");           // Obsolete since 1.2.0, replaced by BQM convert tool.
    pluginActionsDisabled << QString("batch_color_images");             // Obsolete since 1.2.0, replaced by BQM color tool.
    pluginActionsDisabled << QString("batch_filter_images");            // Obsolete since 1.2.0, replaced by BQM enhance tool.
    pluginActionsDisabled << QString("batch_recompress_images");        // Obsolete since 3.0.0, replaced by BQM enhance tool.

    kipiPluginLoader = new PluginLoader(app);
    kipiPluginLoader->setInterface(kipiInterface);
    kipiPluginLoader->setIgnoredPluginsList(ignores);
    kipiPluginLoader->setDisabledPluginActions(pluginActionsDisabled);
    kipiPluginLoader->init();

    parent->connect(kipiPluginLoader, SIGNAL(replug()),
                    parent, SLOT(slotKipiPluginPlug()));

    kipiPluginLoader->loadPlugins();

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if(!albumList.isEmpty())
    {
        kipiInterface->slotCurrentAlbumChanged(albumList);
    }
    else
    {
        kipiInterface->slotCurrentAlbumChanged(QList<Album*>());
    }

    parent->connect(AlbumManager::instance(), SIGNAL(signalAlbumCurrentChanged(QList<Album*>)),
                    kipiInterface, SLOT(slotCurrentAlbumChanged(QList<Album*>)));
}

QString KipiPluginLoader::Private::categoryName(Category cat) const
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

// -- Static values ------------------------------------------------------------------------------------------------------

KipiPluginLoader* KipiPluginLoader::m_instance = 0;

// -----------------------------------------------------------------------------------------------------------------------

KipiPluginLoader::KipiPluginLoader(QObject* const parent, SplashScreen* const splash)
    : QObject(parent), d(new Private(this))
{
    m_instance      = this;
    d->splashScreen = splash;

    d->loadPlugins();
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
    KActionCategory* const category = d->kipiCategoryMap[cat];

    if (category)
    {
        return category->actions();
    }

    return QList<QAction*>();
}

void KipiPluginLoader::slotKipiPluginPlug()
{
    // Ugly hack. Remove "advancedslideshow" action from Slideshow menu
    foreach(QAction* const action, d->app->slideShowMenu()->menu()->actions())
    {
        if (action->objectName() == "advancedslideshow")
            d->app->slideShowMenu()->removeAction(action);
    }

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

    // First we remove all plugins from the gui
    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* const plugin = (*it)->plugin();

        if (!plugin || !dynamic_cast<KXMLGUIClient*>(plugin) || !(*it)->shouldLoad())
        {
            continue;
        }

        d->app->guiFactory()->removeClient(plugin);
    }

    QStringList pluginActionsDisabled = d->kipiPluginLoader->disabledPluginActions();

    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* const plugin = (*it)->plugin();

        if (!plugin || !dynamic_cast<KXMLGUIClient*>(plugin) || !(*it)->shouldLoad())
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

            if (cat == InvalidCategory)
            {
                kWarning() << "Plugin action '" << actionName << "' has invalid category!";
                continue;
            }

            if (!pluginActionsDisabled.contains(actionName))
            {
                KActionCategory* category = d->kipiCategoryMap[cat];

                if (!category)
                {
                    category = new KActionCategory(d->categoryName(cat), d->kipipluginsActionCollection);
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

    // We add them back
    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* const plugin = (*it)->plugin();

        if (!plugin || !dynamic_cast<KXMLGUIClient*>(plugin) || !(*it)->shouldLoad())
        {
            continue;
        }

        // Ugly hack. Remove "advancedslideshow action from AdvancedSlideshow plugin
        // actionCollection() and add it to the Slideshow menu
        if (plugin->objectName() == QString("AdvancedSlideshow"))
        {
            QAction* const action = plugin->actionCollection()->action("advancedslideshow");
            if (action)
            {
                QAction* const _action = plugin->actionCollection()->takeAction(action);
                d->app->slideShowMenu()->addAction(_action);
            }
        }
        d->app->guiFactory()->addClient(plugin);
    }

    // load KIPI actions settings
    d->kipipluginsActionCollection->readSettings();
}

} // namespace Digikam
