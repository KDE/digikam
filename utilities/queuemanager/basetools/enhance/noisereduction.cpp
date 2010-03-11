/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "nrfilter.h"
#include "nrsettings.h"

namespace Digikam
{

NoiseReduction::NoiseReduction(QObject* parent)
              : BatchTool("NoiseReduction", EnhanceTool, parent)
{
    setToolTitle(i18n("Noise Reduction"));
    setToolDescription(i18n("A tool to remove photograph noise using wavelets."));
    setToolIcon(KIcon(SmallIcon("noisereduction")));

    QWidget* box   = new QWidget;
    m_settingsView = new NRSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

NoiseReduction::~NoiseReduction()
{
}

BatchToolSettings NoiseReduction::defaultSettings()
{
    BatchToolSettings prm;
    NRContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("LeadThreshold", (double)defaultPrm.leadThreshold);
    prm.insert("LeadSoftness",  (double)defaultPrm.leadSoftness);
    prm.insert("Advanced",      (bool)defaultPrm.advanced);
    prm.insert("YThreshold",    (double)defaultPrm.thresholds[0]);
    prm.insert("CrThreshold",   (double)defaultPrm.thresholds[1]);
    prm.insert("CbThreshold",   (double)defaultPrm.thresholds[2]);
    prm.insert("YSoftness",     (double)defaultPrm.softness[0]);
    prm.insert("CrSoftness",    (double)defaultPrm.softness[1]);
    prm.insert("CbSoftness",    (double)defaultPrm.softness[2]);

    return prm;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    NRContainer prm;
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
    BatchToolSettings prm;
    NRContainer currentPrm = m_settingsView->settings();

    prm.insert("LeadThreshold", (double)currentPrm.leadThreshold);
    prm.insert("LeadSoftness",  (double)currentPrm.leadSoftness);
    prm.insert("Advanced",      (bool)currentPrm.advanced);
    prm.insert("YThreshold",    (double)currentPrm.thresholds[0]);
    prm.insert("CrThreshold",   (double)currentPrm.thresholds[1]);
    prm.insert("CbThreshold",   (double)currentPrm.thresholds[2]);
    prm.insert("YSoftness",     (double)currentPrm.softness[0]);
    prm.insert("CrSoftness",    (double)currentPrm.softness[1]);
    prm.insert("CbSoftness",    (double)currentPrm.softness[2]);

    setSettings(prm);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg()) return false;

    NRContainer prm;
    prm.leadThreshold = settings()["LeadThreshold"].toDouble();
    prm.leadSoftness  = settings()["LeadSoftness"].toDouble();
    prm.advanced      = settings()["Advanced"].toBool();
    prm.thresholds[0] = settings()["YThreshold"].toDouble();
    prm.thresholds[1] = settings()["CrThreshold"].toDouble();
    prm.thresholds[2] = settings()["CbThreshold"].toDouble();
    prm.softness[0]   = settings()["YSoftness"].toDouble();
    prm.softness[1]   = settings()["CrSoftness"].toDouble();
    prm.softness[2]   = settings()["CbSoftness"].toDouble();

    NRFilter wnr(&image(), 0L, prm);
    wnr.startFilterDirectly();
    DImg trg = wnr.getTargetImage();
    image().putImageData(trg.bits());

    return (savefromDImg());
}

}  // namespace Digikam
