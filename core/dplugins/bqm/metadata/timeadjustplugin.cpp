/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to ajust time
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

#include "timeadjustplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "timeadjust.h"

namespace Digikam
{

TimeAdjustPlugin::TimeAdjustPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

TimeAdjustPlugin::~TimeAdjustPlugin()
{
}

QString TimeAdjustPlugin::name() const
{
    return i18n("Time Adjust");
}

QString TimeAdjustPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TimeAdjustPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("appointment-new"));
}

QString TimeAdjustPlugin::description() const
{
    return i18n("A tool to adjust date and time-stamp from images");
}

QString TimeAdjustPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can adjust time in images.</p>");
}

QList<DPluginAuthor> TimeAdjustPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            ;
}

void TimeAdjustPlugin::setup(QObject* const parent)
{
    TimeAdjust* const tool = new TimeAdjust(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
