/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to blur images
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

#include "blurplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "blur.h"

namespace Digikam
{

BlurPlugin::BlurPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

BlurPlugin::~BlurPlugin()
{
}

QString BlurPlugin::name() const
{
    return i18n("Blur Image");
}

QString BlurPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurimage"));
}

QString BlurPlugin::description() const
{
    return i18n("A tool to blur images");
}

QString BlurPlugin::details() const
{
    return i18n("<p>This batch Queue Manager tool can blur images.</p>");
}

QList<DPluginAuthor> BlurPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            ;
}

void BlurPlugin::setup(QObject* const parent)
{
    Blur* const tool = new Blur(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
