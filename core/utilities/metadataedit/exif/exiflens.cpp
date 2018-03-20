/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF lens settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "exiflens.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QSpinBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metadatacheckbox.h"
#include "dmetadata.h"

namespace Digikam
{

class EXIFLens::Private
{
public:

    Private()
    {
        apertureCheck         = 0;
        maxApertureCheck      = 0;
        focalLength35mmCheck  = 0;
        focalLengthCheck      = 0;
        digitalZoomRatioCheck = 0;
        apertureCB            = 0;
        maxApertureCB         = 0;
        focalLength35mmEdit   = 0;
        focalLengthEdit       = 0;
        digitalZoomRatioEdit  = 0;

        apertureValues.append(QLatin1String("f/1.0"));
        apertureValues.append(QLatin1String("f/1.1"));
        apertureValues.append(QLatin1String("f/1.2"));
        apertureValues.append(QLatin1String("f/1.3"));
        apertureValues.append(QLatin1String("f/1.4"));
        apertureValues.append(QLatin1String("f/1.6"));
        apertureValues.append(QLatin1String("f/1.8"));
        apertureValues.append(QLatin1String("f/2.0"));
        apertureValues.append(QLatin1String("f/2.2"));
        apertureValues.append(QLatin1String("f/2.5"));
        apertureValues.append(QLatin1String("f/2.8"));
        apertureValues.append(QLatin1String("f/3.2"));
        apertureValues.append(QLatin1String("f/3.5"));
        apertureValues.append(QLatin1String("f/3.6"));
        apertureValues.append(QLatin1String("f/4.0"));
        apertureValues.append(QLatin1String("f/4.5"));
        apertureValues.append(QLatin1String("f/4.9"));
        apertureValues.append(QLatin1String("f/5.0"));
        apertureValues.append(QLatin1String("f/5.6"));
        apertureValues.append(QLatin1String("f/5.7"));
        apertureValues.append(QLatin1String("f/6.3"));
        apertureValues.append(QLatin1String("f/7.0"));
        apertureValues.append(QLatin1String("f/7.1"));
        apertureValues.append(QLatin1String("f/8.0"));
        apertureValues.append(QLatin1String("f/9.0"));
        apertureValues.append(QLatin1String("f/10.0"));
        apertureValues.append(QLatin1String("f/10.1"));
        apertureValues.append(QLatin1String("f/11.0"));
        apertureValues.append(QLatin1String("f/11.3"));
        apertureValues.append(QLatin1String("f/12.0"));
        apertureValues.append(QLatin1String("f/12.7"));
        apertureValues.append(QLatin1String("f/13.0"));
        apertureValues.append(QLatin1String("f/14.0"));
        apertureValues.append(QLatin1String("f/14.3"));
        apertureValues.append(QLatin1String("f/16.0"));
        apertureValues.append(QLatin1String("f/18.0"));
        apertureValues.append(QLatin1String("f/20.0"));
        apertureValues.append(QLatin1String("f/20.2"));
        apertureValues.append(QLatin1String("f/22.0"));
        apertureValues.append(QLatin1String("f/22.6"));
        apertureValues.append(QLatin1String("f/25.0"));
        apertureValues.append(QLatin1String("f/25.4"));
        apertureValues.append(QLatin1String("f/28.5"));
        apertureValues.append(QLatin1String("f/29.0"));
        apertureValues.append(QLatin1String("f/32.0"));
        apertureValues.append(QLatin1String("f/35.9"));
        apertureValues.append(QLatin1String("f/36.0"));
        apertureValues.append(QLatin1String("f/40.0"));
        apertureValues.append(QLatin1String("f/40.3"));
        apertureValues.append(QLatin1String("f/45.0"));
        apertureValues.append(QLatin1String("f/45.3"));
        apertureValues.append(QLatin1String("f/50.8"));
        apertureValues.append(QLatin1String("f/51.0"));
        apertureValues.append(QLatin1String("f/57.0"));
        apertureValues.append(QLatin1String("f/64.0"));
        apertureValues.append(QLatin1String("f/72.0"));
        apertureValues.append(QLatin1String("f/81.0"));
        apertureValues.append(QLatin1String("f/91.0"));
    }

    QStringList       apertureValues;

    QCheckBox*        focalLength35mmCheck;
    QCheckBox*        focalLengthCheck;
    QCheckBox*        digitalZoomRatioCheck;

    QComboBox*        apertureCB;
    QComboBox*        maxApertureCB;

    QSpinBox*         focalLength35mmEdit;

    QDoubleSpinBox*   focalLengthEdit;
    QDoubleSpinBox*   digitalZoomRatioEdit;

