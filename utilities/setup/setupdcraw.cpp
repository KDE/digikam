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

// Qt includes.

#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

#include "drawdecoding.h"
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
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    d->dcrawSettings    = new KDcrawIface::DcrawSettingsWidget(this, DcrawSettingsWidget::SIXTEENBITS);
    d->dcrawSettings->setItemIcon(0, SmallIcon("kdcraw"));
    d->dcrawSettings->setItemIcon(1, SmallIcon("whitebalance"));
    d->dcrawSettings->setItemIcon(2, SmallIcon("lensdistortion"));
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
#if KDCRAW_VERSION >= 0x000300
    // Dcraw do not provide a way to set brigness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->dcrawSettings->setEnabledBrightnessSettings(true);
#endif
}

void SetupDcraw::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    group.writeEntry("SixteenBitsImage",        d->dcrawSettings->sixteenBits());
    group.writeEntry("WhiteBalance",            (int)d->dcrawSettings->whiteBalance());
    group.writeEntry("CustomWhiteBalance",      d->dcrawSettings->customWhiteBalance());
    group.writeEntry("CustomWhiteBalanceGreen", d->dcrawSettings->customWhiteBalanceGreen());
    group.writeEntry("RGBInterpolate4Colors",   d->dcrawSettings->useFourColor());
    group.writeEntry("DontStretchPixels",       d->dcrawSettings->useDontStretchPixels());
    group.writeEntry("EnableNoiseReduction",    d->dcrawSettings->useNoiseReduction());
    group.writeEntry("NRThreshold",             d->dcrawSettings->NRThreshold());
    group.writeEntry("EnableCACorrection",      d->dcrawSettings->useCACorrection());
    group.writeEntry("caRedMultiplier",         d->dcrawSettings->caRedMultiplier());
    group.writeEntry("caBlueMultiplier",        d->dcrawSettings->caBlueMultiplier());
    group.writeEntry("UnclipColors",            d->dcrawSettings->unclipColor());
    group.writeEntry("RAWBrightness",           d->dcrawSettings->brightness());
    group.writeEntry("RAWQuality",              (int)d->dcrawSettings->quality());
    group.writeEntry("EnableBlackPoint",        d->dcrawSettings->useBlackPoint());
    group.writeEntry("BlackPoint",              d->dcrawSettings->blackPoint());
#if KDCRAW_VERSION >= 0x000300
    group.writeEntry("EnableWhitePoint",        d->dcrawSettings->useWhitePoint());
    group.writeEntry("WhitePoint",              d->dcrawSettings->whitePoint());
    group.writeEntry("MedianFilterPasses",      d->dcrawSettings->medianFilterPasses());
#endif
    config->sync();
}

void SetupDcraw::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    d->dcrawSettings->setSixteenBits(group.readEntry("SixteenBitsImage", false));
    d->dcrawSettings->setNoiseReduction(group.readEntry("EnableNoiseReduction", false));
    d->dcrawSettings->setNRThreshold(group.readEntry("NRThreshold", 100));
    d->dcrawSettings->setUseCACorrection(group.readEntry("EnableCACorrection", false));
    d->dcrawSettings->setcaRedMultiplier(group.readEntry("caRedMultiplier", 1.0));
    d->dcrawSettings->setcaBlueMultiplier(group.readEntry("caBlueMultiplier", 1.0));
    d->dcrawSettings->setDontStretchPixels(group.readEntry("DontStretchPixels", false));
    d->dcrawSettings->setUnclipColor(group.readEntry("UnclipColors", 0));
    d->dcrawSettings->setWhiteBalance((DRawDecoding::WhiteBalance)
                                      group.readEntry("WhiteBalance",
                                      (int)DRawDecoding::CAMERA));
    d->dcrawSettings->setCustomWhiteBalance(group.readEntry("CustomWhiteBalance", 6500));
    d->dcrawSettings->setCustomWhiteBalanceGreen(group.readEntry("CustomWhiteBalanceGreen", 1.0));
    d->dcrawSettings->setFourColor(group.readEntry("RGBInterpolate4Colors", false));
    d->dcrawSettings->setQuality((DRawDecoding::DecodingQuality)
                                  group.readEntry("RAWQuality",
                                  (int)DRawDecoding::BILINEAR));
    d->dcrawSettings->setBrightness(group.readEntry("RAWBrightness", 1.0));
    d->dcrawSettings->setUseBlackPoint(group.readEntry("EnableBlackPoint", false));
    d->dcrawSettings->setBlackPoint(group.readEntry("BlackPoint", 0));
#if KDCRAW_VERSION >= 0x000300
    d->dcrawSettings->setUseWhitePoint(group.readEntry("EnableWhitePoint", false));
    d->dcrawSettings->setWhitePoint(group.readEntry("WhitePoint", 0));
    d->dcrawSettings->setMedianFilterPasses(group.readEntry("MedianFilterPasses", 0));
#endif
}

}  // namespace Digikam
