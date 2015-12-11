/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#include "lut3d.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "lut3dsettings.h"
#include "lut3dfilter.h"
#include "dimg.h"

namespace Digikam
{

Lut3D::Lut3D(QObject* const parent)
    : BatchTool(QLatin1String("Lut3D"), ColorTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("Lut3D Colors Adjust"));
    setToolDescription(i18n("Change image colors using Lut3D"));
    setToolIconName(QLatin1String("draw-cuboid"));
}

Lut3D::~Lut3D()
{
}

void Lut3D::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new Lut3DSettings(m_settingsWidget, true);
    m_settingsView->startPreviewFilters();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

void Lut3D::slotResetSettingsToDefault()
{
    // We need to call this method there to reset all curves points.
    // Curves values are cleaned with default settings passed after.
    m_settingsView->resetToDefault();
    BatchTool::slotResetSettingsToDefault();
}

BatchToolSettings Lut3D::defaultSettings()
{
    BatchToolSettings prm;
    Lut3DContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("path"),      defaultPrm.path);
    prm.insert(QLatin1String("intensity"), (int)defaultPrm.intensity);

    return prm;
}

void Lut3D::slotAssignSettings2Widget()
{
    Lut3DContainer prm;

    prm.path      = settings()[QLatin1String("path")].toString();
    prm.intensity = settings()[QLatin1String("intensity")].toInt();

    m_settingsView->setSettings(prm);
}

void Lut3D::slotSettingsChanged()
{
    BatchToolSettings prm;
    Lut3DContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("path"),      currentPrm.path);
    prm.insert(QLatin1String("intensity"), (int)currentPrm.intensity);

    BatchTool::slotSettingsChanged(prm);
}

bool Lut3D::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    Lut3DContainer prm;

    prm.path      = settings()[QLatin1String("path")].toString();
    prm.intensity = settings()[QLatin1String("intensity")].toInt();

    Lut3DFilter lut3d(&image(), prm);
    applyFilter(&lut3d);

    return (savefromDImg());
}

}  // namespace Digikam
