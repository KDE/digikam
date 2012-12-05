/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : LDR tonemapper batch tool.
 *
 * Copyright (C) 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrast.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "localcontrastfilter.h"
#include "localcontrastsettings.h"
#include "localcontrastcontainer.h"

namespace Digikam
{

LocalContrast::LocalContrast(QObject* const parent)
    : BatchTool("LocalContrast", EnhanceTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("Local Contrast"));
    setToolDescription(i18n("Emulate tone mapping."));
    setToolIconName("tonemap");
}

LocalContrast::~LocalContrast()
{
}

void LocalContrast::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new LocalContrastSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings LocalContrast::defaultSettings()
{
    BatchToolSettings prm;
    LocalContrastContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("stretchContrast", defaultPrm.stretchContrast);
    prm.insert("lowSaturation",   defaultPrm.lowSaturation);
    prm.insert("highSaturation",  defaultPrm.highSaturation);
    prm.insert("functionId",      defaultPrm.functionId);

    prm.insert("stage1Enabled",   defaultPrm.stage[0].enabled);
    prm.insert("stage1Power",     defaultPrm.stage[0].power);
    prm.insert("stage1Blur",      defaultPrm.stage[0].blur);

    prm.insert("stage2Enabled",   defaultPrm.stage[1].enabled);
    prm.insert("stage2Power",     defaultPrm.stage[1].power);
    prm.insert("stage2Blur",      defaultPrm.stage[1].blur);

    prm.insert("stage3Enabled",   defaultPrm.stage[2].enabled);
    prm.insert("stage3Power",     defaultPrm.stage[2].power);
    prm.insert("stage3Blur",      defaultPrm.stage[2].blur);

    prm.insert("stage4Enabled",   defaultPrm.stage[3].enabled);
    prm.insert("stage4Power",     defaultPrm.stage[3].power);
    prm.insert("stage4Blur",      defaultPrm.stage[3].blur);

    return prm;
}

void LocalContrast::slotAssignSettings2Widget()
{
    LocalContrastContainer prm;

    prm.stretchContrast = settings()["stretchContrast"].toBool();
    prm.lowSaturation   = settings()["lowSaturation"].toInt();
    prm.highSaturation  = settings()["highSaturation"].toInt();
    prm.functionId      = settings()["functionId"].toInt();

    prm.stage[0].enabled = settings()["stage1Enabled"].toBool();
    prm.stage[0].power   = settings()["stage1Power"].toDouble();
    prm.stage[0].blur    = settings()["stage1Blur"].toDouble();

    prm.stage[1].enabled = settings()["stage2Enabled"].toBool();
    prm.stage[1].power   = settings()["stage2Power"].toDouble();
    prm.stage[1].blur    = settings()["stage2Blur"].toDouble();

    prm.stage[2].enabled = settings()["stage3Enabled"].toBool();
    prm.stage[2].power   = settings()["stage3Power"].toDouble();
    prm.stage[2].blur    = settings()["stage3Blur"].toDouble();

    prm.stage[3].enabled = settings()["stage4Enabled"].toBool();
    prm.stage[3].power   = settings()["stage4Power"].toDouble();
    prm.stage[3].blur    = settings()["stage4Blur"].toDouble();

    m_settingsView->setSettings(prm);
}

void LocalContrast::slotSettingsChanged()
{
    BatchToolSettings prm;
    LocalContrastContainer currentPrm = m_settingsView->settings();

    prm.insert("stretchContrast", currentPrm.stretchContrast);
    prm.insert("lowSaturation",   currentPrm.lowSaturation);
    prm.insert("highSaturation",  currentPrm.highSaturation);
    prm.insert("functionId",      currentPrm.functionId);

    prm.insert("stage1Enabled",   currentPrm.stage[0].enabled);
    prm.insert("stage1Power",     currentPrm.stage[0].power);
    prm.insert("stage1Blur",      currentPrm.stage[0].blur);

    prm.insert("stage2Enabled",   currentPrm.stage[1].enabled);
    prm.insert("stage2Power",     currentPrm.stage[1].power);
    prm.insert("stage2Blur",      currentPrm.stage[1].blur);

    prm.insert("stage3Enabled",   currentPrm.stage[2].enabled);
    prm.insert("stage3Power",     currentPrm.stage[2].power);
    prm.insert("stage3Blur",      currentPrm.stage[2].blur);

    prm.insert("stage4Enabled",   currentPrm.stage[3].enabled);
    prm.insert("stage4Power",     currentPrm.stage[3].power);
    prm.insert("stage4Blur",      currentPrm.stage[3].blur);

    BatchTool::slotSettingsChanged(prm);
}

bool LocalContrast::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    LocalContrastContainer prm;

    prm.stretchContrast = settings()["stretchContrast"].toBool();
    prm.lowSaturation   = settings()["lowSaturation"].toInt();
    prm.highSaturation  = settings()["highSaturation"].toInt();
    prm.functionId      = settings()["functionId"].toInt();

    prm.stage[0].enabled = settings()["stage1Enabled"].toBool();
    prm.stage[0].power   = settings()["stage1Power"].toDouble();
    prm.stage[0].blur    = settings()["stage1Blur"].toDouble();

    prm.stage[1].enabled = settings()["stage2Enabled"].toBool();
    prm.stage[1].power   = settings()["stage2Power"].toDouble();
    prm.stage[1].blur    = settings()["stage2Blur"].toDouble();

    prm.stage[2].enabled = settings()["stage3Enabled"].toBool();
    prm.stage[2].power   = settings()["stage3Power"].toDouble();
    prm.stage[2].blur    = settings()["stage3Blur"].toDouble();

    prm.stage[3].enabled = settings()["stage4Enabled"].toBool();
    prm.stage[3].power   = settings()["stage4Power"].toDouble();
    prm.stage[3].blur    = settings()["stage4Blur"].toDouble();

    LocalContrastFilter lc(&image(), 0L, prm);
    applyFilter(&lc);

    return (savefromDImg());
}

}  // namespace Digikam
