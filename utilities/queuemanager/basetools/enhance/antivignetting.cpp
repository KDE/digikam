/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "antivignetting.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "antivignettingfilter.h"
#include "antivignettingsettings.h"

namespace Digikam
{

AntiVignetting::AntiVignetting(QObject* const parent)
    : BatchTool("AntiVignetting", EnhanceTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("Anti-Vignetting"));
    setToolDescription(i18n("Remove/add vignetting to photograph."));
    setToolIconName("antivignetting");
}

AntiVignetting::~AntiVignetting()
{
}

void AntiVignetting::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new AntiVignettingSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AntiVignetting::defaultSettings()
{
    BatchToolSettings prm;
    AntiVignettingContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("addvignetting", (bool)defaultPrm.addvignetting);
    prm.insert("density", (double)defaultPrm.density);
    prm.insert("power", (double)defaultPrm.power);
    prm.insert("innerradius", (double)defaultPrm.innerradius);
    prm.insert("outerradius", (double)defaultPrm.outerradius);
    prm.insert("xshift", (double)defaultPrm.xshift);
    prm.insert("yshift", (double)defaultPrm.yshift);

    return prm;
}

void AntiVignetting::slotAssignSettings2Widget()
{
    AntiVignettingContainer prm;
    prm.addvignetting = settings()["addvignetting"].toBool();
    prm.density       = settings()["density"].toDouble();
    prm.power         = settings()["power"].toDouble();
    prm.innerradius   = settings()["innerradius"].toDouble();
    prm.outerradius   = settings()["outerradius"].toDouble();
    prm.xshift        = settings()["xshift"].toDouble();
    prm.yshift        = settings()["yshift"].toDouble();
    m_settingsView->setSettings(prm);
}

void AntiVignetting::slotSettingsChanged()
{
    BatchToolSettings prm;
    AntiVignettingContainer currentPrm = m_settingsView->settings();

    prm.insert("addvignetting", (bool)currentPrm.addvignetting);
    prm.insert("density", (double)currentPrm.density);
    prm.insert("power", (double)currentPrm.power);
    prm.insert("innerradius", (double)currentPrm.innerradius);
    prm.insert("outerradius", (double)currentPrm.outerradius);
    prm.insert("xshift", (double)currentPrm.xshift);
    prm.insert("yshift", (double)currentPrm.yshift);

    BatchTool::slotSettingsChanged(prm);
}

bool AntiVignetting::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    AntiVignettingContainer prm;
    prm.addvignetting = settings()["addvignetting"].toBool();
    prm.density       = settings()["density"].toDouble();
    prm.power         = settings()["power"].toDouble();
    prm.innerradius   = settings()["innerradius"].toDouble();
    prm.outerradius   = settings()["outerradius"].toDouble();
    prm.xshift        = settings()["xshift"].toDouble();
    prm.yshift        = settings()["yshift"].toDouble();

    AntiVignettingFilter vig(&image(), 0L, prm);
    applyFilter(&vig);

    return (savefromDImg());
}

}  // namespace Digikam
