/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereduction.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "nrestimate.h"
#include "nrfilter.h"
#include "nrsettings.h"

namespace Digikam
{

NoiseReduction::NoiseReduction(QObject* const parent)
    : BatchTool(QLatin1String("NoiseReduction"), EnhanceTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("Noise Reduction"));
    setToolDescription(i18n("Remove photograph noise using wavelets."));
    setToolIconName(QLatin1String("noisereduction"));
}

NoiseReduction::~NoiseReduction()
{
}

void NoiseReduction::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new NRSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalEstimateNoise()),
            this, SLOT(slotSettingsChanged()));

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings NoiseReduction::defaultSettings()
{
    BatchToolSettings prm;
    NRContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("YThreshold"),    (double)defaultPrm.thresholds[0]);
    prm.insert(QLatin1String("CrThreshold"),   (double)defaultPrm.thresholds[1]);
    prm.insert(QLatin1String("CbThreshold"),   (double)defaultPrm.thresholds[2]);
    prm.insert(QLatin1String("YSoftness"),     (double)defaultPrm.softness[0]);
    prm.insert(QLatin1String("CrSoftness"),    (double)defaultPrm.softness[1]);
    prm.insert(QLatin1String("CbSoftness"),    (double)defaultPrm.softness[2]);
    prm.insert(QLatin1String("EstimateNoise"), false);

    return prm;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    NRContainer prm;
    prm.thresholds[0] = settings()[QLatin1String("YThreshold")].toDouble();
    prm.thresholds[1] = settings()[QLatin1String("CrThreshold")].toDouble();
    prm.thresholds[2] = settings()[QLatin1String("CbThreshold")].toDouble();
    prm.softness[0]   = settings()[QLatin1String("YSoftness")].toDouble();
    prm.softness[1]   = settings()[QLatin1String("CrSoftness")].toDouble();
    prm.softness[2]   = settings()[QLatin1String("CbSoftness")].toDouble();
    m_settingsView->setSettings(prm);
    m_settingsView->setEstimateNoise(settings()[QLatin1String("EstimateNoise")].toBool());
}

void NoiseReduction::slotSettingsChanged()
{
    BatchToolSettings prm;
    NRContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("YThreshold"),    (double)currentPrm.thresholds[0]);
    prm.insert(QLatin1String("CrThreshold"),   (double)currentPrm.thresholds[1]);
    prm.insert(QLatin1String("CbThreshold"),   (double)currentPrm.thresholds[2]);
    prm.insert(QLatin1String("YSoftness"),     (double)currentPrm.softness[0]);
    prm.insert(QLatin1String("CrSoftness"),    (double)currentPrm.softness[1]);
    prm.insert(QLatin1String("CbSoftness"),    (double)currentPrm.softness[2]);
    prm.insert(QLatin1String("EstimateNoise"), m_settingsView->estimateNoise());

    BatchTool::slotSettingsChanged(prm);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    NRContainer prm;

    if (settings()[QLatin1String("EstimateNoise")].toBool())
    {
        NREstimate nre(&image());
        nre.startFilterDirectly();
        prm = nre.settings();
    }
    else
    {
        prm.thresholds[0] = settings()[QLatin1String("YThreshold")].toDouble();
        prm.thresholds[1] = settings()[QLatin1String("CrThreshold")].toDouble();
        prm.thresholds[2] = settings()[QLatin1String("CbThreshold")].toDouble();
        prm.softness[0]   = settings()[QLatin1String("YSoftness")].toDouble();
        prm.softness[1]   = settings()[QLatin1String("CrSoftness")].toDouble();
        prm.softness[2]   = settings()[QLatin1String("CbSoftness")].toDouble();
    }

    NRFilter wnr(&image(), 0L, prm);
    applyFilter(&wnr);

    return (savefromDImg());
}

}  // namespace Digikam
