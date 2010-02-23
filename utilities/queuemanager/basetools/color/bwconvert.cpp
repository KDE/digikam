/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : Black and White conversion batch tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwconvert.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "bwsepiafilter.h"
#include "bwsepiasettings.h"

namespace Digikam
{

BWConvert::BWConvert(QObject* parent)
         : BatchTool("BWConvert", BaseTool, parent)
{
    setToolTitle(i18n("B&W Convert"));
    setToolDescription(i18n("A tool to convert to black and white."));
    setToolIcon(KIcon(SmallIcon("bwtonal")));

    QWidget* box   = new QWidget;
    m_preview      = DImg();
    m_settingsView = new BWSepiaSettings(box, &m_preview);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

BWConvert::~BWConvert()
{
}

BatchToolSettings BWConvert::defaultSettings()
{
    BatchToolSettings prm;
    BWSepiaContainer defaultPrm = m_settingsView->defaultSettings();
/*
    prm.insert("Brightness", (double)defaultPrm.brightness);
    prm.insert("Contrast",   (double)defaultPrm.contrast);
    prm.insert("Gamma",      (double)defaultPrm.gamma);
*/
    return prm;
}

void BWConvert::slotAssignSettings2Widget()
{
    BWSepiaContainer prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    m_settingsView->setSettings(prm);
}

void BWConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    BWSepiaContainer currentPrm = m_settingsView->settings();
/*
    prm.insert("Brightness", (double)currentPrm.brightness);
    prm.insert("Contrast",   (double)currentPrm.contrast);
    prm.insert("Gamma",      (double)currentPrm.gamma);
*/
    setSettings(prm);
}

bool BWConvert::toolOperations()
{
    if (!loadToDImg()) return false;

    BWSepiaContainer prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    BWSepiaFilter bw(&image(), 0L, prm);
    bw.startFilterDirectly();
    image().putImageData(bw.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
