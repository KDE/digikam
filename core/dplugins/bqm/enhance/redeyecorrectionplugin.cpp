/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to reduce red eyes
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

#include "redeyecorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "redeyecorrection.h"

namespace Digikam
{

RedEyeCorrectionPlugin::RedEyeCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

RedEyeCorrectionPlugin::~RedEyeCorrectionPlugin()
{
}

QString RedEyeCorrectionPlugin::name() const
{
    return i18n("Red Eye Correction");
}

QString RedEyeCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RedEyeCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("redeyes"));
}

QString RedEyeCorrectionPlugin::description() const
{
    return i18n("A tool to automatically detect and correct red eye effect");
}

QString RedEyeCorrectionPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can reduce red eye effect on images.</p>");
}

QList<DPluginAuthor> RedEyeCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Omar Amin"),
                             QLatin1String("Omar dot moh dot amin at gmail dot com"),
                             QLatin1String("(C) 2016"))
            ;
}

void RedEyeCorrectionPlugin::setup(QObject* const parent)
{
    RedEyeCorrection* const tool = new RedEyeCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
