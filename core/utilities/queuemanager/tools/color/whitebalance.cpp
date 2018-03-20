/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-27
 * Description : White Balance batch tool.
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

#include "whitebalance.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "wbfilter.h"
#include "wbsettings.h"

namespace Digikam
{

WhiteBalance::WhiteBalance(QObject* const parent)
    : BatchTool(QLatin1String("WhiteBalance"), ColorTool, parent)
{
    m_settingsView = 0;

    setToolTitle(i18n("White Balance"));
    setToolDescription(i18n("Adjust White Balance."));
    setToolIconName(QLatin1String("bordertool"));
}

WhiteBalance::~WhiteBalance()
{
}

void WhiteBalance::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new WBSettings(m_settingsWidget);
    m_settingsView->showAdvancedButtons(false);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings WhiteBalance::defaultSettings()
{
    BatchToolSettings prm;
    WBContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("black"),          (double)defaultPrm.black);
    prm.insert(QLatin1String("temperature"),    (double)defaultPrm.temperature);
    prm.insert(QLatin1String("green"),          (double)defaultPrm.green);
    prm.insert(QLatin1String("dark"),           (double)defaultPrm.dark);
    prm.insert(QLatin1String("gamma"),          (double)defaultPrm.gamma);
    prm.insert(QLatin1String("saturation"),     (double)defaultPrm.saturation);
    prm.insert(QLatin1String("expositionMain"), (double)defaultPrm.expositionMain);
    prm.insert(QLatin1String("expositionFine"), (double)defaultPrm.expositionFine);

    return prm;
}

void WhiteBalance::slotAssignSettings2Widget()
{
    WBContainer prm;

    prm.black          = settings()[QLatin1String("black")].toDouble();
    prm.temperature    = settings()[QLatin1String("temperature")].toDouble();
    prm.green          = settings()[QLatin1String("green")].toDouble();
    prm.dark           = settings()[QLatin1String("dark")].toDouble();
    prm.gamma          = settings()[QLatin1String("gamma")].toDouble();
    prm.saturation     = settings()[QLatin1String("saturation")].toDouble();
    prm.expositionMain = settings()[QLatin1String("expositionMain")].toDouble();
    prm.expositionFine = settings()[QLatin1String("expositionFine")].toDouble();

    m_settingsView->setSettings(prm);
}

void WhiteBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    WBContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("black"),          (double)currentPrm.black);
    prm.insert(QLatin1String("temperature"),    (double)currentPrm.temperature);
    prm.insert(QLatin1String("green"),          (double)currentPrm.green);
    prm.insert(QLatin1String("dark"),           (double)currentPrm.dark);
    prm.insert(QLatin1String("gamma"),          (double)currentPrm.gamma);
    prm.insert(QLatin1String("saturation"),     (double)currentPrm.saturation);
    prm.insert(QLatin1String("expositionMain"), (double)currentPrm.expositionMain);
    prm.insert(QLatin1String("expositionFine"), (double)currentPrm.expositionFine);

    BatchTool::slotSettingsChanged(prm);
}

bool WhiteBalance::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    WBContainer prm;

    prm.black          = settings()[QLatin1String("black")].toDouble();
    prm.temperature    = settings()[QLatin1String("temperature")].toDouble();
    prm.green          = settings()[QLatin1String("green")].toDouble();
    prm.dark           = settings()[QLatin1String("dark")].toDouble();
    prm.gamma          = settings()[QLatin1String("gamma")].toDouble();
    prm.saturation     = settings()[QLatin1String("saturation")].toDouble();
    prm.expositionMain = settings()[QLatin1String("expositionMain")].toDouble();
    prm.expositionFine = settings()[QLatin1String("expositionFine")].toDouble();

    WBFilter wb(&image(), 0L, prm);
    applyFilter(&wb);

    return (savefromDImg());
}

}  // namespace Digikam
