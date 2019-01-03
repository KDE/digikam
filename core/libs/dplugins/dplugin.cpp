/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define digiKam plugin
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

#include "dplugin.h"

// Qt includes

#include <QApplication>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPlugin::Private
{
public:

    explicit Private()
      : shouldLoaded(false),
        iface(0)
    {
    }

    bool                  shouldLoaded;
    DInfoInterface*       iface;
    QList<DPluginAction*> actions;
};

DPlugin::DPlugin(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
}

DPlugin::~DPlugin()
{
    delete d;
}

QString DPlugin::version() const
{
    return QLatin1String(digikam_version_short);
}

void DPlugin::setInfoIface(DInfoInterface* const iface)
{
    d->iface = iface;
}

DInfoInterface* DPlugin::infoIface() const
{
    return d->iface;
}

int DPlugin::count() const
{
    int count       = 0;
    QObject* parent = 0;

    foreach (DPluginAction* const ac, d->actions)
    {
        if (ac)
        {
            // NOTE: we will return the count of actions registered with the same parents,
            //       as each parent registered the same list of actions through setup().

            if (!count)
            {
                parent = ac->parent(),
                ++count;
            }
            else if (ac->parent() == parent)
            {
                ++count;
            }
        }
    }

    return count;
}

QIcon DPlugin::icon() const
{
    return qApp->windowIcon();
}

QList<DPluginAction*> DPlugin::actions(QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPluginAction* const ac, d->actions)
    {
        if (ac && (ac->parent() == parent))
        {
            list << ac;
        }
    }

    return list;
}

DPluginAction* DPlugin::findActionByName(const QString& name, QObject* const parent) const
{
    foreach (DPluginAction* const ac, actions(parent))
    {
        if (ac && (ac->actionName() == name))
        {
            return ac;
        }
    }

    return 0;
}

void DPlugin::addAction(DPluginAction* const ac)
{
    d->actions.append(ac);
}

QStringList DPlugin::pluginCategories() const
{
    QStringList list;

    foreach (DPluginAction* const ac, d->actions)
    {
        QString cat = ac->actionCategoryToString();

        if (!list.contains(cat))
        {
            list << cat;
        }
    }

    list.sort();

    return list;
}

QStringList DPlugin::pluginAuthors() const
{
    QStringList list;

    foreach (const DPluginAuthor& au, authors())
    {
        if (!list.contains(au.name))
        {
            list << au.name;
        }
    }

    list.sort();

    return list;
}

bool DPlugin::shouldLoaded() const
{
    return d->shouldLoaded;
}

void DPlugin::setShouldLoaded(bool b)
{
    d->shouldLoaded = b;
}

} // namespace Digikam
