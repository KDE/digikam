/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP origin settings page.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "xmporigin.h"

// Qt includes

#include <QCheckBox>
#include <QMap>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "squeezedcombobox.h"
#include "dlayoutbox.h"
#include "metadatacheckbox.h"
#include "timezonecombobox.h"
#include "dmetadata.h"
#include "countryselector.h"
#include "dexpanderbox.h"

namespace Digikam
{

class XMPOrigin::Private
{
public:

    Private()
    {
        cityEdit               = 0;
        sublocationEdit        = 0;
        provinceEdit           = 0;
        cityCheck              = 0;
        sublocationCheck       = 0;
        provinceCheck          = 0;
        countryCheck           = 0;
        dateCreatedSel         = 0;
        dateDigitalizedSel     = 0;
        zoneCreatedSel         = 0;
        zoneDigitalizedSel     = 0;
        dateCreatedCheck       = 0;
        dateDigitalizedCheck   = 0;
        syncEXIFDateCheck      = 0;
        setTodayCreatedBtn     = 0;
        setTodayDigitalizedBtn = 0;
        countryCB              = 0;
    }

    QCheckBox*                     dateCreatedCheck;
    QCheckBox*                     dateDigitalizedCheck;
    QCheckBox*                     syncEXIFDateCheck;
    QCheckBox*                     cityCheck;
    QCheckBox*                     sublocationCheck;
    QCheckBox*                     provinceCheck;

    QPushButton*                   setTodayCreatedBtn;
    QPushButton*                   setTodayDigitalizedBtn;

    QDateTimeEdit*                 dateCreatedSel;
    QDateTimeEdit*                 dateDigitalizedSel;

    TimeZoneComboBox*              zoneCreatedSel;
    TimeZoneComboBox*              zoneDigitalizedSel;

    QLineEdit*                     cityEdit;
    QLineEdit*                     sublocationEdit;
    QLineEdit*                     provinceEdit;

    MetadataCheckBox*              countryCheck;

    CountrySelector*               countryCB;
};

XMPOrigin::XMPOrigin(QWidget* const parent)
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

    d->dateDigitalizedCheck   = new QCheckBox(i18n("Digitization date"), this);
    d->zoneDigitalizedSel     = new TimeZoneComboBox(this);

    d->dateDigitalizedSel     = new QDateTimeEdit(this);
    d->dateDigitalizedSel->setDisplayFormat(dateTimeFormat);

    d->setTodayDigitalizedBtn = new QPushButton();
    d->setTodayDigitalizedBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayDigitalizedBtn->setWhatsThis(i18n("Set digitization date to today"));

    d->dateDigitalizedSel->setWhatsThis(i18n("Set here the creation date of "
                                             "digital representation."));
    d->zoneDigitalizedSel->setWhatsThis(i18n("Set here the time zone of "
                                             "digital representation."));

    slotSetTodayDigitalized();

    // --------------------------------------------------------

    d->dateCreatedCheck   = new QCheckBox(i18n("Creation date"), this);
    d->zoneCreatedSel     = new TimeZoneComboBox(this);

    d->dateCreatedSel     = new QDateTimeEdit(this);
    d->dateCreatedSel->setDisplayFormat(dateTimeFormat);

    d->syncEXIFDateCheck  = new QCheckBox(i18n("Sync EXIF creation date"), this);

    d->setTodayCreatedBtn = new QPushButton();
    d->setTodayCreatedBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayCreatedBtn->setWhatsThis(i18n("Set creation date to today"));

    d->dateCreatedSel->setWhatsThis(i18n("Set here the creation date of "
                                         "intellectual content."));
    d->zoneCreatedSel->setWhatsThis(i18n("Set here the time zone of "
                                         "intellectual content."));

    slotSetTodayCreated();

    // --------------------------------------------------------

    d->cityCheck = new QCheckBox(i18n("City:"), this);
    d->cityEdit  = new QLineEdit(this);
    d->cityEdit->setClearButtonEnabled(true);
    d->cityEdit->setWhatsThis(i18n("Set here the content's city of origin."));

    // --------------------------------------------------------

    d->sublocationCheck = new QCheckBox(i18n("Sublocation:"), this);
    d->sublocationEdit  = new QLineEdit(this);
    d->sublocationEdit->setClearButtonEnabled(true);
    d->sublocationEdit->setWhatsThis(i18n("Set here the content's location within the city."));

    // --------------------------------------------------------

