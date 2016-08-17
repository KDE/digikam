/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye tool for automatic detection and correction filter.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "redeyecorrection.h"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "redeyecorrectionfilter.h"

namespace Digikam
{

RedEyeCorrection::RedEyeCorrection(QObject* const parent)
    : BatchTool(QLatin1String("RedEyeCorrection"), EnhanceTool, parent)
{
    setToolTitle(i18n("RedEye-Correction"));
    setToolDescription(i18n("Automatically detect and correct RedEye effect."));
    setToolIconName(QLatin1String("redeyes"));
}

RedEyeCorrection::~RedEyeCorrection()
{
}

void RedEyeCorrection::registerSettingsWidget()
{
    // TODO: add settings widget
/*
    m_settingsWidget = new QWidget;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
*/

    BatchTool::registerSettingsWidget();
}

BatchToolSettings RedEyeCorrection::defaultSettings()
{
    // TODO: add settings widget serialization of parameters
    BatchToolSettings prm;

    return prm;
}

void RedEyeCorrection::slotAssignSettings2Widget()
{
    // TODO: add settings widget serialization of parameters
}

void RedEyeCorrection::slotSettingsChanged()
{
    // TODO: add settings widget serialization of parameters
}

bool RedEyeCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    RedEyeCorrectionFilter vig(&image(), 0L);
    applyFilter(&vig);

    return (savefromDImg());
}

}  // namespace Digikam
