/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    setToolIconName(QLatin1String("redeyecorrection"));
}

RedEyeCorrection::~RedEyeCorrection()
{
}

void RedEyeCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    //m_settingsView   = new AntiVignettingSettings(m_settingsWidget);

    //connect(m_settingsView, SIGNAL(signalSettingsChanged()),
    //        this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings RedEyeCorrection::defaultSettings()
{
    BatchToolSettings prm;
    //AntiVignettingContainer defaultPrm = m_settingsView->defaultSettings();

//    prm.insert(QLatin1String("addvignetting"), (bool)defaultPrm.addvignetting);
//    prm.insert(QLatin1String("density"), (double)defaultPrm.density);
//    prm.insert(QLatin1String("power"), (double)defaultPrm.power);
//    prm.insert(QLatin1String("innerradius"), (double)defaultPrm.innerradius);
//    prm.insert(QLatin1String("outerradius"), (double)defaultPrm.outerradius);
//    prm.insert(QLatin1String("xshift"), (double)defaultPrm.xshift);
//    prm.insert(QLatin1String("yshift"), (double)defaultPrm.yshift);

    return prm;
}

void RedEyeCorrection::slotAssignSettings2Widget()
{
//    AntiVignettingContainer prm;
//    prm.addvignetting = settings()[QLatin1String("addvignetting")].toBool();
//    prm.density       = settings()[QLatin1String("density")].toDouble();
//    prm.power         = settings()[QLatin1String("power")].toDouble();
//    prm.innerradius   = settings()[QLatin1String("innerradius")].toDouble();
//    prm.outerradius   = settings()[QLatin1String("outerradius")].toDouble();
//    prm.xshift        = settings()[QLatin1String("xshift")].toDouble();
//    prm.yshift        = settings()[QLatin1String("yshift")].toDouble();
//    m_settingsView->setSettings(prm);
}

void RedEyeCorrection::slotSettingsChanged()
{
//    BatchToolSettings prm;
//    AntiVignettingContainer currentPrm = m_settingsView->settings();

//    prm.insert(QLatin1String("addvignetting"), (bool)currentPrm.addvignetting);
//    prm.insert(QLatin1String("density"), (double)currentPrm.density);
//    prm.insert(QLatin1String("power"), (double)currentPrm.power);
//    prm.insert(QLatin1String("innerradius"), (double)currentPrm.innerradius);
//    prm.insert(QLatin1String("outerradius"), (double)currentPrm.outerradius);
//    prm.insert(QLatin1String("xshift"), (double)currentPrm.xshift);
//    prm.insert(QLatin1String("yshift"), (double)currentPrm.yshift);

//    BatchTool::slotSettingsChanged(prm);
}

bool RedEyeCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

//    AntiVignettingContainer prm;
//    prm.addvignetting = settings()[QLatin1String("addvignetting")].toBool();
//    prm.density       = settings()[QLatin1String("density")].toDouble();
//    prm.power         = settings()[QLatin1String("power")].toDouble();
//    prm.innerradius   = settings()[QLatin1String("innerradius")].toDouble();
//    prm.outerradius   = settings()[QLatin1String("outerradius")].toDouble();
//    prm.xshift        = settings()[QLatin1String("xshift")].toDouble();
//    prm.yshift        = settings()[QLatin1String("yshift")].toDouble();

    RedEyeCorrectionFilter vig(&image(), 0L);
    applyFilter(&vig);

    return (savefromDImg());
}

}  // namespace Digikam
