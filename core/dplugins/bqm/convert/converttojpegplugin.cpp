/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to jpeg.
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

#include "converttojpegplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert2jpeg.h"

namespace Digikam
{

ConvertToJpegPlugin::ConvertToJpegPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ConvertToJpegPlugin::~ConvertToJpegPlugin()
{
}

QString ConvertToJpegPlugin::name() const
{
    return i18n("Convert To JPEG");
}

QString ConvertToJpegPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToJpegPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString ConvertToJpegPlugin::description() const
{
    return i18n("Convert images to JPEG format");
}

QString ConvertToJpegPlugin::details() const
{
    return i18n("<p>This batch Queue manager tool can convert images data to JPEG format.</p>"
                "<p>Warning: JPEG is a lossy image compression format.</p>"
                "<p>See details about this format from <a href='https://en.wikipedia.org/wiki/JPEG'>this page</a>.</p>");
}

QList<DPluginAuthor> ConvertToJpegPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2008-2019"))
            ;
}

void ConvertToJpegPlugin::setup(QObject* const parent)
{
    Convert2JPEG* const tool = new Convert2JPEG(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
