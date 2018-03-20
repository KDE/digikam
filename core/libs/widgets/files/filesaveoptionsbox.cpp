/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save
 *               options into image editor.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "jpegsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"
#include "pgfsettings.h"

#ifdef HAVE_JASPER
#include "jp2ksettings.h"
#endif // HAVE_JASPER

namespace Digikam
{

class FileSaveOptionsBox::Private
{

public:

    Private() :
        noneOptions(0),
        noneGrid(0),
        labelNone(0),
        JPEGOptions(0),
        PNGOptions(0),
        TIFFOptions(0),
#ifdef HAVE_JASPER
        JPEG2000Options(0),
#endif // HAVE_JASPER
        PGFOptions(0)
    {
    }

    QWidget*      noneOptions;

    QGridLayout*  noneGrid;

    QLabel*       labelNone;

    JPEGSettings* JPEGOptions;
    PNGSettings*  PNGOptions;
    TIFFSettings* TIFFOptions;

#ifdef HAVE_JASPER
    JP2KSettings* JPEG2000Options;
#endif // HAVE_JASPER

    PGFSettings*  PGFOptions;
};

FileSaveOptionsBox::FileSaveOptionsBox(QWidget* const parent)
    : QStackedWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    //-- NONE Settings ------------------------------------------------------

    d->noneOptions = new QWidget(this);
    d->noneGrid    = new QGridLayout(d->noneOptions);
    d->noneGrid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    d->noneOptions->setLayout(d->noneGrid);
    d->labelNone   = new QLabel(i18n("No options available"), d->noneOptions);
    d->noneGrid->addWidget(d->labelNone, 0, 0, 0, 1);

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions     = new JPEGSettings(this);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions      = new PNGSettings(this);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions     = new TIFFSettings(this);

    //-- JPEG 2000 Settings -------------------------------------------------
#ifdef HAVE_JASPER
    d->JPEG2000Options = new JP2KSettings(this);
#endif // HAVE_JASPER

    //-- PGF Settings -------------------------------------------------

    d->PGFOptions      = new PGFSettings(this);

    //-----------------------------------------------------------------------

    insertWidget(DImg::NONE, d->noneOptions);
    insertWidget(DImg::JPEG, d->JPEGOptions);
    insertWidget(DImg::PNG,  d->PNGOptions);
    insertWidget(DImg::TIFF, d->TIFFOptions);
#ifdef HAVE_JASPER
    insertWidget(DImg::JP2K, d->JPEG2000Options);
#endif // HAVE_JASPER
    insertWidget(DImg::PGF,  d->PGFOptions);

    //-----------------------------------------------------------------------

    readSettings();
}

FileSaveOptionsBox::~FileSaveOptionsBox()
{
    delete d;
}

void FileSaveOptionsBox::setImageFileFormat(const QString& ext)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << "Format selected: " << ext;
    setCurrentIndex(discoverFormat(ext, DImg::NONE));
}

DImg::FORMAT FileSaveOptionsBox::discoverFormat(const QString& filename, DImg::FORMAT fallback)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << "Trying to discover format based on filename '" << filename
                         << "', fallback = " << fallback;

    QStringList splitParts = filename.split(QLatin1Char('.'));
    QString ext;

    if (splitParts.size() < 2)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "filename '" << filename
                             << "' does not contain an extension separated by a point.";
        ext = filename;
    }
    else
    {
        ext = splitParts.at(splitParts.size() - 1);
    }

    ext = ext.toUpper();

    DImg::FORMAT format = fallback;

    if (ext.contains(QLatin1String("JPEG")) || ext.contains(QLatin1String("JPG")) || ext.contains(QLatin1String("JPE")))
    {
        format = DImg::JPEG;
    }
    else if (ext.contains(QLatin1String("PNG")))
    {
        format = DImg::PNG;
    }
    else if (ext.contains(QLatin1String("TIFF")) || ext.contains(QLatin1String("TIF")))
    {
        format = DImg::TIFF;
    }
#ifdef HAVE_JASPER
    else if (ext.contains(QLatin1String("JP2")) || ext.contains(QLatin1String("JPX")) || ext.contains(QLatin1String("JPC")) ||
             ext.contains(QLatin1String("PGX")) || ext.contains(QLatin1String("J2K")))
    {
        format = DImg::JP2K;
    }
#endif // HAVE_JASPER
    else if (ext.contains(QLatin1String("PGF")))
    {
        format = DImg::PGF;
    }
    else
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Using fallback format " << fallback;
    }

    qCDebug(DIGIKAM_WIDGETS_LOG) << "Discovered format: " << format;

    return format;
}

void FileSaveOptionsBox::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("ImageViewer Settings");
    group.writeEntry(QLatin1String("JPEGCompression"),     d->JPEGOptions->getCompressionValue());
    group.writeEntry(QLatin1String("JPEGSubSampling"),     d->JPEGOptions->getSubSamplingValue());
    group.writeEntry(QLatin1String("PNGCompression"),      d->PNGOptions->getCompressionValue());
    group.writeEntry(QLatin1String("TIFFCompression"),     d->TIFFOptions->getCompression());
#ifdef HAVE_JASPER
    group.writeEntry(QLatin1String("JPEG2000Compression"), d->JPEG2000Options->getCompressionValue());
    group.writeEntry(QLatin1String("JPEG2000LossLess"),    d->JPEG2000Options->getLossLessCompression());
#endif // HAVE_JASPER
    group.writeEntry(QLatin1String("PGFCompression"),      d->PGFOptions->getCompressionValue());
    group.writeEntry(QLatin1String("PGFLossLess"),         d->PGFOptions->getLossLessCompression());
    config->sync();
}

void FileSaveOptionsBox::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("ImageViewer Settings");
    d->JPEGOptions->setCompressionValue( group.readEntry(QLatin1String("JPEGCompression"),         75) );
    d->JPEGOptions->setSubSamplingValue( group.readEntry(QLatin1String("JPEGSubSampling"),         1) );  // Medium subsampling
    d->PNGOptions->setCompressionValue( group.readEntry(QLatin1String("PNGCompression"),           9) );
    d->TIFFOptions->setCompression( group.readEntry(QLatin1String("TIFFCompression"),              false) );
#ifdef HAVE_JASPER
    d->JPEG2000Options->setCompressionValue( group.readEntry(QLatin1String("JPEG2000Compression"), 75) );
    d->JPEG2000Options->setLossLessCompression( group.readEntry(QLatin1String("JPEG2000LossLess"), true) );
#endif // HAVE_JASPER
    d->PGFOptions->setCompressionValue( group.readEntry(QLatin1String("PGFCompression"),           3) );
    d->PGFOptions->setLossLessCompression( group.readEntry(QLatin1String("PGFLossLess"),           true) );
}

}  // namespace Digikam
