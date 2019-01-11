/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to correct vignetting
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

#include "antivignettingplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "antivignetting.h"

namespace Digikam
{

AntiVignettingPlugin::AntiVignettingPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

AntiVignettingPlugin::~AntiVignettingPlugin()
{
}

QString AntiVignettingPlugin::name() const
{
    return i18n("Anti-Vignetting");
}

QString AntiVignettingPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AntiVignettingPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("antivignetting"));
}

QString AntiVignettingPlugin::description() const
{
    return i18n("Remove/add vignetting to photograph");
}

QString AntiVignettingPlugin::details() const
{
    return i18n("<p>This batch Queue Manager tool remove or add vignettings to images.</p>");
}

QList<DPluginAuthor> AntiVignettingPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            ;
}

void AntiVignettingPlugin::setup(QObject* const parent)
{
    AntiVignetting* const tool = new AntiVignetting(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
