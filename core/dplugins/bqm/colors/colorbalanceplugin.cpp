/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to fix colors balance
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

#include "colorbalanceplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorbalance.h"

namespace Digikam
{

ColorBalancePlugin::ColorBalancePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ColorBalancePlugin::~ColorBalancePlugin()
{
}

QString ColorBalancePlugin::name() const
{
    return i18n("Color Balance");
}

QString ColorBalancePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorBalancePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustrgb"));
}

QString ColorBalancePlugin::description() const
{
    return i18n("A tool to adjust color balance");
}

QString ColorBalancePlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can adjust color balance from images.</p>");
}

QList<DPluginAuthor> ColorBalancePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void ColorBalancePlugin::setup(QObject* const parent)
{
    ColorBalance* const tool = new ColorBalance(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
