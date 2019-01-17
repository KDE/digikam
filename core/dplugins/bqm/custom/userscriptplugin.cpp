/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to run user script.
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

#include "userscriptplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "userscript.h"

namespace Digikam
{

UserScriptPlugin::UserScriptPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

UserScriptPlugin::~UserScriptPlugin()
{
}

QString UserScriptPlugin::name() const
{
    return i18n("User Shell Script");
}

QString UserScriptPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon UserScriptPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-x-script"));
}

QString UserScriptPlugin::description() const
{
    return i18n("A tool to execute a custom shell script");
}

QString UserScriptPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can run user shell script as workflow stage.</p>");
}

QList<DPluginAuthor> UserScriptPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            << DPluginAuthor(QLatin1String("Hubert Law"),
                             QLatin1String("hhclaw dot eb at gmail dot com"),
                             QLatin1String("(C) 2014"))
            ;
}

void UserScriptPlugin::setup(QObject* const parent)
{
    UserScript* const tool = new UserScript(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
