/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-02-06
 * Description : setup RAW decoding settings.
 * 
 * Copyright 2007 by Gilles Caulier
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

// QT includes.

#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

#include "setupdcraw.h"
#include "setupdcraw.moc"

namespace Digikam
{

class SetupDcrawPriv
{
public:


    SetupDcrawPriv()
    {
        dcrawSettings = 0;
    }

    KDcrawIface::DcrawSettingsWidget *dcrawSettings;
};

SetupDcraw::SetupDcraw(QWidget* parent )
          : QWidget(parent)
{
    d = new SetupDcrawPriv;
    QVBoxLayout *layout = new QVBoxLayout(parent, 0, KDialog::spacingHint());
    d->dcrawSettings    = new KDcrawIface::DcrawSettingsWidget(parent, true, false);
    layout->addWidget(d->dcrawSettings);
    layout->addStretch();

    readSettings();
}

SetupDcraw::~SetupDcraw()
{
    delete d;
}

void SetupDcraw::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("SixteenBitsImage", d->dcrawSettings->sixteenBits());
    config->writeEntry("CameraColorBalance", d->dcrawSettings->useCameraWB());
    config->writeEntry("AutomaticColorBalance", d->dcrawSettings->useAutoColorBalance());
    config->writeEntry("RGBInterpolate4Colors", d->dcrawSettings->useFourColor());
    config->writeEntry("DontStretchPixels", d->dcrawSettings->useDontStretchPixels());
    config->writeEntry("EnableNoiseReduction", d->dcrawSettings->useNoiseReduction());
    config->writeEntry("NRThreshold", d->dcrawSettings->NRThreshold());
    config->writeEntry("UnclipColors", d->dcrawSettings->unclipColor());
    config->writeEntry("RAWBrightness", d->dcrawSettings->brightness());
    config->writeEntry("RAWQuality", d->dcrawSettings->quality());
    config->sync();
}

void SetupDcraw::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    d->dcrawSettings->setSixteenBits(config->readBoolEntry("SixteenBitsImage", false));
    d->dcrawSettings->setNoiseReduction(config->readBoolEntry("EnableNoiseReduction", false));
    d->dcrawSettings->setNRThreshold(config->readNumEntry("NRThreshold", 100));
    d->dcrawSettings->setDontStretchPixels(config->readBoolEntry("DontStretchPixels", false));
    d->dcrawSettings->setUnclipColor(config->readNumEntry("UnclipColors", 0));
    d->dcrawSettings->setCameraWB(config->readBoolEntry("CameraColorBalance", true));
    d->dcrawSettings->setAutoColorBalance(config->readBoolEntry("AutomaticColorBalance", true));
    d->dcrawSettings->setFourColor(config->readBoolEntry("RGBInterpolate4Colors", false));
    d->dcrawSettings->setQuality((KDcrawIface::RawDecodingSettings::DecodingQuality)
                                  config->readNumEntry("RAWQuality",
                                  KDcrawIface::RawDecodingSettings::BILINEAR));
    d->dcrawSettings->setBrightness(config->readDoubleNumEntry("RAWBrightness", 1.0));
}

}  // namespace Digikam
