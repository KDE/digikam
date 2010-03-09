/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : LDR tonemapper batch tool.
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
#include "tonemappingparameters.h"

namespace Digikam
{

LocalContrast::LocalContrast(QObject* parent)
             : BatchTool("LocalContrast", BaseTool, parent)
{
    setToolTitle(i18n("Local Contrast"));
    setToolDescription(i18n("A tool to emulate tone mapping."));
    setToolIcon(KIcon(SmallIcon("contrast")));

    QWidget *box   = new QWidget;
    m_settingsView = new LocalContrastSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

LocalContrast::~LocalContrast()
{
}

BatchToolSettings LocalContrast::defaultSettings()
{
    BatchToolSettings prm;
    ToneMappingParameters defaultPrm = m_settingsView->defaultSettings();
/*
    prm.insert("Brightness", (double)defaultPrm.brightness);
    prm.insert("Contrast",   (double)defaultPrm.contrast);
    prm.insert("Gamma",      (double)defaultPrm.gamma);
*/
    return prm;
}

void LocalContrast::slotAssignSettings2Widget()
{
    ToneMappingParameters prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    m_settingsView->setSettings(prm);
}

void LocalContrast::slotSettingsChanged()
{
    BatchToolSettings prm;
    ToneMappingParameters currentPrm = m_settingsView->settings();
/*
    prm.insert("Brightness", (double)currentPrm.brightness);
    prm.insert("Contrast",   (double)currentPrm.contrast);
    prm.insert("Gamma",      (double)currentPrm.gamma);
*/
    setSettings(prm);
}

bool LocalContrast::toolOperations()
{
    if (!loadToDImg()) return false;

    ToneMappingParameters prm;
/*
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    LocalContrastFilter lc(&image(), 0L, prm);
    lc.startFilterDirectly();
    image().putImageData(lc.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
