/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * Copyright (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupdcraw.moc"

// Qt includes

#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/dcrawsettingswidget.h>
#include <libkdcraw/version.h>

// Local includes

#include "drawdecoding.h"

using namespace KDcrawIface;

namespace Digikam
{

class SetupDcrawPriv
{
public:


    SetupDcrawPriv() :
        configGroupName("ImageViewer Settings"), 

        dcrawSettings(0)
    {}

    const QString        configGroupName; 

    DcrawSettingsWidget* dcrawSettings;
};

SetupDcraw::SetupDcraw(QWidget* parent)
          : QScrollArea(parent), d(new SetupDcrawPriv)
{
    QWidget *panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QGridLayout *layout = new QGridLayout(panel);
    d->dcrawSettings    = new DcrawSettingsWidget(panel, DcrawSettingsWidget::SIXTEENBITS);
    d->dcrawSettings->setItemIcon(0, SmallIcon("kdcraw"));
    d->dcrawSettings->setItemIcon(1, SmallIcon("whitebalance"));
    d->dcrawSettings->setItemIcon(2, SmallIcon("lensdistortion"));
    layout->addWidget(d->dcrawSettings, 0, 0);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setRowStretch(0, 10);

    connect(d->dcrawSettings, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupDcraw::~SetupDcraw()
{
    delete d;
}

void SetupDcraw::slotSixteenBitsImageToggled(bool)
{
    // Dcraw do not provide a way to set brightness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->dcrawSettings->setEnabledBrightnessSettings(true);
}

void SetupDcraw::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->dcrawSettings->writeSettings(group);
    config->sync();
}

void SetupDcraw::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->dcrawSettings->readSettings(group);
}

}  // namespace Digikam
