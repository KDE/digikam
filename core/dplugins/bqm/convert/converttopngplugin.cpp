/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to PNG.
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

#include "converttopngplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert2png.h"

namespace Digikam
{

ConvertToPngPlugin::ConvertToPngPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ConvertToPngPlugin::~ConvertToPngPlugin()
{
}

QString ConvertToPngPlugin::name() const
{
    return i18n("Convert To PNG");
}

QString ConvertToPngPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToPngPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-png"));
}

QString ConvertToPngPlugin::description() const
{
    return i18n("A tool to convert images to PNG format");
}

QString ConvertToPngPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images data to PNG format.</p>"
                "<p>The Portable Network Graphics is a raster-graphics file-format that supports lossless data compression.</p>"
                "<p>See details about this format from <a href='https://en.wikipedia.org/wiki/Portable_Network_Graphics'>this page</a>.</p>");
}

QList<DPluginAuthor> ConvertToPngPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2019"))
            ;
}

void ConvertToPngPlugin::setup(QObject* const parent)
{
    Convert2PNG* const tool = new Convert2PNG(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
