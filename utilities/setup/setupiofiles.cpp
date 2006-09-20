/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-23
 * Description : setup image editor Input Output files.
 *
 * Copyright 2006 by Gilles Caulier
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

#include "dcrawbinary.h"
#include "dcrawsettingswidget.h"
#include "setupiofiles.h"
#include "setupiofiles.moc"

namespace Digikam
{

class SetupIOFilesPriv
{
public:


    SetupIOFilesPriv()
    {
        labelJPEGcompression = 0;
        labelPNGcompression  = 0;
        JPEGcompression      = 0;
        PNGcompression       = 0;
        TIFFcompression      = 0;
        dcrawSettings        = 0;
    }

    QLabel              *labelJPEGcompression;
    QLabel              *labelPNGcompression;

    QCheckBox           *TIFFcompression;

    KIntNumInput        *JPEGcompression;
    KIntNumInput        *PNGcompression;

    DcrawSettingsWidget *dcrawSettings;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    d->dcrawSettings = new DcrawSettingsWidget(parent, Digikam::DcrawBinary::instance()->version());
    layout->addWidget(d->dcrawSettings);

    // --------------------------------------------------------

    QGroupBox *savingOptionsGroup = new QGroupBox(0, Qt::Vertical, i18n("Saving Images Options"), parent);

    QGridLayout* grid2 = new QGridLayout( savingOptionsGroup->layout(), 2, 1, KDialog::spacingHint());

    d->JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
    d->JPEGcompression->setRange(1, 100, 1, true );
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), savingOptionsGroup);

    QWhatsThis::add( d->JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                                "<b>1</b>: low quality (high compression and small "
                                                "file size)<p>"
                                                "<b>50</b>: medium quality<p>"
                                                "<b>75</b>: good quality (default)<p>"
                                                "<b>100</b>: high quality (no compression and "
                                                "large file size)<p>"
                                                "<b>Note: JPEG is not a lossless image "
                                                "compression format.</b>"));
    grid2->addMultiCellWidget(d->labelJPEGcompression, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->JPEGcompression, 0, 0, 1, 1);

    d->PNGcompression = new KIntNumInput(1, savingOptionsGroup);
    d->PNGcompression->setRange(1, 9, 1, true );
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), savingOptionsGroup);

    QWhatsThis::add( d->PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                             "<b>1</b>: low compression (large file size but "
                                             "short compression duration - default)<p>"
                                             "<b>5</b>: medium compression<p>"
                                             "<b>9</b>: high compression (small file size but "
                                             "long compression duration)<p>"
                                             "<b>Note: PNG is always a lossless image "
                                             "compression format.</b>"));
    grid2->addMultiCellWidget(d->labelPNGcompression, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->PNGcompression, 1, 1, 1, 1);

    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"),
                                       savingOptionsGroup);

    QWhatsThis::add( d->TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                              "If you enable this option, you can reduce "
                                              "the final file size of the TIFF image.</p>"
                                              "<p>A lossless compression format (Deflate) "
                                              "is used to save the file.<p>"));
    grid2->addMultiCellWidget(d->TIFFcompression, 2, 2, 0, 1);
    grid2->setColStretch(1, 10);

    layout->addWidget(savingOptionsGroup);
    layout->addStretch();

    readSettings();
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
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

    config->writeEntry("JPEGCompression", d->JPEGcompression->value());
    config->writeEntry("PNGCompression", d->PNGcompression->value());
    config->writeEntry("TIFFCompression", d->TIFFcompression->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
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

    d->JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    d->PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
}

}  // namespace Digikam
