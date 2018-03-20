/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance batch tool.
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

#include "colorbalance.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "cbfilter.h"
#include "cbsettings.h"

namespace Digikam
{

ColorBalance::ColorBalance(QObject* const parent)
    : BatchTool(QLatin1String("ColorBalance"), ColorTool, parent)
{
    m_settingsView = 0;

    setToolTitle(i18n("Color Balance"));
    setToolDescription(i18n("Adjust color balance."));
    setToolIconName(QLatin1String("adjustrgb"));
}

ColorBalance::~ColorBalance()
{
}

void ColorBalance::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new CBSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ColorBalance::defaultSettings()
{
    BatchToolSettings prm;
    CBContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Red"), (double)defaultPrm.red);
    prm.insert(QLatin1String("Green"), (double)defaultPrm.green);
    prm.insert(QLatin1String("Blue"), (double)defaultPrm.blue);

    return prm;
}

void ColorBalance::slotAssignSettings2Widget()
{
    CBContainer prm;
    prm.red   = settings()[QLatin1String("Red")].toDouble();
    prm.green = settings()[QLatin1String("Green")].toDouble();
    prm.blue  = settings()[QLatin1String("Blue")].toDouble();
    m_settingsView->setSettings(prm);
}

void ColorBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    CBContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Red"), (double)currentPrm.red);
    prm.insert(QLatin1String("Green"), (double)currentPrm.green);
    prm.insert(QLatin1String("Blue"), (double)currentPrm.blue);

    BatchTool::slotSettingsChanged(prm);
}

bool ColorBalance::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    CBContainer prm;
    prm.red   = settings()[QLatin1String("Red")].toDouble();
    prm.green = settings()[QLatin1String("Green")].toDouble();
    prm.blue  = settings()[QLatin1String("Blue")].toDouble();

    CBFilter cb(&image(), 0L, prm);
    applyFilter(&cb);

    return (savefromDImg());
}

}  // namespace Digikam
