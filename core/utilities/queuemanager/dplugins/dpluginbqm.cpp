/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Batch Queue Manager digiKam plugin definition.
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

#include "dpluginbqm.h"

// Qt includes

#include <QApplication>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginBqm::Private
{
public:

    explicit Private()
    {
    }

    QList<BatchTool*> tools;
};

DPluginBqm::DPluginBqm(QObject* const parent)
    : DPlugin(parent),
      d(new Private)
{
}

DPluginBqm::~DPluginBqm()
{
    delete d;
}

void DPluginBqm::setVisible(bool b)
{
    emit signalVisible(b);
}

int DPluginBqm::count() const
{
    return d->tools.count();
}

QList<BatchTool*> DPluginBqm::tools(QObject* const parent) const
{
    QList<BatchTool*> list;

    foreach (BatchTool* const t, d->tools)
    {
        if (t && (t->parent() == parent))
        {
            list << t;
        }
    }

    return list;
}

BatchTool* DPluginBqm::findToolByName(const QString& name, QObject* const parent) const
{
    foreach (BatchTool* const t, tools(parent))
    {
        if (t && (t->objectName() == name))
        {
            return t;
        }
    }

    return nullptr;
}

void DPluginBqm::addTool(BatchTool* const t)
{
    t->setProperty("DPluginIId",      iid());
    t->setProperty("DPluginIfaceIId", ifaceIid());
    d->tools.append(t);
}

QStringList DPluginBqm::categories() const
{
    QStringList list;

    foreach (BatchTool* const t, d->tools)
    {
        QString cat = t->toolGroupToString();

        if (!list.contains(cat))
        {
            list << cat;
        }
    }

    list.sort();

    return list;
}

bool DPluginBqm::hasVisibilityProperty() const
{
    // NOTE: all BQM plugins are not yet configurable.
    // Code is missing in BQM to check workflow tools list validity if a plugin is disabled from setup dialog.
    return false;
}

} // namespace Digikam
