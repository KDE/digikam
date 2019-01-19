/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to remove metadata
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

#include "removemetadataplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "removemetadata.h"

namespace Digikam
{

RemoveMetadataPlugin::RemoveMetadataPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

RemoveMetadataPlugin::~RemoveMetadataPlugin()
{
}

QString RemoveMetadataPlugin::name() const
{
    return i18n("Remove Metadata");
}

QString RemoveMetadataPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RemoveMetadataPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("format-text-code"));
}

QString RemoveMetadataPlugin::description() const
{
    return i18n("A tool to remove Exif, Iptc, or Xmp metadata from images");
}

QString RemoveMetadataPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can remove metadata as Exif, Iptc, or Xmp from images.</p>");
}

QList<DPluginAuthor> RemoveMetadataPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void RemoveMetadataPlugin::setup(QObject* const parent)
{
    RemoveMetadata* const tool = new RemoveMetadata(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
