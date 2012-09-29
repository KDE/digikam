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

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

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
    setToolIcon(KIcon(SmallIcon("tonemap")));
}

LocalContrast::~LocalContrast()
{
}

QWidget* LocalContrast::createSettingsWidget()
{
    QWidget* box   = new QWidget;
    m_settingsView = new LocalContrastSettings(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    return box;
}

BatchToolSettings LocalContrast::defaultSettings()
{
    BatchToolSettings prm;
    LocalContrastContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("stretch_contrast", defaultPrm.stretch_contrast);
    prm.insert("low_saturation",   defaultPrm.low_saturation);
    prm.insert("high_saturation",  defaultPrm.high_saturation);
    prm.insert("function_id",      defaultPrm.function_id);

    prm.insert("stage1_enabled",   defaultPrm.stage[0].enabled);
    prm.insert("stage1_power",     defaultPrm.stage[0].power);
    prm.insert("stage1_blur",      defaultPrm.stage[0].blur);

    prm.insert("stage2_enabled",   defaultPrm.stage[1].enabled);
    prm.insert("stage2_power",     defaultPrm.stage[1].power);
    prm.insert("stage2_blur",      defaultPrm.stage[1].blur);

    prm.insert("stage3_enabled",   defaultPrm.stage[2].enabled);
    prm.insert("stage3_power",     defaultPrm.stage[2].power);
    prm.insert("stage3_blur",      defaultPrm.stage[2].blur);

    prm.insert("stage4_enabled",   defaultPrm.stage[3].enabled);
    prm.insert("stage4_power",     defaultPrm.stage[3].power);
    prm.insert("stage4_blur",      defaultPrm.stage[3].blur);

    return prm;
}

void LocalContrast::slotAssignSettings2Widget()
{
    LocalContrastContainer prm;

    prm.stretch_contrast = settings()["stretch_contrast"].toBool();
    prm.low_saturation   = settings()["low_saturation"].toInt();
    prm.high_saturation  = settings()["high_saturation"].toInt();
    prm.function_id      = settings()["function_id"].toInt();

    prm.stage[0].enabled = settings()["stage1_enabled"].toBool();
    prm.stage[0].power   = settings()["stage1_power"].toDouble();
    prm.stage[0].blur    = settings()["stage1_blur"].toDouble();

    prm.stage[1].enabled = settings()["stage2_enabled"].toBool();
    prm.stage[1].power   = settings()["stage2_power"].toDouble();
    prm.stage[1].blur    = settings()["stage2_blur"].toDouble();

    prm.stage[2].enabled = settings()["stage3_enabled"].toBool();
    prm.stage[2].power   = settings()["stage3_power"].toDouble();
    prm.stage[2].blur    = settings()["stage3_blur"].toDouble();

    prm.stage[3].enabled = settings()["stage4_enabled"].toBool();
    prm.stage[3].power   = settings()["stage4_power"].toDouble();
    prm.stage[3].blur    = settings()["stage4_blur"].toDouble();

    m_settingsView->setSettings(prm);
}

void LocalContrast::slotSettingsChanged()
{
    BatchToolSettings prm;
    LocalContrastContainer currentPrm = m_settingsView->settings();

    prm.insert("stretch_contrast", currentPrm.stretch_contrast);
    prm.insert("low_saturation",   currentPrm.low_saturation);
    prm.insert("high_saturation",  currentPrm.high_saturation);
    prm.insert("function_id",      currentPrm.function_id);

    prm.insert("stage1_enabled",   currentPrm.stage[0].enabled);
    prm.insert("stage1_power",     currentPrm.stage[0].power);
    prm.insert("stage1_blur",      currentPrm.stage[0].blur);

    prm.insert("stage2_enabled",   currentPrm.stage[1].enabled);
    prm.insert("stage2_power",     currentPrm.stage[1].power);
    prm.insert("stage2_blur",      currentPrm.stage[1].blur);

    prm.insert("stage3_enabled",   currentPrm.stage[2].enabled);
    prm.insert("stage3_power",     currentPrm.stage[2].power);
    prm.insert("stage3_blur",      currentPrm.stage[2].blur);

    prm.insert("stage4_enabled",   currentPrm.stage[3].enabled);
    prm.insert("stage4_power",     currentPrm.stage[3].power);
    prm.insert("stage4_blur",      currentPrm.stage[3].blur);

    BatchTool::slotSettingsChanged(prm);
}

bool LocalContrast::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    LocalContrastContainer prm;

    prm.stretch_contrast = settings()["stretch_contrast"].toBool();
    prm.low_saturation   = settings()["low_saturation"].toInt();
    prm.high_saturation  = settings()["high_saturation"].toInt();
    prm.function_id      = settings()["function_id"].toInt();

    prm.stage[0].enabled = settings()["stage1_enabled"].toBool();
    prm.stage[0].power   = settings()["stage1_power"].toDouble();
    prm.stage[0].blur    = settings()["stage1_blur"].toDouble();

    prm.stage[1].enabled = settings()["stage2_enabled"].toBool();
    prm.stage[1].power   = settings()["stage2_power"].toDouble();
    prm.stage[1].blur    = settings()["stage2_blur"].toDouble();

    prm.stage[2].enabled = settings()["stage3_enabled"].toBool();
    prm.stage[2].power   = settings()["stage3_power"].toDouble();
    prm.stage[2].blur    = settings()["stage3_blur"].toDouble();

    prm.stage[3].enabled = settings()["stage4_enabled"].toBool();
    prm.stage[3].power   = settings()["stage4_power"].toDouble();
    prm.stage[3].blur    = settings()["stage4_blur"].toDouble();

    LocalContrastFilter lc(&image(), 0L, prm);
    applyFilter(&lc);

    return (savefromDImg());
}

}  // namespace Digikam
