/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Brightness/Contrast/Gamma batch tool.
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

#include "bcgcorrection.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "bcgsettings.h"
#include "bcgfilter.h"

namespace Digikam
{

BCGCorrection::BCGCorrection(QObject* const parent)
    : BatchTool(QLatin1String("BCGCorrection"), ColorTool, parent)
{
    m_settingsView = 0;
    setToolTitle(i18n("BCG Correction"));
    setToolDescription(i18n("Fix Brightness/Contrast/Gamma."));
    setToolIconName(QLatin1String("contrast"));
}

BCGCorrection::~BCGCorrection()
{
}

void BCGCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new BCGSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings BCGCorrection::defaultSettings()
{
    BatchToolSettings prm;
    BCGContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Brightness"), (double)defaultPrm.brightness);
    prm.insert(QLatin1String("Contrast"),   (double)defaultPrm.contrast);
    prm.insert(QLatin1String("Gamma"),      (double)defaultPrm.gamma);

    return prm;
}

void BCGCorrection::slotAssignSettings2Widget()
{
    BCGContainer prm;
    prm.brightness = settings()[QLatin1String("Brightness")].toDouble();
    prm.contrast   = settings()[QLatin1String("Contrast")].toDouble();
    prm.gamma      = settings()[QLatin1String("Gamma")].toDouble();
    m_settingsView->setSettings(prm);
}

void BCGCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    BCGContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Brightness"), (double)currentPrm.brightness);
    prm.insert(QLatin1String("Contrast"),   (double)currentPrm.contrast);
    prm.insert(QLatin1String("Gamma"),      (double)currentPrm.gamma);

    BatchTool::slotSettingsChanged(prm);
}

bool BCGCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BCGContainer prm;
    prm.brightness = settings()[QLatin1String("Brightness")].toDouble();
    prm.contrast   = settings()[QLatin1String("Contrast")].toDouble();
    prm.gamma      = settings()[QLatin1String("Gamma")].toDouble();

    BCGFilter bcg(&image(), 0L, prm);
    applyFilter(&bcg);

    return (savefromDImg());
}

}  // namespace Digikam
