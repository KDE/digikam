/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-01-23
 * Description : setup image editor output files settings.
 * 
 * Copyright 2006-2007 by Gilles Caulier
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
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kseparator.h>

// Local includes.

#include "setupiofiles.h"
#include "setupiofiles.moc"

namespace Digikam
{

class SetupIOFilesPriv
{
public:


    SetupIOFilesPriv()
    {
        labelJPEGcompression     = 0;
        JPEGcompression          = 0;

        labelPNGcompression      = 0;
        PNGcompression           = 0;

        TIFFcompression          = 0;

        labelJPEG2000compression = 0;
        JPEG2000compression      = 0;
        JPEG2000LossLess         = 0;
    }

    QLabel       *labelPNGcompression;
    QLabel       *labelJPEGcompression;
    QLabel       *labelJPEG2000compression;

    QCheckBox    *JPEG2000LossLess;
    QCheckBox    *TIFFcompression;

    KIntNumInput *PNGcompression;
    KIntNumInput *JPEGcompression;
    KIntNumInput *JPEG2000compression;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;

    QGridLayout* grid = new QGridLayout(parent, 8, 1, KDialog::spacingHint());

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGcompression = new KIntNumInput(75, parent);
    d->JPEGcompression->setRange(1, 100, 1, true );
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), parent);

    QWhatsThis::add( d->JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                                "<b>1</b>: low quality (high compression and small "
                                                "file size)<p>"
                                                "<b>50</b>: medium quality<p>"
                                                "<b>75</b>: good quality (default)<p>"
                                                "<b>100</b>: high quality (no compression and "
                                                "large file size)<p>"
                                                "<b>Note: JPEG is not a lossless image "
                                                "compression format.</b>"));
    grid->addMultiCellWidget(d->labelJPEGcompression, 0, 0, 0, 0);
    grid->addMultiCellWidget(d->JPEGcompression, 0, 0, 1, 1);

    KSeparator *line1 = new KSeparator(Horizontal, parent);
    grid->addMultiCellWidget(line1, 1, 1, 0, 1);

    //-- PNG Settings -------------------------------------------------------

    d->PNGcompression = new KIntNumInput(1, parent);
    d->PNGcompression->setRange(1, 9, 1, true );
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), parent);

    QWhatsThis::add( d->PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                             "<b>1</b>: low compression (large file size but "
                                             "short compression duration - default)<p>"
                                             "<b>5</b>: medium compression<p>"
                                             "<b>9</b>: high compression (small file size but "
                                             "long compression duration)<p>"
                                             "<b>Note: PNG is always a lossless image "
                                             "compression format.</b>"));
    grid->addMultiCellWidget(d->labelPNGcompression, 2, 2, 0, 0);
    grid->addMultiCellWidget(d->PNGcompression, 2, 2, 1, 1);

    KSeparator *line2 = new KSeparator(Horizontal, parent);
    grid->addMultiCellWidget(line2, 3, 3, 0, 1);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"), parent);

    QWhatsThis::add( d->TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                              "If you enable this option, you can reduce "
                                              "the final file size of the TIFF image.</p>"
                                              "<p>A lossless compression format (Deflate) "
                                              "is used to save the file.<p>"));
    grid->addMultiCellWidget(d->TIFFcompression, 4, 4, 0, 1);

    KSeparator *line3 = new KSeparator(Horizontal, parent);
    grid->addMultiCellWidget(line3, 5, 5, 0, 1);

    //-- JPEG 2000 Settings -------------------------------------------------

    d->JPEG2000LossLess = new QCheckBox(i18n("LossLess JPEG 2000 files"), parent);

    QWhatsThis::add( d->JPEG2000LossLess, i18n("<p>Toggle lossless compression for JPEG 2000 images.<p>"
                                               "If you enable this option, you will use a lossless method "
                                               "to compress JPEG 2000 pictures.<p>"));
    grid->addMultiCellWidget(d->JPEG2000LossLess, 6, 6, 0, 1);

    d->JPEG2000compression = new KIntNumInput(75, parent);
    d->JPEG2000compression->setRange(1, 100, 1, true );
    d->labelJPEG2000compression = new QLabel(i18n("JPEG 2000 quality:"), parent);

    QWhatsThis::add( d->JPEGcompression, i18n("<p>The quality value for JPEG 2000 images:<p>"
                                              "<b>1</b>: low quality (high compression and small "
                                              "file size)<p>"
                                              "<b>50</b>: medium quality<p>"
                                              "<b>75</b>: good quality (default)<p>"
                                              "<b>100</b>: high quality (no compression and "
                                              "large file size)<p>"
                                              "<b>Note: JPEG 2000 is not a lossless image "
                                              "compression format when you use this setting.</b>"));
    grid->addMultiCellWidget(d->labelJPEG2000compression, 7, 7, 0, 0);
    grid->addMultiCellWidget(d->JPEG2000compression, 7, 7, 1, 1);

    grid->setColStretch(1, 10);
    grid->setRowStretch(8, 10);

    connect(d->JPEG2000LossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleJPEG2000LossLess(bool)));

    connect(d->JPEG2000LossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleJPEG2000LossLess(bool)));

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
    config->writeEntry("JPEGCompression", d->JPEGcompression->value());
    config->writeEntry("PNGCompression", d->PNGcompression->value());
    config->writeEntry("TIFFCompression", d->TIFFcompression->isChecked());
    config->writeEntry("JPEG2000Compression", d->JPEG2000compression->value());
    config->writeEntry("JPEG2000LossLess", d->JPEG2000LossLess->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    d->JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    d->PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    d->JPEG2000compression->setValue( config->readNumEntry("JPEG2000Compression", 75) );
    d->JPEG2000LossLess->setChecked( config->readBoolEntry("JPEG2000LossLess", true) );
    slotToggleJPEG2000LossLess(d->JPEG2000LossLess->isChecked());
}

void SetupIOFiles::slotToggleJPEG2000LossLess(bool b)
{
    d->JPEG2000compression->setEnabled(!b);
    d->labelJPEG2000compression->setEnabled(!b);
}

}  // namespace Digikam
