/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPlugin::Private
{
public:

    explicit Private()
      : shouldLoaded(false)
    {
    }

    bool shouldLoaded;
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

QIcon DPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("plugins"));
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
