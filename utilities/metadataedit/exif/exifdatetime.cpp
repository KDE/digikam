/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF date and time settings page.
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

#include "exifdatetime.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QSpinBox>
#include <QDateTimeEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

namespace Digikam
{

class EXIFDateTime::Private
{
public:

    Private()
    {
        dateCreatedSel             = 0;
        dateOriginalSel            = 0;
        dateDigitalizedSel         = 0;
        dateCreatedSubSecEdit      = 0;
        dateOriginalSubSecEdit     = 0;
        dateDigitalizedSubSecEdit  = 0;

        dateCreatedCheck           = 0;
        dateOriginalCheck          = 0;
        dateDigitalizedCheck       = 0;
        dateCreatedSubSecCheck     = 0;
        dateOriginalSubSecCheck    = 0;
        dateDigitalizedSubSecCheck = 0;
        syncXMPDateCheck           = 0;
        syncIPTCDateCheck          = 0;

        setTodayCreatedBtn         = 0;
        setTodayOriginalBtn        = 0;
        setTodayDigitalizedBtn     = 0;
    }

    QCheckBox*       dateCreatedCheck;
    QCheckBox*       dateOriginalCheck;
    QCheckBox*       dateDigitalizedCheck;
    QCheckBox*       dateCreatedSubSecCheck;
    QCheckBox*       dateOriginalSubSecCheck;
    QCheckBox*       dateDigitalizedSubSecCheck;
    QCheckBox*       syncXMPDateCheck;
    QCheckBox*       syncIPTCDateCheck;

    QPushButton*     setTodayCreatedBtn;
    QPushButton*     setTodayOriginalBtn;
    QPushButton*     setTodayDigitalizedBtn;

    QSpinBox*        dateCreatedSubSecEdit;
    QSpinBox*        dateOriginalSubSecEdit;
    QSpinBox*        dateDigitalizedSubSecEdit;

