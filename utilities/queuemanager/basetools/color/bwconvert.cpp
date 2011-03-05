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
    : BatchTool("BWConvert", ColorTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("B&W Convert"));
    setToolDescription(i18n("A tool to convert to black and white."));
    setToolIcon(KIcon(SmallIcon("bwtonal")));
}

BWConvert::~BWConvert()
{
}

QWidget* BWConvert::createSettingsWidget()
{
    QWidget* box   = new QWidget;
    m_settingsView = new BWSepiaSettings(box, &m_preview);

    m_settingsView->startPreviewFilters();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    return box;
}

void BWConvert::slotResetSettingsToDefault()
{
    // We need to call this method there to reset all curves points.
    // Curves values are cleaned with default settings passed after.
    m_settingsView->resetToDefault();
    BatchTool::slotResetSettingsToDefault();
}

BatchToolSettings BWConvert::defaultSettings()
{
    BatchToolSettings prm;
    BWSepiaContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("filmType",   (int)defaultPrm.filmType);
    prm.insert("filterType", (int)defaultPrm.filterType);
    prm.insert("toneType",   (int)defaultPrm.toneType);
    prm.insert("contrast",   (double)defaultPrm.bcgPrm.contrast);
    prm.insert("strength",   (double)defaultPrm.strength);
    prm.insert("curves",     defaultPrm.curvesPrm.values[LuminosityChannel]);

    return prm;
}

void BWConvert::slotAssignSettings2Widget()
{
    BWSepiaContainer prm;

    prm.filmType               = settings()["filmType"].toInt();
    prm.filterType             = settings()["filterType"].toInt();
    prm.toneType               = settings()["toneType"].toInt();
    prm.bcgPrm.contrast        = settings()["contrast"].toDouble();
    prm.strength               = settings()["strength"].toDouble();
    prm.curvesPrm.values[LuminosityChannel] = settings()["curves"].value<QPolygon>();

    m_settingsView->setSettings(prm);
}

void BWConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    BWSepiaContainer currentPrm = m_settingsView->settings();

    prm.insert("filmType",   (int)currentPrm.filmType);
    prm.insert("filterType", (int)currentPrm.filterType);
    prm.insert("toneType",   (int)currentPrm.toneType);
    prm.insert("contrast",   (double)currentPrm.bcgPrm.contrast);
    prm.insert("strength",   (double)currentPrm.strength);
    prm.insert("curves",     currentPrm.curvesPrm.values[LuminosityChannel]);

    BatchTool::slotSettingsChanged(prm);
}

bool BWConvert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BWSepiaContainer prm;

    prm.filmType               = settings()["filmType"].toInt();
    prm.filterType             = settings()["filterType"].toInt();
    prm.toneType               = settings()["toneType"].toInt();
    prm.bcgPrm.contrast        = settings()["contrast"].toDouble();
    prm.strength               = settings()["strength"].toDouble();
    prm.curvesPrm.values[LuminosityChannel] = settings()["curves"].value<QPolygon>();

    BWSepiaFilter bw(&image(), 0L, prm);
    bw.startFilterDirectly();
    image().putImageData(bw.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
