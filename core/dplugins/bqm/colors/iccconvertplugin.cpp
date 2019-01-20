/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to color space
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

#include "iccconvertplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iccconvert.h"

namespace Digikam
{

IccConvertPlugin::IccConvertPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

IccConvertPlugin::~IccConvertPlugin()
{
}

QString IccConvertPlugin::name() const
{
    return i18n("Color Profile Conversion");
}

QString IccConvertPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon IccConvertPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-display-color"));
}

QString IccConvertPlugin::description() const
{
    return i18n("A tool to convert images to a color space");
}

QString IccConvertPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images to a different color space.</p>");
}

QList<DPluginAuthor> IccConvertPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void IccConvertPlugin::setup(QObject* const parent)
{
    IccConvert* const tool = new IccConvert(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
