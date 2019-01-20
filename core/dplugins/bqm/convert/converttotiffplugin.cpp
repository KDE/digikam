/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to TIFF.
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

#include "converttotiffplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert2tiff.h"

namespace Digikam
{

ConvertToTiffPlugin::ConvertToTiffPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ConvertToTiffPlugin::~ConvertToTiffPlugin()
{
}

QString ConvertToTiffPlugin::name() const
{
    return i18n("Convert To TIFF");
}

QString ConvertToTiffPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToTiffPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-tiff"));
}

QString ConvertToTiffPlugin::description() const
{
    return i18n("A tool to convert images to TIFF format");
}

QString ConvertToTiffPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images data to TIFF format.</p>"
                "<p>The Tagged Image File Format is a computer file format for storing raster graphics images, popular among graphic artists, the publishing industry, and photographers.</p>"
                "<p>See details about this format from <a href='https://en.wikipedia.org/wiki/TIFF'>this page</a>.</p>");
}

QList<DPluginAuthor> ConvertToTiffPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2019"))
            ;
}

void ConvertToTiffPlugin::setup(QObject* const parent)
{
    Convert2TIFF* const tool = new Convert2TIFF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