    MetadataCheckBox* apertureCheck;
    MetadataCheckBox* maxApertureCheck;
};

EXIFLens::EXIFLens(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->focalLengthCheck = new QCheckBox(i18n("Focal length (mm):"), this);
    d->focalLengthEdit  = new QDoubleSpinBox(this);
    d->focalLengthEdit->setRange(1.0, 10000.0);
    d->focalLengthEdit->setSingleStep(1.0);
    d->focalLengthEdit->setValue(50.0);
    d->focalLengthEdit->setDecimals(1);
    d->focalLengthEdit->setWhatsThis(i18n("Set here the lens focal length in millimeters "
                                          "used by camera to take the picture."));

    // --------------------------------------------------------

    d->focalLength35mmCheck = new QCheckBox(i18n("Focal length in 35mm film (mm):"), this);
    d->focalLength35mmEdit  = new QSpinBox(this);
    d->focalLength35mmEdit->setMinimum(1);
    d->focalLength35mmEdit->setMaximum(10000);
    d->focalLength35mmEdit->setSingleStep(1);
    d->focalLength35mmEdit->setValue(1);
    d->focalLength35mmEdit->setWhatsThis(i18n("Set here equivalent focal length assuming "
                                              "a 35mm film camera, in mm. A value of 0 means the focal "
                                              "length is unknown."));

    // --------------------------------------------------------

    d->digitalZoomRatioCheck = new QCheckBox(i18n("Digital zoom ratio:"), this);
    d->digitalZoomRatioEdit  = new QDoubleSpinBox(this);
    d->digitalZoomRatioEdit->setRange(0.0, 100.0);
    d->digitalZoomRatioEdit->setSingleStep(0.1);
    d->digitalZoomRatioEdit->setValue(1.0);
    d->digitalZoomRatioEdit->setDecimals(1);
    d->digitalZoomRatioEdit->setWhatsThis(i18n("Set here the digital zoom ratio "
                                               "used by camera to take the picture."));

    // --------------------------------------------------------

    d->apertureCheck = new MetadataCheckBox(i18n("Lens aperture (f-number):"), this);
    d->apertureCB    = new QComboBox(this);
    d->apertureCB->insertItems(0, d->apertureValues);
    d->apertureCB->setWhatsThis(i18n("Select here the lens aperture used by camera "
                                     "to take the picture."));

    // --------------------------------------------------------

    d->maxApertureCheck = new MetadataCheckBox(i18n("Max. lens aperture (f-number):"), this);
    d->maxApertureCB    = new QComboBox(this);
    d->maxApertureCB->insertItems(0, d->apertureValues);
    d->maxApertureCB->setWhatsThis(i18n("Select here the smallest aperture of the lens used by camera "
                                        "to take the picture."));

    grid->addWidget(d->focalLengthCheck,        0, 0, 1, 1);
    grid->addWidget(d->focalLengthEdit,         0, 2, 1, 1);
    grid->addWidget(d->focalLength35mmCheck,    1, 0, 1, 1);
    grid->addWidget(d->focalLength35mmEdit,     1, 2, 1, 1);
    grid->addWidget(d->digitalZoomRatioCheck,   2, 0, 1, 1);
    grid->addWidget(d->digitalZoomRatioEdit,    2, 2, 1, 1);
    grid->addWidget(d->apertureCheck,           3, 0, 1, 1);
    grid->addWidget(d->apertureCB,              3, 2, 1, 1);
    grid->addWidget(d->maxApertureCheck,        4, 0, 1, 1);
    grid->addWidget(d->maxApertureCB,           4, 2, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(5, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->focalLengthCheck, SIGNAL(toggled(bool)),
            d->focalLengthEdit, SLOT(setEnabled(bool)));

    connect(d->focalLength35mmCheck, SIGNAL(toggled(bool)),
            d->focalLength35mmEdit, SLOT(setEnabled(bool)));

    connect(d->digitalZoomRatioCheck, SIGNAL(toggled(bool)),
            d->digitalZoomRatioEdit, SLOT(setEnabled(bool)));

    connect(d->apertureCheck, SIGNAL(toggled(bool)),
            d->apertureCB, SLOT(setEnabled(bool)));

    connect(d->maxApertureCheck, SIGNAL(toggled(bool)),
            d->maxApertureCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->focalLength35mmCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->focalLengthCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->digitalZoomRatioCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->apertureCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->maxApertureCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->apertureCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->maxApertureCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->focalLength35mmEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->focalLengthEdit, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalModified()));

    connect(d->digitalZoomRatioEdit, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalModified()));
}

EXIFLens::~EXIFLens()
{
    delete d;
}