    QDateTimeEdit*   dateCreatedSel;
    QDateTimeEdit*   dateOriginalSel;
    QDateTimeEdit*   dateDigitalizedSel;
};

EXIFDateTime::EXIFDateTime(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    QString dateTimeFormat  = QLocale().dateTimeFormat(QLocale::ShortFormat);

    if (!dateTimeFormat.contains(QLatin1String("yyyy")))
    {
        dateTimeFormat.replace(QLatin1String("yy"),
                               QLatin1String("yyyy"));
    }

    if (!dateTimeFormat.contains(QLatin1String("ss")))
    {
        dateTimeFormat.replace(QLatin1String("mm"),
                               QLatin1String("mm:ss"));
    }

    // --------------------------------------------------------

    d->dateCreatedCheck       = new QCheckBox(i18n("Creation date and time"), this);
    d->dateCreatedSubSecCheck = new QCheckBox(i18n("Creation sub-second"), this);

    d->dateCreatedSel         = new QDateTimeEdit(this);
    d->dateCreatedSel->setDisplayFormat(dateTimeFormat);

    d->dateCreatedSubSecEdit  = new QSpinBox(this);
    d->dateCreatedSubSecEdit->setMinimum(0);
    d->dateCreatedSubSecEdit->setMaximum(999);
    d->dateCreatedSubSecEdit->setSingleStep(1);
    d->dateCreatedSubSecEdit->setValue(0);

    d->syncXMPDateCheck       = new QCheckBox(i18n("Sync XMP creation date"), this);
    d->syncIPTCDateCheck      = new QCheckBox(i18n("Sync IPTC creation date"), this);

    d->setTodayCreatedBtn     = new QPushButton();
    d->setTodayCreatedBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayCreatedBtn->setWhatsThis(i18n("Set creation date to today"));

    if (!DMetadata::supportXmp())
        d->syncXMPDateCheck->setEnabled(false);

    d->dateCreatedSel->setWhatsThis(i18n("Set here the date and time of image creation. "
                                       "In this standard it is the date and time the file was changed."));
    d->dateCreatedSubSecEdit->setWhatsThis(i18n("Set here the fractions of seconds for the date "
                                                "and time of image creation."));

    slotSetTodayCreated();

    // --------------------------------------------------------

    d->dateOriginalCheck       = new QCheckBox(i18n("Original date and time"), this);
    d->dateOriginalSubSecCheck = new QCheckBox(i18n("Original sub-second"), this);

    d->dateOriginalSel         = new QDateTimeEdit(this);
    d->dateOriginalSel->setDisplayFormat(dateTimeFormat);

    d->dateOriginalSubSecEdit  = new QSpinBox(this);
    d->dateOriginalSubSecEdit->setMinimum(0);
    d->dateOriginalSubSecEdit->setMaximum(999);
    d->dateOriginalSubSecEdit->setSingleStep(1);
    d->dateOriginalSubSecEdit->setValue(0);

    d->setTodayOriginalBtn     = new QPushButton();
    d->setTodayOriginalBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayOriginalBtn->setWhatsThis(i18n("Set original date to today"));

    d->dateOriginalSel->setWhatsThis(i18n("Set here the date and time when the original image "
                                          "data was generated. For a digital still camera the date and "
                                          "time the picture was taken are recorded."));
    d->dateOriginalSubSecEdit->setWhatsThis(i18n("Set here the fractions of seconds for the date "
                                                 "and time when the original image data was generated."));

    slotSetTodayOriginal();

    // --------------------------------------------------------

    d->dateDigitalizedCheck       = new QCheckBox(i18n("Digitization date and time"), this);
    d->dateDigitalizedSubSecCheck = new QCheckBox(i18n("Digitization sub-second"), this);

    d->dateDigitalizedSel         = new QDateTimeEdit(this);
    d->dateDigitalizedSel->setDisplayFormat(dateTimeFormat);

    d->dateDigitalizedSubSecEdit  = new QSpinBox(this);
    d->dateDigitalizedSubSecEdit->setMinimum(0);
    d->dateDigitalizedSubSecEdit->setMaximum(999);
    d->dateDigitalizedSubSecEdit->setSingleStep(1);
    d->dateDigitalizedSubSecEdit->setValue(0);

    d->setTodayDigitalizedBtn     = new QPushButton();
    d->setTodayDigitalizedBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayDigitalizedBtn->setWhatsThis(i18n("Set digitization date to today"));

    d->dateDigitalizedSel->setWhatsThis(i18n("Set here the date and time when the image was "
                                             "stored as digital data. If, for example, an image was "
                                             "captured by a digital still camera and at the same "
                                             "time the file was recorded, then Original and Digitization "
                                             "date and time will have the same contents."));
    d->dateDigitalizedSubSecEdit->setWhatsThis(i18n("Set here the fractions of seconds for the date "
                                                    "and time when the image was stored as digital data."));

    slotSetTodayDigitalized();

    // --------------------------------------------------------

    grid->addWidget(d->dateCreatedCheck,                    0, 0, 1, 1);
    grid->addWidget(d->dateCreatedSubSecCheck,              0, 1, 1, 3);
    grid->addWidget(d->dateCreatedSel,                      1, 0, 1, 1);
    grid->addWidget(d->dateCreatedSubSecEdit,               1, 1, 1, 1);
    grid->addWidget(d->setTodayCreatedBtn,                  1, 3, 1, 1);
    grid->addWidget(d->syncXMPDateCheck,                    2, 0, 1, 4);
    grid->addWidget(d->syncIPTCDateCheck,                   3, 0, 1, 4);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  4, 0, 1, 4);
    grid->addWidget(d->dateOriginalCheck,                   5, 0, 1, 1);
    grid->addWidget(d->dateOriginalSubSecCheck,             5, 1, 1, 3);
    grid->addWidget(d->dateOriginalSel,                     6, 0, 1, 1);
    grid->addWidget(d->dateOriginalSubSecEdit,              6, 1, 1, 1);
    grid->addWidget(d->setTodayOriginalBtn,                 6, 3, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  7, 0, 1, 4);
    grid->addWidget(d->dateDigitalizedCheck,                8, 0, 1, 1);
    grid->addWidget(d->dateDigitalizedSubSecCheck,          8, 1, 1, 3);
    grid->addWidget(d->dateDigitalizedSel,                  9, 0, 1, 1);
    grid->addWidget(d->dateDigitalizedSubSecEdit,           9, 1, 1, 1);
    grid->addWidget(d->setTodayDigitalizedBtn,              9, 3, 1, 1);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(10, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->dateCreatedSel, SLOT(setEnabled(bool)));

