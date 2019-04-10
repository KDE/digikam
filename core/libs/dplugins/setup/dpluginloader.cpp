/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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
#include "dplugingeneric.h"
#include "dplugineditor.h"

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

void DPluginLoader::cleanUp()
{
    foreach (QPluginLoader* const loader, d->allLoaders)
    {
        loader->unload();
    }

    d->allLoaders.clear();
}

QString DPluginLoader::configGroupName() const
{
    return QLatin1String("EnabledDPlugins");
}

QList<DPlugin*> DPluginLoader::allPlugins() const
{
    return d->allPlugins;
}

QList<DPluginAction*> DPluginLoader::pluginsActions(DPluginAction::ActionType type, QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPlugin* const p, allPlugins())
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            foreach (DPluginAction* const ac, gene->actions(parent))
            {
                if (ac && (ac->actionType() == type))
                {
                    list << ac;
                }
            }
        }
    }

    if (list.isEmpty())
    {
        foreach (DPlugin* const p, allPlugins())
        {
            DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                foreach (DPluginAction* const ac, edit->actions(parent))
                {
                    if (ac && (ac->actionType() == type))
                    {
                        list << ac;
                    }
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);
    return list;
}

QList<DPluginAction*> DPluginLoader::pluginsActions(DPluginAction::ActionCategory cat, QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPlugin* const p, allPlugins())
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            foreach (DPluginAction* const ac, gene->actions(parent))
            {
                if (ac && (ac->actionCategory() == cat))
                {
                    list << ac;
                }
            }
        }
    }

    if (list.isEmpty())
    {
        foreach (DPlugin* const p, allPlugins())
        {
            DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                foreach (DPluginAction* const ac, edit->actions(parent))
                {
                    if (ac && (ac->actionCategory() == cat))
                    {
                        list << ac;
                    }
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);
    return list;
}

QList<DPluginAction*> DPluginLoader::pluginActions(const QString& pluginIID, QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPlugin* const p, allPlugins())
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            if (p->iid() == pluginIID)
            {
                foreach (DPluginAction* const ac, gene->actions(parent))
                {
                    list << ac;
                }

                break;
            }
        }
    }

    if (list.isEmpty())
    {
        foreach (DPlugin* const p, allPlugins())
        {
            DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

            if (edit)
            {
                if (p->iid() == pluginIID)
                {
                    foreach (DPluginAction* const ac, edit->actions(parent))
                    {
                        list << ac;
                    }

                    break;
                }
            }
        }
    }

    std::sort(list.begin(), list.end(), DPluginAction::pluginActionLessThan);
    return list;
}

DPluginAction* DPluginLoader::pluginAction(const QString& actionName, QObject* const parent) const
{
    foreach (DPlugin* const p, allPlugins())
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(p);

        if (gene)
        {
            foreach (DPluginAction* const ac, gene->actions(parent))
            {
                if (ac && (ac->objectName() == actionName))
                {
                    return ac;
                }
            }
        }

        DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(p);

        if (edit)
        {
            foreach (DPluginAction* const ac, edit->actions(parent))
            {
                if (ac && (ac->objectName() == actionName))
                {
                    return ac;
                }
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
    d->blacklist << filename;
}

void DPluginLoader::appendPluginToWhiteList(const QString& filename)
{
    d->whitelist << filename;
}

void DPluginLoader::registerGenericPlugins(QObject* const parent)
{
    foreach (DPlugin* const plugin, d->allPlugins)
    {
        DPluginGeneric* const gene = dynamic_cast<DPluginGeneric*>(plugin);

        if (gene)
        {
            gene->setup(parent);
            gene->setVisible(plugin->shouldLoaded());

            qCDebug(DIGIKAM_GENERAL_LOG) << "Generic plugin named" << gene->name()
                                         << "registered to" << parent;
        }
    }
}

void DPluginLoader::registerEditorPlugins(QObject* const parent)
{
    foreach (DPlugin* const plugin, d->allPlugins)
    {
        DPluginEditor* const edit = dynamic_cast<DPluginEditor*>(plugin);

        if (edit)
        {
            edit->setup(parent);
            edit->setVisible(plugin->shouldLoaded());

            qCDebug(DIGIKAM_GENERAL_LOG) << "Editor plugin named" << edit->name()
                                         << "registered to" << parent;
        }
    }
}

} // namepace Digikam
