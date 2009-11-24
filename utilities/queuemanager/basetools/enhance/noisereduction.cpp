/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereduction.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurllabel.h>

// Local includes

#include "dimg.h"
#include "waveletsnr.h"
#include "noisereductionsettings.h"

namespace Digikam
{

NoiseReduction::NoiseReduction(QObject* parent)
              : BatchTool("NoiseReduction", BaseTool, parent)
{
    setToolTitle(i18n("Noise Reduction"));
    setToolDescription(i18n("A tool to remove photograph noise using wavelets."));
    setToolIcon(KIcon(SmallIcon("noisereduction")));

    QWidget *box   = new QWidget;
    m_settingsView = new NoiseReductionSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

NoiseReduction::~NoiseReduction()
{
}

BatchToolSettings NoiseReduction::defaultSettings()
{
    BatchToolSettings settings;
    WaveletsNRContainer prm = m_settingsView->defaultSettings();

    settings.insert("LeadThreshold", (double)prm.leadThreshold);
    settings.insert("LeadSoftness",  (double)prm.leadSoftness);
    settings.insert("Advanced",      (bool)prm.advanced);
    settings.insert("YThreshold",    (double)prm.thresholds[0]);
    settings.insert("CrThreshold",   (double)prm.thresholds[1]);
    settings.insert("CbThreshold",   (double)prm.thresholds[2]);
    settings.insert("YSoftness",     (double)prm.softness[0]);
    settings.insert("CrSoftness",    (double)prm.softness[1]);
    settings.insert("CbSoftness",    (double)prm.softness[2]);

    return settings;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    WaveletsNRContainer prm;
    prm.leadThreshold = settings()["LeadThreshold"].toDouble();
    prm.leadSoftness  = settings()["LeadSoftness"].toDouble();
    prm.advanced      = settings()["Advanced"].toBool();
    prm.thresholds[0] = settings()["YThreshold"].toDouble();
    prm.thresholds[1] = settings()["CrThreshold"].toDouble();
    prm.thresholds[2] = settings()["CbThreshold"].toDouble();
    prm.softness[0]   = settings()["YSoftness"].toDouble();
    prm.softness[1]   = settings()["CrSoftness"].toDouble();
    prm.softness[2]   = settings()["CbSoftness"].toDouble();
    m_settingsView->setSettings(prm);
}

void NoiseReduction::slotSettingsChanged()
{
    BatchToolSettings settings;
    WaveletsNRContainer prm = m_settingsView->settings();

    settings.insert("LeadThreshold", (double)prm.leadThreshold);
    settings.insert("LeadSoftness",  (double)prm.leadSoftness);
    settings.insert("Advanced",      (bool)prm.advanced);
    settings.insert("YThreshold",    (double)prm.thresholds[0]);
    settings.insert("CrThreshold",   (double)prm.thresholds[1]);
    settings.insert("CbThreshold",   (double)prm.thresholds[2]);
    settings.insert("YSoftness",     (double)prm.softness[0]);
    settings.insert("CrSoftness",    (double)prm.softness[1]);
    settings.insert("CbSoftness",    (double)prm.softness[2]);

    setSettings(settings);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg()) return false;

    WaveletsNRContainer prm;
    prm.leadThreshold = settings()["LeadThreshold"].toDouble();
    prm.leadSoftness  = settings()["LeadSoftness"].toDouble();
    prm.advanced      = settings()["Advanced"].toBool();
    prm.thresholds[0] = settings()["YThreshold"].toDouble();
    prm.thresholds[1] = settings()["CrThreshold"].toDouble();
    prm.thresholds[2] = settings()["CbThreshold"].toDouble();
    prm.softness[0]   = settings()["YSoftness"].toDouble();
    prm.softness[1]   = settings()["CrSoftness"].toDouble();
    prm.softness[2]   = settings()["CbSoftness"].toDouble();

    WaveletsNR wnr(&image(), 0L, prm);
    wnr.startFilterDirectly();
    DImg trg = wnr.getTargetImage();
    image().putImageData(trg.bits());

    return (savefromDImg());
}

}  // namespace Digikam
