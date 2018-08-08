/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : template for external plugin
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QIcon>

namespace Digikam
{

class Q_DECL_HIDDEN DPlugin::Private
{
public:

    explicit Private()
      : iface(0)
    {
    }

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

void DPlugin::setInfoIface(DInfoInterface* const iface)
{
    d->iface = iface;
}

DInfoInterface* DPlugin::infoIface() const
{
    return d->iface;
}

QIcon DPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("digikam"));
}

QList<DPluginAction*> DPlugin::actions() const
{
    return d->actions;
}

DPluginAction* DPlugin::findActionByName(const QString& name) const
{
    foreach (DPluginAction* const ac, actions())
    {
        if (ac->actionName() == name)
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

QString DPlugin::aboutDataText() const
{
    return QString();
}

} // namespace Digikam
