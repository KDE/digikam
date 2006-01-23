/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
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
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klistview.h>
#include <ktrader.h>

// Local includes.

#include "setupiofiles.h"

namespace Digikam
{

class SetupIOFilesPriv
{
public:

    SetupIOFilesPriv()
    {
        RAWquality            = 0;
        JPEGcompression       = 0;
        PNGcompression        = 0;
        cameraColorBalance    = 0;
        automaticColorBalance = 0;
        enableRAWQuality      = 0;
        RGBInterpolate4Colors = 0;
        TIFFcompression       = 0;
    }

    KIntNumInput *RAWquality;
    KIntNumInput *JPEGcompression;
    KIntNumInput *PNGcompression;
    
    QCheckBox    *cameraColorBalance;
    QCheckBox    *automaticColorBalance;
    QCheckBox    *enableRAWQuality;
    QCheckBox    *RGBInterpolate4Colors;
    QCheckBox    *TIFFcompression;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent );
    
    // --------------------------------------------------------
    
    QVGroupBox *RAWfileOptionsGroup = new QVGroupBox(i18n("RAW Image Decoding Options"), parent);
    
    d->RGBInterpolate4Colors = new QCheckBox(i18n("Interpolate RGB as four colors"), RAWfileOptionsGroup);
    QWhatsThis::add( d->RGBInterpolate4Colors, i18n("<p>Interpolate RGB as four colors. This blurs the image a little, "
                                                    "but it eliminates false 2x2 mesh patterns.<p>"));
    
    d->automaticColorBalance = new QCheckBox(i18n("Automatic color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->automaticColorBalance, i18n("<p>Automatic color balance. The default is to use a fixed color "
                                                    "balance based on a white card photographed in sunlight.<p>"));
    
    d->cameraColorBalance = new QCheckBox(i18n("Camera color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->cameraColorBalance, i18n("<p>Use the color balance specified by the camera. If this can't "
                                                "be found, reverts to the default.<p>"));
    
    d->enableRAWQuality = new QCheckBox(i18n("Enable RAW decoding quality"), RAWfileOptionsGroup);
    QWhatsThis::add( d->enableRAWQuality, i18n("<p>Toggle quality decoding option for RAW images.<p>"));
    
    d->RAWquality = new KIntNumInput(0, RAWfileOptionsGroup);
    d->RAWquality->setRange(0, 3, 1, true );
    d->RAWquality->setLabel( i18n("&RAW file decoding quality:"), AlignLeft|AlignVCenter );
    
    QWhatsThis::add( d->RAWquality, i18n("<p>The decoding quality value for RAW images:<p>"
                                        "<b>0</b>: medium quality (default - for slow computer)<p>"
                                        "<b>1</b>: good quality<p>"
                                        "<b>2</b>: high quality<p>"
                                        "<b>3</b>: very high quality (for speed computer)</b>"));
    
    layout->addWidget(RAWfileOptionsGroup);
    
    connect(d->enableRAWQuality, SIGNAL(toggled(bool)),
            d->RAWquality, SLOT(setEnabled(bool)));
    
    // --------------------------------------------------------
    
    QVGroupBox *savingOptionsGroup = new QVGroupBox(i18n("Saving Images Options"),
                                                    parent);
    
    d->JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
    d->JPEGcompression->setRange(1, 100, 1, true );
    d->JPEGcompression->setLabel( i18n("&JPEG quality:"), AlignLeft|AlignVCenter );
    
    QWhatsThis::add( d->JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                                "<b>1</b>: low quality (high compression and small file size)<p>"
                                                "<b>50</b>: medium quality<p>"
                            "<b>75</b>: good quality (default)<p>"
                                                "<b>100</b>: high quality (no compression and large file size)<p>"
                                                "<b>Note: JPEG is not a lossless image compression format.</b>"));
    
    d->PNGcompression = new KIntNumInput(1, savingOptionsGroup);
    d->PNGcompression->setRange(1, 9, 1, true );
    d->PNGcompression->setLabel( i18n("&PNG compression:"), AlignLeft|AlignVCenter );
    
    QWhatsThis::add( d->PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                            "<b>1</b>: low compression (large file size but "
                                            "short compression duration - default)<p>"
                                            "<b>5</b>: medium compression<p>"
                                            "<b>9</b>: high compression (small file size but "
                                            "long compression duration)<p>"
                                            "<b>Note: PNG is always a lossless image compression format.</b>"));
    
    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"),
                                        savingOptionsGroup);
    
    QWhatsThis::add( d->TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                                "If you enable this option, you can reduce "
                                                "the final file size of the TIFF image.</p>"
                                                "<p>A lossless compression format (Adobe Deflate) "
                                                "is used to save the file.<p>"));
    
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
    config->writeEntry("RAWQuality", d->RAWquality->value());
    config->writeEntry("EnableRAWQuality", d->enableRAWQuality->isChecked());
    config->writeEntry("AutomaticColorBalance", d->automaticColorBalance->isChecked());
    config->writeEntry("CameraColorBalance", d->cameraColorBalance->isChecked());
    config->writeEntry("RGBInterpolate4Colors", d->RGBInterpolate4Colors->isChecked());
    config->writeEntry("JPEGCompression", d->JPEGcompression->value());
    config->writeEntry("PNGCompression", d->PNGcompression->value());
    config->writeEntry("TIFFCompression", d->TIFFcompression->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    d->RAWquality->setValue( config->readNumEntry("RAWQuality", 0) );
    d->enableRAWQuality->setChecked(config->readBoolEntry("EnableRAWQuality", false));
    d->cameraColorBalance->setChecked(config->readBoolEntry("AutomaticColorBalance", true));
    d->automaticColorBalance->setChecked(config->readBoolEntry("CameraColorBalance", true));
    d->RGBInterpolate4Colors->setChecked(config->readBoolEntry("RGBInterpolate4Colors", false));
    d->JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    d->PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    d->RAWquality->setEnabled(d->enableRAWQuality->isChecked());
}

}  // namespace Digikam

#include "setupiofiles.moc"
