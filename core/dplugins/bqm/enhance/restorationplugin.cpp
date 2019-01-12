/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to restore images
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

#include "restorationplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "restoration.h"

namespace Digikam
{

RestorationPlugin::RestorationPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

RestorationPlugin::~RestorationPlugin()
{
}

QString RestorationPlugin::name() const
{
    return i18n("Restoration");
}

QString RestorationPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RestorationPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString RestorationPlugin::description() const
{
    return i18n("A tool to restore photographs using Greystoration algorithm");
}

QString RestorationPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can restore images.</p>");
}

QList<DPluginAuthor> RestorationPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            ;
}

void RestorationPlugin::setup(QObject* const parent)
{
    Restoration* const tool = new Restoration(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
