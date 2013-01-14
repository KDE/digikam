/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : a batch tool to apply color effects to images.
 *
 * Copyright (C) 2012 by Alexander Dymo <adymo at develop dot org>
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

#include "colorfx.moc"

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "colorfxfilter.h"
#include "colorfxsettings.h"

namespace Digikam
{

ColorFX::ColorFX(QObject* const parent)
    : BatchTool("ColorFX", FiltersTool, parent)
{
    m_settingsView = 0;

    setToolTitle(i18n("Color Effects"));
    setToolDescription(i18n("Apply color effects"));
    setToolIconName("colorfx");
}

ColorFX::~ColorFX()
{
}

void ColorFX::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new ColorFXSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(m_settingsView, SIGNAL(signalLevelOrIterationChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ColorFX::defaultSettings()
{
    BatchToolSettings prm;
    ColorFXContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("colorFXType", (int)defaultPrm.colorFXType);
    prm.insert("level",       (int)defaultPrm.level);
    prm.insert("iterations",  (int)defaultPrm.iterations);

    return prm;
}

void ColorFX::slotAssignSettings2Widget()
{
    ColorFXContainer prm;
    prm.colorFXType = settings()["colorFXType"].toInt();
    prm.level       = settings()["level"].toInt();
    prm.iterations  = settings()["iterations"].toInt();
    m_settingsView->setSettings(prm);
}

void ColorFX::slotSettingsChanged()
{
    BatchToolSettings prm;
    ColorFXContainer currentPrm = m_settingsView->settings();

    prm.insert("colorFXType", (int)currentPrm.colorFXType);
    prm.insert("level",       (int)currentPrm.level);
    prm.insert("iterations",  (int)currentPrm.iterations);

    BatchTool::slotSettingsChanged(prm);
}

bool ColorFX::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    ColorFXContainer prm;
    prm.colorFXType = settings()["colorFXType"].toInt();
    prm.level       = settings()["level"].toInt();
    prm.iterations  = settings()["iterations"].toInt();

    ColorFXFilter fg(&image(), 0L, prm);
    applyFilter(&fg);

    return savefromDImg();
}

} // namespace Digikam
