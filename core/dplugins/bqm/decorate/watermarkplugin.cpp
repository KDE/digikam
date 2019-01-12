/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to add watermark
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

#include "watermarkplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "watermark.h"

namespace Digikam
{

WaterMarkPlugin::WaterMarkPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

WaterMarkPlugin::~WaterMarkPlugin()
{
}

QString WaterMarkPlugin::name() const
{
    return i18n("Add Watermark");
}

QString WaterMarkPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WaterMarkPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("insert-text"));
}

QString WaterMarkPlugin::description() const
{
    return i18n("A tool to overlay an image or text as a visible watermark");
}

QString WaterMarkPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can add a text watermark over images.</p>");
}

QList<DPluginAuthor> WaterMarkPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            << DPluginAuthor(QLatin1String("Mikkel Baekhoej Christensen"),
                             QLatin1String("mbc at baekhoej dot dk"),
                             QLatin1String("(C) 2010"))
            << DPluginAuthor(QLatin1String("Ahmed Fathi"),
                             QLatin1String("ahmed dot fathi dot abdelmageed at gmail dot com"),
                             QLatin1String("(C) 2017"))            
            ;
}

void WaterMarkPlugin::setup(QObject* const parent)
{
    WaterMark* const tool = new WaterMark(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
