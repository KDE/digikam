/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// #include "setupdcraw.h"
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
        configSixteenBitsImageEntry("SixteenBitsImage"),
        configWhiteBalanceEntry("WhiteBalance"),
        configCustomWhiteBalanceEntry("CustomWhiteBalance"),
        configCustomWhiteBalanceGreenEntry("CustomWhiteBalanceGreen"),
        configRGBInterpolate4ColorsEntry("RGBInterpolate4Colors"),
        configDontStretchPixelsEntry("DontStretchPixels"),
        configEnableNoiseReductionEntry("EnableNoiseReduction"),
        configNRThresholdEntry("NRThreshold"),
        configEnableCACorrectionEntry("EnableCACorrection"),
        configcaRedMultiplierEntry("caRedMultiplier"),
        configcaBlueMultiplierEntry("caBlueMultiplier"),
        configUnclipColorsEntry("UnclipColors"),
        configRAWBrightnessEntry("RAWBrightness"),
        configRAWQualityEntry("RAWQuality"),
        configMedianFilterPassesEntry("MedianFilterPasses"),
        configAutoBrightnessEntry("AutoBrightness"),

        dcrawSettings(0)
    {}

    const QString        configGroupName; 
    const QString        configSixteenBitsImageEntry;
    const QString        configWhiteBalanceEntry;
    const QString        configCustomWhiteBalanceEntry;
    const QString        configCustomWhiteBalanceGreenEntry;
    const QString        configRGBInterpolate4ColorsEntry;
    const QString        configDontStretchPixelsEntry;
    const QString        configEnableNoiseReductionEntry;
    const QString        configNRThresholdEntry;
    const QString        configEnableCACorrectionEntry;
    const QString        configcaRedMultiplierEntry;
    const QString        configcaBlueMultiplierEntry;
    const QString        configUnclipColorsEntry;
    const QString        configRAWBrightnessEntry;
    const QString        configRAWQualityEntry;
    const QString        configMedianFilterPassesEntry;
    const QString        configAutoBrightnessEntry;

    DcrawSettingsWidget* dcrawSettings;
};

SetupDcraw::SetupDcraw(QWidget* parent )
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
#if KDCRAW_VERSION >= 0x000300
    // Dcraw do not provide a way to set brightness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->dcrawSettings->setEnabledBrightnessSettings(true);
#endif
}

void SetupDcraw::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configSixteenBitsImageEntry,        d->dcrawSettings->sixteenBits());
    group.writeEntry(d->configWhiteBalanceEntry,            (int)d->dcrawSettings->whiteBalance());
    group.writeEntry(d->configCustomWhiteBalanceEntry,      d->dcrawSettings->customWhiteBalance());
    group.writeEntry(d->configCustomWhiteBalanceGreenEntry, d->dcrawSettings->customWhiteBalanceGreen());
    group.writeEntry(d->configRGBInterpolate4ColorsEntry,   d->dcrawSettings->useFourColor());
    group.writeEntry(d->configDontStretchPixelsEntry,       d->dcrawSettings->useDontStretchPixels());
    group.writeEntry(d->configEnableNoiseReductionEntry,    d->dcrawSettings->useNoiseReduction());
    group.writeEntry(d->configNRThresholdEntry,             d->dcrawSettings->NRThreshold());
    group.writeEntry(d->configEnableCACorrectionEntry,      d->dcrawSettings->useCACorrection());
    group.writeEntry(d->configcaRedMultiplierEntry,         d->dcrawSettings->caRedMultiplier());
    group.writeEntry(d->configcaBlueMultiplierEntry,        d->dcrawSettings->caBlueMultiplier());
    group.writeEntry(d->configUnclipColorsEntry,            d->dcrawSettings->unclipColor());
    group.writeEntry(d->configRAWBrightnessEntry,           d->dcrawSettings->brightness());
    group.writeEntry(d->configRAWQualityEntry,              (int)d->dcrawSettings->quality());
    group.writeEntry(d->configMedianFilterPassesEntry,      d->dcrawSettings->medianFilterPasses());
#if KDCRAW_VERSION >= 0x000500
    group.writeEntry(d->configAutoBrightnessEntry,          d->dcrawSettings->useAutoBrightness());
#endif
    config->sync();
}

void SetupDcraw::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->dcrawSettings->setSixteenBits(group.readEntry(d->configSixteenBitsImageEntry,        false));
    d->dcrawSettings->setNoiseReduction(group.readEntry(d->configEnableNoiseReductionEntry, false));
    d->dcrawSettings->setNRThreshold(group.readEntry(d->configNRThresholdEntry,             100));
    d->dcrawSettings->setUseCACorrection(group.readEntry(d->configEnableCACorrectionEntry,  false));
    d->dcrawSettings->setcaRedMultiplier(group.readEntry(d->configcaRedMultiplierEntry,     1.0));
    d->dcrawSettings->setcaBlueMultiplier(group.readEntry(d->configcaBlueMultiplierEntry,   1.0));
    d->dcrawSettings->setDontStretchPixels(group.readEntry(d->configDontStretchPixelsEntry, false));
    d->dcrawSettings->setUnclipColor(group.readEntry(d->configUnclipColorsEntry,            0));
    d->dcrawSettings->setWhiteBalance((DRawDecoding::WhiteBalance)
                                      group.readEntry(d->configWhiteBalanceEntry,
                                      (int)DRawDecoding::CAMERA));
    d->dcrawSettings->setCustomWhiteBalance(group.readEntry(d->configCustomWhiteBalanceEntry,           6500));
    d->dcrawSettings->setCustomWhiteBalanceGreen(group.readEntry(d->configCustomWhiteBalanceGreenEntry, 1.0));
    d->dcrawSettings->setFourColor(group.readEntry(d->configRGBInterpolate4ColorsEntry,                 false));
    d->dcrawSettings->setQuality((DRawDecoding::DecodingQuality)
                                  group.readEntry(d->configRAWQualityEntry,
                                  (int)DRawDecoding::BILINEAR));
    d->dcrawSettings->setBrightness(group.readEntry(d->configRAWBrightnessEntry,              1.0));
    d->dcrawSettings->setMedianFilterPasses(group.readEntry(d->configMedianFilterPassesEntry, 0));

#if KDCRAW_VERSION >= 0x000500
    d->dcrawSettings->setAutoBrightness(group.readEntry(d->configAutoBrightnessEntry, true));
#endif
}

}  // namespace Digikam