    d->provinceCheck = new QCheckBox(i18n("State/Province:"), this);
    d->provinceEdit  = new QLineEdit(this);
    d->provinceEdit->setClearButtonEnabled(true);
    d->provinceEdit->setWhatsThis(i18n("Set here the content's Province or State of origin."));

    // --------------------------------------------------------

    d->countryCheck = new MetadataCheckBox(i18n("Country:"), this);
    d->countryCB    = new CountrySelector(this);
    d->countryCB->setWhatsThis(i18n("Select here country name of content origin."));
    // Remove 2 last items for the list (separator + Unknown item)
    d->countryCB->removeItem(d->countryCB->count()-1);
    d->countryCB->removeItem(d->countryCB->count()-1);

    // --------------------------------------------------------

    grid->addWidget(d->dateDigitalizedCheck,                0, 0, 1, 6);
    grid->addWidget(d->dateDigitalizedSel,                  1, 0, 1, 3);
    grid->addWidget(d->zoneDigitalizedSel,                  1, 3, 1, 1);
    grid->addWidget(d->setTodayDigitalizedBtn,              1, 5, 1, 1);
    grid->addWidget(d->dateCreatedCheck,                    2, 0, 1, 6);
    grid->addWidget(d->dateCreatedSel,                      3, 0, 1, 3);
    grid->addWidget(d->zoneCreatedSel,                      3, 3, 1, 1);
    grid->addWidget(d->setTodayCreatedBtn,                  3, 5, 1, 1);
    grid->addWidget(d->syncEXIFDateCheck,                   5, 0, 1, 6);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  6, 0, 1, 6);
    grid->addWidget(d->cityCheck,                           7, 0, 1, 1);
    grid->addWidget(d->cityEdit,                            7, 1, 1, 5);
    grid->addWidget(d->sublocationCheck,                    8, 0, 1, 1);
    grid->addWidget(d->sublocationEdit,                     8, 1, 1, 5);
    grid->addWidget(d->provinceCheck,                       9, 0, 1, 1);
    grid->addWidget(d->provinceEdit,                        9, 1, 1, 5);
    grid->addWidget(d->countryCheck,                       10, 0, 1, 1);
    grid->addWidget(d->countryCB,                          10, 1, 1, 5);
    grid->setColumnStretch(4, 10);
    grid->setRowStretch(11, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->dateCreatedSel, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            d->dateDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->zoneCreatedSel, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            d->zoneDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->syncEXIFDateCheck, SLOT(setEnabled(bool)));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            d->cityEdit, SLOT(setEnabled(bool)));

    connect(d->sublocationCheck, SIGNAL(toggled(bool)),
            d->sublocationEdit, SLOT(setEnabled(bool)));

    connect(d->provinceCheck, SIGNAL(toggled(bool)),
            d->provinceEdit, SLOT(setEnabled(bool)));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            d->countryCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->provinceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->dateCreatedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->zoneCreatedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->zoneDigitalizedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->setTodayCreatedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayCreated()));

    connect(d->setTodayDigitalizedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayDigitalized()));

    // --------------------------------------------------------

    connect(d->countryCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->cityEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->provinceEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

XMPOrigin::~XMPOrigin()
{
    delete d;
}

void XMPOrigin::slotSetTodayCreated()
{
    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->zoneCreatedSel->setToUTC();
}

void XMPOrigin::slotSetTodayDigitalized()
{
    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->zoneDigitalizedSel->setToUTC();
}

bool XMPOrigin::syncEXIFDateIsChecked() const
{
    return d->syncEXIFDateCheck->isChecked();
}

void XMPOrigin::setCheckedSyncEXIFDate(bool c)
{
    d->syncEXIFDateCheck->setChecked(c);
}

QDateTime XMPOrigin::getXMPCreationDate() const
{
    return d->dateCreatedSel->dateTime();
}

void XMPOrigin::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);

    QString     data;
    QStringList code, list;
    QDateTime   dateTime;
    QString     dateTimeStr;

    dateTimeStr = meta.getXmpTagString("Xmp.photoshop.DateCreated", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.CreateDate", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.ModifyDate", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.exif.DateTimeOriginal", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.tiff.DateTime", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.ModifyDate", false);

    if (dateTimeStr.isEmpty())
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.MetadataDate", false);

    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->dateCreatedCheck->setChecked(false);
    d->zoneCreatedSel->setToUTC();

    if (!dateTimeStr.isEmpty())
    {
        dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);

        if (dateTime.isValid())
        {
            d->dateCreatedSel->setDateTime(dateTime);
            d->dateCreatedCheck->setChecked(true);
            d->zoneCreatedSel->setTimeZone(dateTimeStr);
        }
    }

    d->dateCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->zoneCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->syncEXIFDateCheck->setEnabled(d->dateCreatedCheck->isChecked());

    dateTimeStr = meta.getXmpTagString("Xmp.exif.DateTimeDigitized", false);

    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->dateDigitalizedCheck->setChecked(false);
    d->zoneDigitalizedSel->setToUTC();

    if (!dateTimeStr.isEmpty())
    {
        dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);

        if (dateTime.isValid())
        {
            d->dateDigitalizedSel->setDateTime(dateTime);
            d->dateDigitalizedCheck->setChecked(true);
            d->zoneDigitalizedSel->setTimeZone(dateTimeStr);
        }
    }

    d->dateDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());
    d->zoneDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());

    d->cityEdit->clear();
    d->cityCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.City", false);

    if (!data.isNull())
    {
        d->cityEdit->setText(data);
        d->cityCheck->setChecked(true);
    }

    d->cityEdit->setEnabled(d->cityCheck->isChecked());

    d->sublocationEdit->clear();
    d->sublocationCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.Location", false);

    if (!data.isNull())
    {
        d->sublocationEdit->setText(data);
        d->sublocationCheck->setChecked(true);
    }

    d->sublocationEdit->setEnabled(d->sublocationCheck->isChecked());

    d->provinceEdit->clear();
    d->provinceCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.State", false);

    if (!data.isNull())
    {
        d->provinceEdit->setText(data);
        d->provinceCheck->setChecked(true);
    }

    d->provinceEdit->setEnabled(d->provinceCheck->isChecked());

    d->countryCB->setCurrentIndex(0);
    d->countryCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CountryCode", false);

    if (!data.isNull())
    {
        int item = -1;

        for (int i = 0 ; i < d->countryCB->count() ; ++i)
        {
            if (d->countryCB->itemText(i).left(3) == data)
                item = i;
        }

        if (item != -1)
        {
            d->countryCB->setCurrentIndex(item);
            d->countryCheck->setChecked(true);
        }
        else
        {
            d->countryCheck->setValid(false);
        }
    }

    d->countryCB->setEnabled(d->countryCheck->isChecked());

    blockSignals(false);
}

