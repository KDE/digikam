/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust color curves
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

#include "curvesadjustplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "curvesadjust.h"

namespace Digikam
{

CurvesAdjustPlugin::CurvesAdjustPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

CurvesAdjustPlugin::~CurvesAdjustPlugin()
{
}

QString CurvesAdjustPlugin::name() const
{
    return i18n("Curves Adjust");
}

QString CurvesAdjustPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CurvesAdjustPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString CurvesAdjustPlugin::description() const
{
    return i18n("A tool to adjust color curves");
}

QString CurvesAdjustPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can adjust the color curves from images.</p>");
}

QList<DPluginAuthor> CurvesAdjustPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void CurvesAdjustPlugin::setup(QObject* const parent)
{
    CurvesAdjust* const tool = new CurvesAdjust(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
