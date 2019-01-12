/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to add border
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "borderplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "border.h"

namespace Digikam
{

BorderPlugin::BorderPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

BorderPlugin::~BorderPlugin()
{
}

QString BorderPlugin::name() const
{
    return i18n("Add Border");
}

QString BorderPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BorderPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bordertool"));
}

QString BorderPlugin::description() const
{
    return i18n("A tool to add a border around images");
}

QString BorderPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can add decorative border around images.</p>");
}

QList<DPluginAuthor> BorderPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void BorderPlugin::setup(QObject* const parent)
{
    Border* const tool = new Border(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
