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

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kseparator.h>

// Local includes.

#include "jpegsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"
#include "jp2ksettings.h"
#include "setupiofiles.h"
#include "setupiofiles.moc"

namespace Digikam
{

class SetupIOFilesPriv
{
public:


    SetupIOFilesPriv()
    {
        JPEGOptions     = 0;
        PNGOptions      = 0;
        TIFFOptions     = 0;
        JPEG2000Options = 0;
    }

    JPEGSettings *JPEGOptions;

    PNGSettings  *PNGOptions;

    TIFFSettings *TIFFOptions;

    JP2KSettings *JPEG2000Options;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;

    QVBoxLayout* vbox = new QVBoxLayout(parent);

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions    = new JPEGSettings(parent);
    KSeparator *line1 = new KSeparator(Horizontal, parent);
    vbox->addWidget(d->JPEGOptions);
    vbox->addWidget(line1);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions     = new PNGSettings(parent);
    KSeparator *line2 = new KSeparator(Horizontal, parent);
    vbox->addWidget(d->PNGOptions);
    vbox->addWidget(line2);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions    = new TIFFSettings(parent);
    KSeparator *line3 = new KSeparator(Horizontal, parent);
    vbox->addWidget(d->TIFFOptions);
    vbox->addWidget(line3);

    //-- JPEG 2000 Settings -------------------------------------------------

    d->JPEG2000Options = new JP2KSettings(parent);
    vbox->addWidget(d->JPEG2000Options);

    vbox->addStretch(10);
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
    config->writeEntry("JPEGCompression", d->JPEGOptions->getCompressionValue());
    config->writeEntry("PNGCompression", d->PNGOptions->getCompressionValue());
    config->writeEntry("TIFFCompression", d->TIFFOptions->getCompression());
    config->writeEntry("JPEG2000Compression", d->JPEG2000Options->getCompressionValue());
    config->writeEntry("JPEG2000LossLess", d->JPEG2000Options->getLossLessCompression());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    d->JPEGOptions->setCompressionValue(config->readNumEntry("JPEGCompression", 75) );
    d->PNGOptions->setCompressionValue(config->readNumEntry("PNGCompression", 9) );
    d->TIFFOptions->setCompression(config->readBoolEntry("TIFFCompression", false));
    d->JPEG2000Options->setCompressionValue( config->readNumEntry("JPEG2000Compression", 75) );
    d->JPEG2000Options->setLossLessCompression( config->readBoolEntry("JPEG2000LossLess", true) );
}

}  // namespace Digikam
