/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to PGF.
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

#include "converttopgfplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert2pgf.h"

namespace Digikam
{

ConvertToPgfPlugin::ConvertToPgfPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ConvertToPgfPlugin::~ConvertToPgfPlugin()
{
}

QString ConvertToPgfPlugin::name() const
{
    return i18n("Convert To PGF");
}

QString ConvertToPgfPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToPgfPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString ConvertToPgfPlugin::description() const
{
    return i18n("A tool to convert images to PGF format");
}

QString ConvertToPgfPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can convert images data to PGF format.</p>"
                "<p>The Progressive Graphics File is a wavelet-based bitmapped image format that employs lossless and lossy data compression.</p>"
                "<p>See details about this format from <a href='https://en.wikipedia.org/wiki/Progressive_Graphics_File'>this page</a>.</p>");
}

QList<DPluginAuthor> ConvertToPgfPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"))
            ;
}

void ConvertToPgfPlugin::setup(QObject* const parent)
{
    Convert2PGF* const tool = new Convert2PGF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
