/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time: private container
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginloader_p.h"

// Qt includes

#include <QStringList>
#include <QTime>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLibraryInfo>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

DPluginLoader::Private::Private()
    : pluginsLoaded(false)
{
}

DPluginLoader::Private::~Private()
{
}

QStringList DPluginLoader::Private::pluginEntriesList() const
{
    QStringList allFiles;

    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath);
    path.append(QDir::separator() + QLatin1String("digikam") + QDir::separator());

    qCDebug(DIGIKAM_GENERAL_LOG) << "Parsing plugins from" << path;

    foreach (const QString& file, QDir(path).entryList(QDir::Files))
    {
        allFiles << path + file;
    }

    // remove duplicate entries
    allFiles.sort();

    for (int i = 1 ; i < allFiles.size() ; ++i)
    {
        if (allFiles.at(i) == allFiles.at(i-1))
        {
            allFiles.removeAt(i);
            --i;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Plugins found:" << allFiles;

    return allFiles;
}

/** Append obj to the given plugins list.
 */
bool DPluginLoader::Private::appendPlugin(QObject* const obj, QPluginLoader* const loader, QList<DPlugin*>& list)
{
    DPlugin* const plugin = qobject_cast<DPlugin*>(obj);

    if (plugin)
    {
        Q_ASSERT(obj->metaObject()->superClass()); // all our plugins have a super class

        if (plugin->internalVersion() == QLatin1String(digikam_version_short))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Plugin of type" << obj->metaObject()->superClass()->className()
                                         << "loaded from"    << loader->fileName();


            KSharedConfigPtr config = KSharedConfig::openConfig();
            KConfigGroup group      = config->group(DPluginLoader::instance()->configGroupName());
            bool toLoad             = group.readEntry(plugin->id(), false);

            if (toLoad)
            {
                plugin->setup();
                plugin->setLoaded(true);
            }

            list << plugin;
        }

        return true;
    }

    return false;
}

void DPluginLoader::Private::loadPlugins()
{
    if (pluginsLoaded)
    {
        return;
    }

    QTime t;
    t.start();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Starting to load external tools.";

    QStringList toolFileNameList = pluginEntriesList();

    Q_ASSERT(allPlugins.isEmpty());

    bool foundPlugin = false;

    for (const QString& fileName : toolFileNameList)
    {
        QString const baseName = QFileInfo(fileName).baseName();

        if (!whitelist.isEmpty() && !whitelist.contains(baseName))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring non-whitelisted plugin" << fileName;
            continue;
        }

        if (blacklist.contains(baseName))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring blacklisted plugin" << fileName;
            continue;
        }

        // qCDebug(DIGIKAM_GENERAL_LOG) << fileName << " - " << pluginPath(fileName);
        QString const path          = QDir(fileName).canonicalPath();
        QPluginLoader* const loader = new QPluginLoader(path, DPluginLoader::instance());
        QObject* const obj          = loader->instance();

        if (obj)
        {
            bool isPlugin = appendPlugin(obj, loader, allPlugins);

            if (!isPlugin)
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Ignoring the following plugin since it couldn't be loaded:"
                                               << path;

                qCDebug(DIGIKAM_GENERAL_LOG) << "External plugin failure:" << path
                                             << "is a plugin, but it does not implement the"
                                             << "right interface or it was compiled against"
                                             << "an old version of digiKam. Ignoring it.";
                delete loader;
            }
            else
            {
                foundPlugin = true;
            }
        }
        else
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Ignoring to load the following file since it doesn't look like "
                                              "a valid digiKam external plugin:" << path << endl
                                           << "Reason:" << loader->errorString();
            delete loader;
        }
    }

    if (!foundPlugin)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No plugins loaded. Please check if the plugins were installed in the correct path,"
                                       << "or if any errors occurred while loading plugins.";
    }

    pluginsLoaded = true;

    qCDebug(DIGIKAM_GENERAL_LOG) << Q_FUNC_INFO << "Time elapsed:" << t.elapsed() << "ms";
}

} // namepace Digikam
