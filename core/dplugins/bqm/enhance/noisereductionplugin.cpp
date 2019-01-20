/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to to reduce noise
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

#include "noisereductionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "noisereduction.h"

namespace Digikam
{

NoiseReductionPlugin::NoiseReductionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

NoiseReductionPlugin::~NoiseReductionPlugin()
{
}

QString NoiseReductionPlugin::name() const
{
    return i18n("Noise Reduction");
}

QString NoiseReductionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon NoiseReductionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("noisereduction"));
}

QString NoiseReductionPlugin::description() const
{
    return i18n("A tool to remove photograph noise using wavelets");
}

QString NoiseReductionPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can reduce noise in images.</p>");
}

QList<DPluginAuthor> NoiseReductionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"))
            ;
}

void NoiseReductionPlugin::setup(QObject* const parent)
{
    NoiseReduction* const tool = new NoiseReduction(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
