/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to crop images.
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

#include "cropplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "crop.h"

namespace Digikam
{

CropPlugin::CropPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

CropPlugin::~CropPlugin()
{
}

QString CropPlugin::name() const
{
    return i18n("Crop");
}

QString CropPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CropPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString CropPlugin::description() const
{
    return i18n("Crop images to a region");
}

QString CropPlugin::details() const
{
    return i18n("<p>This batch Queue Manager tool can crop images to a region.</p>");
}

QList<DPluginAuthor> CropPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2012-2019"))
            ;
}

void CropPlugin::setup(QObject* const parent)
{
    Crop* const tool = new Crop(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