void EXIFLens::readMetadata(QByteArray& exifData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setExif(exifData);
    long int num=1, den=1;
    long     val=0;

    d->focalLengthEdit->setValue(50.0);
    d->focalLengthCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.FocalLength", num, den))
    {
        d->focalLengthEdit->setValue((double)(num) / (double)(den));
        d->focalLengthCheck->setChecked(true);
    }

    d->focalLengthEdit->setEnabled(d->focalLengthCheck->isChecked());

    d->focalLength35mmEdit->setValue(10);
    d->focalLength35mmCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.FocalLengthIn35mmFilm", val))
    {
        d->focalLength35mmEdit->setValue(val);
        d->focalLength35mmCheck->setChecked(true);
    }

    d->focalLength35mmEdit->setEnabled(d->focalLength35mmCheck->isChecked());

    d->digitalZoomRatioEdit->setValue(1.0);
    d->digitalZoomRatioCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.DigitalZoomRatio", num, den))
    {
        d->digitalZoomRatioEdit->setValue((num == 0) ? 0.0 : (double)(num) / (double)(den));
        d->digitalZoomRatioCheck->setChecked(true);
    }

    d->digitalZoomRatioEdit->setEnabled(d->digitalZoomRatioCheck->isChecked());

    d->apertureCB->setCurrentIndex(0);
    d->apertureCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.FNumber", num, den))
    {
        QString fnumber = QString::number((double)(num)/(double)(den), 'f', 1);
        int item        = -1;

        for (int i = 0 ; i < d->apertureCB->count() ; ++i)
        {
            if (d->apertureCB->itemText(i).remove(0, 2) == fnumber)
                item = i;
        }

        if (item != -1)
        {
            d->apertureCB->setCurrentIndex(item);
            d->apertureCheck->setChecked(true);
        }
    }
    else if (meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))
    {
        double aperture = std::pow(2.0, ((double)(num)/(double)(den))/2.0);
        QString fnumber = QString::number(aperture, 'f', 1);
        int item        = -1;

        for (int i = 0 ; i < d->apertureCB->count() ; ++i)
        {
            if (d->apertureCB->itemText(i).remove(0, 2) == fnumber)
                item = i;
        }

        if (item != -1)
        {
            d->apertureCB->setCurrentIndex(item);
            d->apertureCheck->setChecked(true);
        }
        else
            d->apertureCheck->setValid(false);
    }
    d->apertureCB->setEnabled(d->apertureCheck->isChecked());

    d->maxApertureCB->setCurrentIndex(0);
    d->maxApertureCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.MaxApertureValue", num, den))
    {
        double maxAperture = std::pow(2.0, ((double)(num)/(double)(den))/2.0);
        QString fnumber    = QString::number(maxAperture, 'f', 1);
        int item           = -1;

        for (int i = 0 ; i < d->apertureCB->count() ; ++i)
        {
            if (d->maxApertureCB->itemText(i).remove(0, 2) == fnumber)
                item = i;
        }

        if (item != -1)
        {
            d->maxApertureCB->setCurrentIndex(item);
            d->maxApertureCheck->setChecked(true);
        }
        else
        {
            d->maxApertureCheck->setValid(false);
        }
    }

    d->maxApertureCB->setEnabled(d->maxApertureCheck->isChecked());

    blockSignals(false);
}

void EXIFLens::applyMetadata(QByteArray& exifData)
{
    DMetadata meta;
    meta.setExif(exifData);
    long int num=1, den=1;

    if (d->focalLengthCheck->isChecked())
    {
        meta.convertToRational(d->focalLengthEdit->value(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.FocalLength", num, den);
    }
    else
        meta.removeExifTag("Exif.Photo.FocalLength");

    if (d->focalLength35mmCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.FocalLengthIn35mmFilm", d->focalLength35mmEdit->value());
    else
        meta.removeExifTag("Exif.Photo.FocalLengthIn35mmFilm");

    if (d->digitalZoomRatioCheck->isChecked())
    {
        meta.convertToRational(d->digitalZoomRatioEdit->value(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.DigitalZoomRatio", num, den);
    }
    else
        meta.removeExifTag("Exif.Photo.DigitalZoomRatio");

    if (d->apertureCheck->isChecked())
    {
        meta.convertToRational(d->apertureCB->currentText().remove(0, 2).toDouble(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.FNumber", num, den);

        double fnumber  = d->apertureCB->currentText().remove(0, 2).toDouble();
        double aperture = 2.0*(std::log(fnumber)/std::log(2.0));
        meta.convertToRational(aperture, &num, &den, 8);
        meta.setExifTagRational("Exif.Photo.ApertureValue", num, den);
    }
    else if (d->apertureCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.FNumber");
        meta.removeExifTag("Exif.Photo.ApertureValue");
    }

    if (d->maxApertureCheck->isChecked())
    {
        double fnumber  = d->maxApertureCB->currentText().remove(0, 2).toDouble();
        double aperture = 2.0*(std::log(fnumber)/std::log(2.0));
        meta.convertToRational(aperture, &num, &den, 8);
        meta.setExifTagRational("Exif.Photo.MaxApertureValue", num, den);
    }
    else if (d->maxApertureCheck->isValid())
        meta.removeExifTag("Exif.Photo.MaxApertureValue");

    exifData = meta.getExifEncoded();
}

}  // namespace Digikam
