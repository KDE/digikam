/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to sharp images
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

#include "sharpenplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "sharpen.h"

namespace Digikam
{

SharpenPlugin::SharpenPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

SharpenPlugin::~SharpenPlugin()
{
}

QString SharpenPlugin::name() const
{
    return i18n("Sharpen Image");
}

QString SharpenPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SharpenPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("sharpenimage"));
}

QString SharpenPlugin::description() const
{
    return i18n("A tool to sharp images");
}

QString SharpenPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can sharp images.</p>");
}

QList<DPluginAuthor> SharpenPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Matthias Welwarsky"),
                             QString::fromUtf8("matze at welwarsky dot de"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"))
            ;
}

void SharpenPlugin::setup(QObject* const parent)
{
    Sharpen* const tool = new Sharpen(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
