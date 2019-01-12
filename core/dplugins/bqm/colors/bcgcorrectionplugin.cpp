/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust BCG
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

#include "bcgcorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "bcgcorrection.h"

namespace Digikam
{

BCGCorrectionPlugin::BCGCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

BCGCorrectionPlugin::~BCGCorrectionPlugin()
{
}

QString BCGCorrectionPlugin::name() const
{
    return i18n("BCG Correction");
}

QString BCGCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BCGCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString BCGCorrectionPlugin::description() const
{
    return i18n("A tool to fix Brightness/Contrast/Gamma");
}

QString BCGCorrectionPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can adjust Brightness/Contrast/Gamma from images.</p>");
}

QList<DPluginAuthor> BCGCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void BCGCorrectionPlugin::setup(QObject* const parent)
{
    BCGCorrection* const tool = new BCGCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
