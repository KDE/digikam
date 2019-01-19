/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert 8 bits color depth to 16
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

#include "convert8to16plugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert8to16.h"

namespace Digikam
{

Convert8To16Plugin::Convert8To16Plugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

Convert8To16Plugin::~Convert8To16Plugin()
{
}

QString Convert8To16Plugin::name() const
{
    return i18n("Convert to 16 bits");
}

QString Convert8To16Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert8To16Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth8to16"));
}

QString Convert8To16Plugin::description() const
{
    return i18n("A tool to convert color depth to 16 bits");
}

QString Convert8To16Plugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images color depth to 16 bits.</p>");
}

QList<DPluginAuthor> Convert8To16Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void Convert8To16Plugin::setup(QObject* const parent)
{
    Convert8to16* const tool = new Convert8to16(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