    connect(d->dateOriginalCheck, SIGNAL(toggled(bool)),
            d->dateOriginalSel, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            d->dateDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->dateCreatedSubSecCheck, SIGNAL(toggled(bool)),
            d->dateCreatedSubSecEdit, SLOT(setEnabled(bool)));

    connect(d->dateOriginalSubSecCheck, SIGNAL(toggled(bool)),
            d->dateOriginalSubSecEdit, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedSubSecCheck, SIGNAL(toggled(bool)),
            d->dateDigitalizedSubSecEdit, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->syncXMPDateCheck, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->syncIPTCDateCheck, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateOriginalCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateCreatedSubSecCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateOriginalSubSecCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSubSecCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->dateCreatedSubSecEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->dateOriginalSubSecEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSubSecEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->dateCreatedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->dateOriginalSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->setTodayCreatedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayCreated()));

    connect(d->setTodayOriginalBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayOriginal()));

    connect(d->setTodayDigitalizedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayDigitalized()));
}

EXIFDateTime::~EXIFDateTime()
{
    delete d;
}

void EXIFDateTime::slotSetTodayCreated()
{
    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->dateCreatedSubSecEdit->setValue(0);
}

void EXIFDateTime::slotSetTodayOriginal()
{
    d->dateOriginalSel->setDateTime(QDateTime::currentDateTime());
    d->dateOriginalSubSecEdit->setValue(0);
}

void EXIFDateTime::slotSetTodayDigitalized()
{
    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->dateDigitalizedSubSecEdit->setValue(0);
}

bool EXIFDateTime::syncXMPDateIsChecked() const
{
    return d->syncXMPDateCheck->isChecked();
}

bool EXIFDateTime::syncIPTCDateIsChecked() const
{
    return d->syncIPTCDateCheck->isChecked();
}

void EXIFDateTime::setCheckedSyncXMPDate(bool c)
{
    d->syncXMPDateCheck->setChecked(c);
}

void EXIFDateTime::setCheckedSyncIPTCDate(bool c)
{
    d->syncIPTCDateCheck->setChecked(c);
}

QDateTime EXIFDateTime::getEXIFCreationDate() const
{
    return d->dateCreatedSel->dateTime();
}

void EXIFDateTime::readMetadata(QByteArray& exifData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setExif(exifData);

    QDateTime datetime;
    QString datetimeStr, data;

    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->dateCreatedCheck->setChecked(false);
    datetimeStr = meta.getExifTagString("Exif.Image.DateTime", false);

    if (!datetimeStr.isEmpty())
    {
        datetime = QDateTime::fromString(datetimeStr, Qt::ISODate);

        if (datetime.isValid())
        {
            d->dateCreatedSel->setDateTime(datetime);
            d->dateCreatedCheck->setChecked(true);
        }
    }

    d->dateCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->syncXMPDateCheck->setEnabled(d->dateCreatedCheck->isChecked());
    d->syncIPTCDateCheck->setEnabled(d->dateCreatedCheck->isChecked());

    d->dateCreatedSubSecEdit->setValue(0);
    d->dateCreatedSubSecCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Photo.SubSecTime", false);

    if (!data.isNull())
    {
        bool ok    = false;
        int subsec = data.toInt(&ok);

        if (ok)
        {
            d->dateCreatedSubSecEdit->setValue(subsec);
            d->dateCreatedSubSecCheck->setChecked(true);
        }
    }

    d->dateCreatedSubSecEdit->setEnabled(d->dateCreatedSubSecCheck->isChecked());

    d->dateOriginalSel->setDateTime(QDateTime::currentDateTime());
    d->dateOriginalCheck->setChecked(false);
    datetimeStr = meta.getExifTagString("Exif.Photo.DateTimeOriginal", false);

    if (!datetimeStr.isEmpty())
    {
        datetime = QDateTime::fromString(datetimeStr, Qt::ISODate);

        if (datetime.isValid())
        {
            d->dateOriginalSel->setDateTime(datetime);
            d->dateOriginalCheck->setChecked(true);
        }
    }

    d->dateOriginalSel->setEnabled(d->dateOriginalCheck->isChecked());

    d->dateOriginalSubSecEdit->setValue(0);
    d->dateOriginalSubSecCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Photo.SubSecTimeOriginal", false);

    if (!data.isNull())
    {
        bool ok    = false;
        int subsec = data.toInt(&ok);

        if (ok)
        {
            d->dateOriginalSubSecEdit->setValue(subsec);
            d->dateOriginalSubSecCheck->setChecked(true);
        }
    }

    d->dateOriginalSubSecEdit->setEnabled(d->dateOriginalSubSecCheck->isChecked());

    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->dateDigitalizedCheck->setChecked(false);
    datetimeStr = meta.getExifTagString("Exif.Photo.DateTimeDigitized", false);

    if (!datetimeStr.isEmpty())
    {
        datetime = QDateTime::fromString(datetimeStr, Qt::ISODate);

        if (datetime.isValid())
        {
            d->dateDigitalizedSel->setDateTime(datetime);
            d->dateDigitalizedCheck->setChecked(true);
        }
    }

    d->dateDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());

    d->dateDigitalizedSubSecEdit->setValue(0);
    d->dateDigitalizedSubSecCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Photo.SubSecTimeDigitized", false);

    if (!data.isNull())
    {
        bool ok    = false;
        int subsec = data.toInt(&ok);

        if (ok)
        {
            d->dateDigitalizedSubSecEdit->setValue(subsec);
            d->dateDigitalizedSubSecCheck->setChecked(true);
        }
    }

    d->dateDigitalizedSubSecEdit->setEnabled(d->dateDigitalizedSubSecCheck->isChecked());

    blockSignals(false);
}

