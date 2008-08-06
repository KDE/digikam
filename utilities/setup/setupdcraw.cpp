/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <libkdcraw/version.h>
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

    connect(d->dcrawSettings, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    readSettings();
}

SetupDcraw::~SetupDcraw()
{
    delete d;
}

void SetupDcraw::slotSixteenBitsImageToggled(bool)
{
#if KDCRAW_VERSION >= 0x000105
    // Dcraw do not provide a way to set brigness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->dcrawSettings->setEnabledBrightnessSettings(true);
#endif
}

void SetupDcraw::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("SixteenBitsImage",        d->dcrawSettings->sixteenBits());
    config->writeEntry("WhiteBalance",            d->dcrawSettings->whiteBalance());
    config->writeEntry("CustomWhiteBalance",      d->dcrawSettings->customWhiteBalance());
    config->writeEntry("CustomWhiteBalanceGreen", d->dcrawSettings->customWhiteBalanceGreen());
    config->writeEntry("RGBInterpolate4Colors",   d->dcrawSettings->useFourColor());
    config->writeEntry("DontStretchPixels",       d->dcrawSettings->useDontStretchPixels());
    config->writeEntry("EnableNoiseReduction",    d->dcrawSettings->useNoiseReduction());
    config->writeEntry("NRThreshold",             d->dcrawSettings->NRThreshold());
    config->writeEntry("EnableCACorrection",      d->dcrawSettings->useCACorrection());
    config->writeEntry("caRedMultiplier",         d->dcrawSettings->caRedMultiplier());
    config->writeEntry("caBlueMultiplier",        d->dcrawSettings->caBlueMultiplier());
    config->writeEntry("UnclipColors",            d->dcrawSettings->unclipColor());
    config->writeEntry("RAWBrightness",           d->dcrawSettings->brightness());
    config->writeEntry("RAWQuality",              d->dcrawSettings->quality());
    config->writeEntry("EnableBlackPoint",        d->dcrawSettings->useBlackPoint());
    config->writeEntry("BlackPoint",              d->dcrawSettings->blackPoint());
#if KDCRAW_VERSION >= 0x000105
    config->writeEntry("EnableWhitePoint",        d->dcrawSettings->useWhitePoint());
    config->writeEntry("WhitePoint",              d->dcrawSettings->whitePoint());
#endif
    config->sync();
}

void SetupDcraw::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    d->dcrawSettings->setSixteenBits(config->readBoolEntry("SixteenBitsImage", false));
    d->dcrawSettings->setNoiseReduction(config->readBoolEntry("EnableNoiseReduction", false));
    d->dcrawSettings->setNRThreshold(config->readNumEntry("NRThreshold", 100));
    d->dcrawSettings->setUseCACorrection(config->readBoolEntry("EnableCACorrection", false));
    d->dcrawSettings->setcaRedMultiplier(config->readDoubleNumEntry("caRedMultiplier", 1.0));
    d->dcrawSettings->setcaBlueMultiplier(config->readDoubleNumEntry("caBlueMultiplier", 1.0));
    d->dcrawSettings->setDontStretchPixels(config->readBoolEntry("DontStretchPixels", false));
    d->dcrawSettings->setUnclipColor(config->readNumEntry("UnclipColors", 0));
    d->dcrawSettings->setWhiteBalance((KDcrawIface::RawDecodingSettings::WhiteBalance)
                                      config->readNumEntry("WhiteBalance",
                                      KDcrawIface::RawDecodingSettings::CAMERA));
    d->dcrawSettings->setCustomWhiteBalance(config->readNumEntry("CustomWhiteBalance", 6500));
    d->dcrawSettings->setCustomWhiteBalanceGreen(config->readDoubleNumEntry("CustomWhiteBalanceGreen", 1.0));
    d->dcrawSettings->setFourColor(config->readBoolEntry("RGBInterpolate4Colors", false));
    d->dcrawSettings->setQuality((KDcrawIface::RawDecodingSettings::DecodingQuality)
                                  config->readNumEntry("RAWQuality",
                                  KDcrawIface::RawDecodingSettings::BILINEAR));
    d->dcrawSettings->setBrightness(config->readDoubleNumEntry("RAWBrightness", 1.0));
    d->dcrawSettings->setUseBlackPoint(config->readBoolEntry("EnableBlackPoint", false));
    d->dcrawSettings->setBlackPoint(config->readNumEntry("BlackPoint", 0));
#if KDCRAW_VERSION >= 0x000105
    d->dcrawSettings->setUseWhitePoint(config->readBoolEntry("EnableWhitePoint", false));
    d->dcrawSettings->setWhitePoint(config->readNumEntry("WhitePoint", 0));
#endif
}

}  // namespace Digikam
