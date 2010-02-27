/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-27
 * Description : White Balance batch tool.
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

#include "whitebalance.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "wbfilter.h"
#include "wbsettings.h"

namespace Digikam
{

WhiteBalance::WhiteBalance(QObject* parent)
            : BatchTool("WhiteBalance", BaseTool, parent)
{
    setToolTitle(i18n("White Balance"));
    setToolDescription(i18n("A tool to adjust White Balance."));
    setToolIcon(KIcon(SmallIcon("whitebalance")));

    QWidget *box   = new QWidget;
    m_settingsView = new WBSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

WhiteBalance::~WhiteBalance()
{
}

BatchToolSettings WhiteBalance::defaultSettings()
{
    BatchToolSettings prm;
    WBContainer defaultPrm = m_settingsView->defaultSettings();
/*
    prm.insert("Brightness", (double)defaultPrm.brightness);
    prm.insert("Contrast",   (double)defaultPrm.contrast);
    prm.insert("Gamma",      (double)defaultPrm.gamma);
*/
    return prm;
}

void WhiteBalance::slotAssignSettings2Widget()
{
    WBContainer prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    m_settingsView->setSettings(prm);
}

void WhiteBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    WBContainer currentPrm = m_settingsView->settings();
/*
    prm.insert("Brightness", (double)currentPrm.brightness);
    prm.insert("Contrast",   (double)currentPrm.contrast);
    prm.insert("Gamma",      (double)currentPrm.gamma);
*/
    setSettings(prm);
}

bool WhiteBalance::toolOperations()
{
    if (!loadToDImg()) return false;

    WBContainer prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    WBFilter bcg(&image(), 0L, prm);
    bcg.startFilterDirectly();
    image().putImageData(bcg.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