void EXIFDateTime::applyMetadata(QByteArray& exifData, QByteArray& iptcData, QByteArray& xmpData)
{
    DMetadata meta;
    meta.setExif(exifData);
    meta.setIptc(iptcData);
    meta.setXmp(xmpData);

    if (d->dateCreatedCheck->isChecked())
    {
        meta.setExifTagString("Exif.Image.DateTime",
                   d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));

        if (meta.supportXmp() && d->syncXMPDateCheck->isChecked())
        {
            meta.setXmpTagString("Xmp.exif.DateTimeOriginal",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.photoshop.DateCreated",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.tiff.DateTime",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.CreateDate",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.MetadataDate",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.ModifyDate",
                d->dateCreatedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
        }

        if (syncIPTCDateIsChecked())
        {
            meta.setIptcTagString("Iptc.Application2.DateCreated",
                       d->dateCreatedSel->dateTime().date().toString(Qt::ISODate));
            meta.setIptcTagString("Iptc.Application2.TimeCreated",
                       d->dateCreatedSel->dateTime().time().toString(Qt::ISODate));
        }
    }
    else
        meta.removeExifTag("Exif.Image.DateTime");

    if (d->dateCreatedSubSecCheck->isChecked())
        meta.setExifTagString("Exif.Photo.SubSecTime",
                   QString::number(d->dateCreatedSubSecEdit->value()));
    else
        meta.removeExifTag("Exif.Photo.SubSecTime");

    if (d->dateOriginalCheck->isChecked())
        meta.setExifTagString("Exif.Photo.DateTimeOriginal",
                   d->dateOriginalSel->dateTime().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
    else
        meta.removeExifTag("Exif.Photo.DateTimeOriginal");

    if (d->dateOriginalSubSecCheck->isChecked())
        meta.setExifTagString("Exif.Photo.SubSecTimeOriginal",
                   QString::number(d->dateOriginalSubSecEdit->value()));
    else
        meta.removeExifTag("Exif.Photo.SubSecTimeOriginal");

    if (d->dateDigitalizedCheck->isChecked())
        meta.setExifTagString("Exif.Photo.DateTimeDigitized",
                   d->dateDigitalizedSel->dateTime().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
    else
        meta.removeExifTag("Exif.Photo.DateTimeDigitized");

    if (d->dateDigitalizedSubSecCheck->isChecked())
        meta.setExifTagString("Exif.Photo.SubSecTimeDigitized",
                   QString::number(d->dateDigitalizedSubSecEdit->value()));
    else
        meta.removeExifTag("Exif.Photo.SubSecTimeDigitized");

    exifData = meta.getExifEncoded();
    iptcData = meta.getIptc();
    xmpData  = meta.getXmp();
}

}  // namespace Digikam
