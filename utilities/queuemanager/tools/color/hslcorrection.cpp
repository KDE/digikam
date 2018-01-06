/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Hue/Saturation/Lightness batch tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "hslcorrection.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "hslfilter.h"
#include "hslsettings.h"

namespace Digikam
{

HSLCorrection::HSLCorrection(QObject* const parent)
    : BatchTool(QLatin1String("HSLCorrection"), ColorTool, parent)
{
    m_settingsView = 0;
    setToolTitle(i18n("HSL Correction"));
    setToolDescription(i18n("Fix Hue/Saturation/Lightness."));
    setToolIconName(QLatin1String("adjusthsl"));
}

HSLCorrection::~HSLCorrection()
{
}

void HSLCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new HSLSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings HSLCorrection::defaultSettings()
{
    BatchToolSettings prm;
    HSLContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Hue"),        (double)defaultPrm.hue);
    prm.insert(QLatin1String("Saturation"), (double)defaultPrm.saturation);
    prm.insert(QLatin1String("Lightness"),  (double)defaultPrm.lightness);
    prm.insert(QLatin1String("Vibrance"),   (double)defaultPrm.vibrance);

    return prm;
}

void HSLCorrection::slotAssignSettings2Widget()
{
    HSLContainer prm;
    prm.hue        = settings()[QLatin1String("Hue")].toDouble();
    prm.saturation = settings()[QLatin1String("Saturation")].toDouble();
    prm.lightness  = settings()[QLatin1String("Lightness")].toDouble();
    prm.vibrance   = settings()[QLatin1String("Vibrance")].toDouble();
    m_settingsView->setSettings(prm);
}

void HSLCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    HSLContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Hue"),        (double)currentPrm.hue);
    prm.insert(QLatin1String("Saturation"), (double)currentPrm.saturation);
    prm.insert(QLatin1String("Lightness"),  (double)currentPrm.lightness);
    prm.insert(QLatin1String("Vibrance"),   (double)currentPrm.vibrance);

    BatchTool::slotSettingsChanged(prm);
}

bool HSLCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    HSLContainer prm;
    prm.hue        = settings()[QLatin1String("Hue")].toDouble();
    prm.saturation = settings()[QLatin1String("Saturation")].toDouble();
    prm.lightness  = settings()[QLatin1String("Lightness")].toDouble();
    prm.vibrance   = settings()[QLatin1String("Vibrance")].toDouble();

    HSLFilter hsl(&image(), 0L, prm);
    applyFilter(&hsl);

    return (savefromDImg());
}

}  // namespace Digikam
