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
    settings.insert("NRThreshold", (double)m_settingsView->thresholdInput()->defaultValue());
    settings.insert("NRSoftness",  (double)m_settingsView->softnessInput()->defaultValue());
    return settings;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    m_settingsView->thresholdInput()->setValue(settings()["NRThreshold"].toDouble());
    m_settingsView->softnessInput()->setValue(settings()["NRSoftness"].toDouble());
}

void NoiseReduction::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("NRThreshold", (double)m_settingsView->thresholdInput()->value());
    settings.insert("NRSoftness",  (double)m_settingsView->softnessInput()->value());
    setSettings(settings);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg()) return false;

    WaveletsNRContainer prm;
    prm.thresholds[0] = settings()["NRThreshold"].toDouble();
    prm.thresholds[1] = settings()["NRThreshold"].toDouble();
    prm.thresholds[2] = settings()["NRThreshold"].toDouble();
    prm.softness[0]   = settings()["NRSoftness"].toDouble();
    prm.softness[1]   = settings()["NRSoftness"].toDouble();
    prm.softness[2]   = settings()["NRSoftness"].toDouble();

    WaveletsNR wnr(&image(), 0L, prm);
    wnr.startFilterDirectly();
    DImg trg = wnr.getTargetImage();
    image().putImageData(trg.bits());

    return (savefromDImg());
}

}  // namespace Digikam
