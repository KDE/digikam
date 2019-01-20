/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert 16 bits color depth to 8
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

#include "convert16to8plugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert16to8.h"

namespace Digikam
{

Convert16To8Plugin::Convert16To8Plugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

Convert16To8Plugin::~Convert16To8Plugin()
{
}

QString Convert16To8Plugin::name() const
{
    return i18n("Convert to 8 bits");
}

QString Convert16To8Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert16To8Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth16to8"));
}

QString Convert16To8Plugin::description() const
{
    return i18n("A tool to convert color depth to 8 bits");
}

QString Convert16To8Plugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert image color depth to 8 bits.</p>");
}

QList<DPluginAuthor> Convert16To8Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void Convert16To8Plugin::setup(QObject* const parent)
{
    Convert16to8* const tool = new Convert16to8(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
