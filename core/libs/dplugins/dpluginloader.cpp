/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time
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

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginLoaderCreator
{
public:

    DPluginLoader object;
};

Q_GLOBAL_STATIC(DPluginLoaderCreator, creator)

// -----------------------------------------------------

DPluginLoader::DPluginLoader()
    : QObject(),
      d(new Private)
{
}

DPluginLoader::~DPluginLoader()
{
    delete d;
}

DPluginLoader* DPluginLoader::instance()
{
    return &creator->object;
}

void DPluginLoader::init()
{
    d->loadPlugins();
}

QString DPluginLoader::configGroupName() const
{
    return QLatin1String("EnabledDPlugins");
}

QList<DPlugin*> DPluginLoader::allPlugins() const
{
    return d->allPlugins;
}

QList<DPluginAction*> DPluginLoader::pluginsActions(DPluginAction::ActionCategory cat, QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPlugin* const p, allPlugins())
    {
        foreach (DPluginAction* const ac, p->actions(parent))
        {
            if (ac && (ac->actionCategory() == cat))
            {
                list << ac;
            }
         }
     }

     return list;
}

QList<DPluginAction*> DPluginLoader::pluginActions(const QString& pluginId, QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPlugin* const p, allPlugins())
    {
        if (p->id() == pluginId)
        {
            foreach (DPluginAction* const ac, p->actions(parent))
            {
                list << ac;
            }

            break;
        }
    }

    return list;
}

DPluginAction* DPluginLoader::pluginAction(const QString& actionName, QObject* const parent) const
{
    foreach (DPlugin* const p, allPlugins())
    {
        foreach (DPluginAction* const ac, p->actions(parent))
        {
            if (ac && (ac->objectName() == actionName))
            {
                return ac;
            }
        }
     }

     qCCritical(DIGIKAM_GENERAL_LOG) << "DPluginAction named" << actionName
                                     << "not found in" << parent->objectName()
                                     << "(" << parent << ")";

     return 0;
}

QString DPluginLoader::pluginXmlSections(DPluginAction::ActionCategory cat, QObject* const parent) const
{
    QString xml;

    foreach (DPluginAction* const ac, pluginsActions(cat, parent))
    {
        xml.append(ac->xmlSection());
    }

    return xml;
}

void DPluginLoader::appendPluginToBlackList(const QString& filename)
{
    d->blacklist << QLatin1String(DIGIKAM_SHARED_LIBRARY_PREFIX) + filename;
}

void DPluginLoader::appendPluginToWhiteList(const QString& filename)
{
    d->whitelist << QLatin1String(DIGIKAM_SHARED_LIBRARY_PREFIX) + filename;
}

void DPluginLoader::registerPlugins(QObject* const parent)
{
    foreach (DPlugin* const plugin, d->allPlugins)
    {
        plugin->setup(parent);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Actions from plugin named" << plugin->name()
                                     << "registered to" << parent;
    }
}

} // namepace Digikam
