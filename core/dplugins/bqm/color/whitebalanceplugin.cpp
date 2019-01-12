/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to fix white balance
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

#include "whitebalanceplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "whitebalance.h"

namespace Digikam
{

WhiteBalancePlugin::WhiteBalancePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

WhiteBalancePlugin::~WhiteBalancePlugin()
{
}

QString WhiteBalancePlugin::name() const
{
    return i18n("White Balance");
}

QString WhiteBalancePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WhiteBalancePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString WhiteBalancePlugin::description() const
{
    return i18n("A tool to adjust white balance");
}

QString WhiteBalancePlugin::details() const
{
    return i18n("<p>This batch Queue Manager tool can adjust the white balance from images.</p>");
}

QList<DPluginAuthor> WhiteBalancePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void WhiteBalancePlugin::setup(QObject* const parent)
{
    WhiteBalance* const tool = new WhiteBalance(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
