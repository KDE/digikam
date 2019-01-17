/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to apply color effects
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

#include "colorfxplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorfx.h"

namespace Digikam
{

ColorFXPlugin::ColorFXPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ColorFXPlugin::~ColorFXPlugin()
{
}

QString ColorFXPlugin::name() const
{
    return i18n("Color Effects");
}

QString ColorFXPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorFXPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ColorFXPlugin::description() const
{
    return i18n("A tool to apply color effects");
}

QString ColorFXPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can apply color effects over images.</p>");
}

QList<DPluginAuthor> ColorFXPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Alexander Dymo"),
                             QLatin1String("adymo at develop dot org"),
                             QLatin1String("(C) 2012"))
            ;
}

void ColorFXPlugin::setup(QObject* const parent)
{
    ColorFX* const tool = new ColorFX(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