void XMPOrigin::applyMetadata(QByteArray& exifData, QByteArray& xmpData)
{
    DMetadata meta;
    meta.setExif(exifData);
    meta.setXmp(xmpData);

    if (d->dateCreatedCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.DateCreated",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.CreateDate",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.exif.DateTimeOriginal",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.tiff.DateTime",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.ModifyDate",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.MetadataDate",
                                   getXMPCreationDate().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                                   d->zoneCreatedSel->getTimeZone());

        if (syncEXIFDateIsChecked())
        {
            meta.setExifTagString("Exif.Image.DateTime",
                    getXMPCreationDate().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.DateCreated");
        meta.removeXmpTag("Xmp.xmp.CreateDate");
        meta.removeXmpTag("Xmp.exif.DateTimeOriginal");
        meta.removeXmpTag("Xmp.tiff.DateTime");
        meta.removeXmpTag("Xmp.xmp.ModifyDate");
        meta.removeXmpTag("Xmp.xmp.MetadataDate");
    }

    if (d->dateDigitalizedCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.exif.DateTimeDigitized",
                             d->dateDigitalizedSel->dateTime().toString(QLatin1String("yyyy:MM:ddThh:mm:ss")) +
                             d->zoneDigitalizedSel->getTimeZone());
    }
    else
    {
        meta.removeXmpTag("Xmp.exif.DateTimeDigitized");
    }

    if (d->cityCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.City", d->cityEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.City");
    }

    if (d->sublocationCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.Location", d->sublocationEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.Location");
    }

    if (d->provinceCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.State", d->provinceEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.State");
    }

    if (d->countryCheck->isChecked())
    {
        QString countryName = d->countryCB->currentText().mid(6);
        QString countryCode = d->countryCB->currentText().left(3);
        meta.setXmpTagString("Xmp.iptc.CountryCode", countryCode);
        meta.setXmpTagString("Xmp.photoshop.Country", countryName);
    }
    else if (d->countryCheck->isValid())
    {
        meta.removeXmpTag("Xmp.iptc.CountryCode");
        meta.removeXmpTag("Xmp.photoshop.Country");
    }

    exifData = meta.getExifEncoded();
    xmpData  = meta.getXmp();
}

}  // namespace Digikam
