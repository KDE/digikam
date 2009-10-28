/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save
 *               options into image editor.
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

#include "filesaveoptionsbox.h"
#include "filesaveoptionsbox.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QWidget>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kimageio.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "jpegsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"
#include "jp2ksettings.h"
#include "pgfsettings.h"
#include "debug.h"

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
        PGFOptions      = 0;
    }

    QWidget      *noneOptions;

    QGridLayout  *noneGrid;

    QLabel       *labelNone;

    JPEGSettings *JPEGOptions;

    PNGSettings  *PNGOptions;

    TIFFSettings *TIFFOptions;

    JP2KSettings *JPEG2000Options;

    PGFSettings  *PGFOptions;
};

FileSaveOptionsBox::FileSaveOptionsBox(QWidget *parent)
                  : QStackedWidget(parent), d(new FileSaveOptionsBoxPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    //-- NONE Settings ------------------------------------------------------

    d->noneOptions = new QWidget(this);
    d->noneGrid    = new QGridLayout(d->noneOptions);
    d->noneGrid->setSpacing(KDialog::spacingHint());
    d->noneOptions->setLayout(d->noneGrid);
    d->labelNone   = new QLabel(i18n("No options available"), d->noneOptions);
    d->noneGrid->addWidget(d->labelNone, 0, 0, 0, 1);

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions = new JPEGSettings(this);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions = new PNGSettings(this);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions = new TIFFSettings(this);

    //-- JPEG 2000 Settings -------------------------------------------------

    d->JPEG2000Options = new JP2KSettings(this);

    //-- PGF Settings -------------------------------------------------

    d->PGFOptions = new PGFSettings(this);

    //-----------------------------------------------------------------------

    insertWidget(DImg::NONE, d->noneOptions);
    insertWidget(DImg::JPEG, d->JPEGOptions);
    insertWidget(DImg::PNG,  d->PNGOptions);
    insertWidget(DImg::TIFF, d->TIFFOptions);
    insertWidget(DImg::JP2K, d->JPEG2000Options);
    insertWidget(DImg::PGF,  d->PGFOptions);

    //-----------------------------------------------------------------------

    readSettings();
}

FileSaveOptionsBox::~FileSaveOptionsBox()
{
    delete d;
}

void FileSaveOptionsBox::slotImageFileSelected(const QString& file)
{
    QString format = QImageReader::imageFormat(file);
    slotImageFileFormatChanged(format);
}

void FileSaveOptionsBox::slotImageFileFormatChanged(const QString& ext)
{
    kDebug() << "Format selected: " << ext;
    QString format = ext.toUpper();

    if (format.contains("JPEG") || format.contains("JPG") || format.contains("JPE"))
        setCurrentIndex(DImg::JPEG);
    else if (format.contains("PNG"))
        setCurrentIndex(DImg::PNG);
    else if (format.contains("TIFF") || format.contains("TIF"))
        setCurrentIndex(DImg::TIFF);
    else if (format.contains("JP2") || format.contains("JPX") || format.contains("JPC") ||
             format.contains("PGX") || format.contains("J2K"))
        setCurrentIndex(DImg::JP2K);
    else if (format.contains("PGF"))
        setCurrentIndex(DImg::PGF);
    else
        setCurrentIndex(DImg::NONE);
}

void FileSaveOptionsBox::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    group.writeEntry("JPEGCompression",     d->JPEGOptions->getCompressionValue());
    group.writeEntry("JPEGSubSampling",     d->JPEGOptions->getSubSamplingValue());
    group.writeEntry("PNGCompression",      d->PNGOptions->getCompressionValue());
    group.writeEntry("TIFFCompression",     d->TIFFOptions->getCompression());
    group.writeEntry("JPEG2000Compression", d->JPEG2000Options->getCompressionValue());
    group.writeEntry("JPEG2000LossLess",    d->JPEG2000Options->getLossLessCompression());
    group.writeEntry("PGFCompression",      d->PGFOptions->getCompressionValue());
    group.writeEntry("PGFLossLess",         d->PGFOptions->getLossLessCompression());
    config->sync();
}

void FileSaveOptionsBox::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    d->JPEGOptions->setCompressionValue( group.readEntry("JPEGCompression", 75) );
    d->JPEGOptions->setSubSamplingValue( group.readEntry("JPEGSubSampling", 1) );  // Medium subsampling
    d->PNGOptions->setCompressionValue( group.readEntry("PNGCompression", 9) );
    d->TIFFOptions->setCompression( group.readEntry("TIFFCompression", false) );
    d->JPEG2000Options->setCompressionValue( group.readEntry("JPEG2000Compression", 75) );
    d->JPEG2000Options->setLossLessCompression( group.readEntry("JPEG2000LossLess", true) );
    d->PGFOptions->setCompressionValue( group.readEntry("PGFCompression", 3) );
    d->PGFOptions->setLossLessCompression( group.readEntry("PGFLossLess", true) );
}

}  // namespace Digikam
