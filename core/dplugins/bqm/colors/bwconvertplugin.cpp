/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to Black and White
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

#include "bwconvertplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "bwconvert.h"

namespace Digikam
{

BWConvertPlugin::BWConvertPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

BWConvertPlugin::~BWConvertPlugin()
{
}

QString BWConvertPlugin::name() const
{
    return i18n("Black and White Convert");
}

QString BWConvertPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BWConvertPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bwtonal"));
}

QString BWConvertPlugin::description() const
{
    return i18n("A tool to convert to black and white");
}

QString BWConvertPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images to black and white.</p>");
}

QList<DPluginAuthor> BWConvertPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void BWConvertPlugin::setup(QObject* const parent)
{
    BWConvert* const tool = new BWConvert(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
