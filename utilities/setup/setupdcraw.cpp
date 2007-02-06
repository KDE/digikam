/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
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

// Local includes.

#include "dcrawsettingswidget.h"
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

    DcrawSettingsWidget *dcrawSettings;
};

SetupDcraw::SetupDcraw(QWidget* parent )
          : QWidget(parent)
{
    d = new SetupDcrawPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );
    d->dcrawSettings    = new DcrawSettingsWidget(parent);
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
    config->writeEntry("SuperCCDsecondarySensor", d->dcrawSettings->useSecondarySensor());
    config->writeEntry("EnableNoiseReduction", d->dcrawSettings->useNoiseReduction());
    config->writeEntry("NRSigmaDomain", d->dcrawSettings->sigmaDomain());
    config->writeEntry("NRSigmaRange", d->dcrawSettings->sigmaRange());
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
    d->dcrawSettings->setSigmaDomain(config->readDoubleNumEntry("NRSigmaDomain", 2.0));
    d->dcrawSettings->setSigmaRange(config->readDoubleNumEntry("NRSigmaRange", 4.0));
    d->dcrawSettings->setSecondarySensor(config->readBoolEntry("SuperCCDsecondarySensor", false));
    d->dcrawSettings->setUnclipColor(config->readNumEntry("UnclipColors", 0));
    d->dcrawSettings->setCameraWB(config->readBoolEntry("CameraColorBalance", true));
    d->dcrawSettings->setAutoColorBalance(config->readBoolEntry("AutomaticColorBalance", true));
    d->dcrawSettings->setFourColor(config->readBoolEntry("RGBInterpolate4Colors", false));
    d->dcrawSettings->setQuality((RawDecodingSettings::DecodingQuality)config->readNumEntry("RAWQuality",
                                  RawDecodingSettings::BILINEAR));
    d->dcrawSettings->setBrightness(config->readDoubleNumEntry("RAWBrightness", 1.0));
}

}  // namespace Digikam
