/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save 
 *               options into image editor.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlayout.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

// KDE includes.

#include <kimageio.h>
#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kfiledialog.h>

// Local includes.

#include "jpegsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"
#include "jp2ksettings.h"
#include "filesaveoptionsbox.h"
#include "filesaveoptionsbox.moc"

namespace Digikam
{

class FileSaveOptionsBoxPriv
{

public:

    FileSaveOptionsBoxPriv()
    {
        noneOptions     = 0;
        JPEGOptions     = 0;
        PNGOptions      = 0;
        TIFFOptions     = 0;
        JPEG2000Options = 0;
    }

    QWidget      *noneOptions;

    QGridLayout  *noneGrid;

    QLabel       *labelNone;

    JPEGSettings *JPEGOptions;

    PNGSettings  *PNGOptions;

    TIFFSettings *TIFFOptions;

    JP2KSettings *JPEG2000Options;
};

FileSaveOptionsBox::FileSaveOptionsBox(QWidget *parent)
                  : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new FileSaveOptionsBoxPriv;

    //-- NONE Settings ------------------------------------------------------

    d->noneOptions = new QWidget(this);
    d->noneGrid    = new QGridLayout(d->noneOptions, 1, 1, KDialog::spacingHint());
    d->labelNone   = new QLabel(i18n("No options available"), d->noneOptions);
    d->noneGrid->addMultiCellWidget(d->labelNone, 0, 0, 0, 1);

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions = new JPEGSettings(this);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions = new PNGSettings(this);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions = new TIFFSettings(this);

    //-- JPEG 2000 Settings -------------------------------------------------

    d->JPEG2000Options = new JP2KSettings(this);

    //-----------------------------------------------------------------------

    addWidget(d->noneOptions,     DImg::NONE);
    addWidget(d->JPEGOptions,     DImg::JPEG);
    addWidget(d->PNGOptions,      DImg::PNG);
    addWidget(d->TIFFOptions,     DImg::TIFF);
    addWidget(d->JPEG2000Options, DImg::JP2K);

    //-----------------------------------------------------------------------

    readSettings();
}

FileSaveOptionsBox::~FileSaveOptionsBox()
{
    delete d;
}

void FileSaveOptionsBox::slotImageFileSelected(const QString& file)
{
    QString format = QImageIO::imageFormat(file);
    toggleFormatOptions(format);
}

void FileSaveOptionsBox::slotImageFileFormatChanged(const QString& filter)
{
    QString format = KImageIO::typeForMime(filter).upper();
    toggleFormatOptions(format);
}

void FileSaveOptionsBox::toggleFormatOptions(const QString& format)
{
    if (format == QString("JPEG"))
        raiseWidget(DImg::JPEG);
    else if (format == QString("PNG"))
        raiseWidget(DImg::PNG);
    else if (format == QString("TIFF"))
        raiseWidget(DImg::TIFF);
    else if (format == QString("JP2"))
        raiseWidget(DImg::JP2K);
    else
        raiseWidget(DImg::NONE);
}

void FileSaveOptionsBox::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("JPEGCompression", d->JPEGOptions->getCompressionValue());
    config->writeEntry("JPEGSubSampling", d->JPEGOptions->getSubSamplingValue());
    config->writeEntry("PNGCompression", d->PNGOptions->getCompressionValue());
    config->writeEntry("TIFFCompression", d->TIFFOptions->getCompression());
    config->writeEntry("JPEG2000Compression", d->JPEG2000Options->getCompressionValue());
    config->writeEntry("JPEG2000LossLess", d->JPEG2000Options->getLossLessCompression());
    config->sync();
}

void FileSaveOptionsBox::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    d->JPEGOptions->setCompressionValue( config->readNumEntry("JPEGCompression", 75) );
    d->JPEGOptions->setSubSamplingValue( config->readNumEntry("JPEGSubSampling", 1) );  // Medium subsampling
    d->PNGOptions->setCompressionValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFOptions->setCompression(config->readBoolEntry("TIFFCompression", false));
    d->JPEG2000Options->setCompressionValue( config->readNumEntry("JPEG2000Compression", 75) );
    d->JPEG2000Options->setLossLessCompression( config->readBoolEntry("JPEG2000LossLess", true) );
}

}  // namespace Digikam
