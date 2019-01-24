/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-10
 * Description : IPTC workflow status properties settings page.
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iptcproperties.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTimeEdit>
#include <QValidator>
#include <QGridLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "metadatacheckbox.h"
#include "timezonecombobox.h"
#include "objectattributesedit.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

using namespace Digikam;

namespace GenericDigikamMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCProperties::Private
{
public:

    explicit Private()
    {
        dateReleasedSel     = 0;
        dateExpiredSel      = 0;
        timeReleasedSel     = 0;
        timeExpiredSel      = 0;
        zoneReleasedSel     = 0;
        zoneExpiredSel      = 0;
        dateReleasedCheck   = 0;
        dateExpiredCheck    = 0;
        timeReleasedCheck   = 0;
        timeExpiredCheck    = 0;
        setTodayReleasedBtn = 0;
        setTodayExpiredBtn  = 0;
        priorityCB          = 0;
        priorityCheck       = 0;
        objectCycleCB       = 0;
        objectTypeCB        = 0;
        objectCycleCheck    = 0;
        objectTypeCheck     = 0;
        objectAttribute     = 0;
        languageBtn         = 0;
        languageCheck       = 0;
        originalTransCheck  = 0;
        originalTransEdit   = 0;
        objectTypeDescEdit  = 0;
    }

    QCheckBox*                     dateReleasedCheck;
    QCheckBox*                     timeReleasedCheck;
    QCheckBox*                     dateExpiredCheck;
    QCheckBox*                     timeExpiredCheck;
    QCheckBox*                     originalTransCheck;

    QTimeEdit*                     timeReleasedSel;
    QTimeEdit*                     timeExpiredSel;

    TimeZoneComboBox*              zoneReleasedSel;
    TimeZoneComboBox*              zoneExpiredSel;

    QPushButton*                   setTodayReleasedBtn;
    QPushButton*                   setTodayExpiredBtn;

    QComboBox*                     priorityCB;
    QComboBox*                     objectCycleCB;
    QComboBox*                     objectTypeCB;

    QLineEdit*                     objectTypeDescEdit;
    QLineEdit*                     originalTransEdit;

    QComboBox*                     languageBtn;

    QDateEdit*                     dateReleasedSel;
    QDateEdit*                     dateExpiredSel;

    MetadataCheckBox*              priorityCheck;
    MetadataCheckBox*              objectCycleCheck;
    MetadataCheckBox*              objectTypeCheck;
    MetadataCheckBox*              languageCheck;

    ObjectAttributesEdit*          objectAttribute;
};

IPTCProperties::IPTCProperties(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegExpValidator(asciiRx, this);

    QString dateFormat  = QLocale().dateFormat(QLocale::ShortFormat);

    if (!dateFormat.contains(QLatin1String("yyyy")))
    {
        dateFormat.replace(QLatin1String("yy"),
                           QLatin1String("yyyy"));
    }

    QString timeFormat = QLatin1String("hh:mm:ss");

    // --------------------------------------------------------

    d->dateReleasedCheck   = new QCheckBox(i18n("Release date"), this);
    d->timeReleasedCheck   = new QCheckBox(i18n("Release time"), this);
    d->zoneReleasedSel     = new TimeZoneComboBox(this);

    d->dateReleasedSel     = new QDateEdit(this);
    d->dateReleasedSel->setDisplayFormat(dateFormat);

    d->timeReleasedSel     = new QTimeEdit(this);
    d->timeReleasedSel->setDisplayFormat(timeFormat);

    d->setTodayReleasedBtn = new QPushButton();
    d->setTodayReleasedBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayReleasedBtn->setWhatsThis(i18n("Set release date to today"));

    d->dateReleasedSel->setWhatsThis(i18n("Set here the earliest intended usable date of "
                                          "intellectual content."));
    d->timeReleasedSel->setWhatsThis(i18n("Set here the earliest intended usable time of "
                                          "intellectual content."));
    d->zoneReleasedSel->setWhatsThis(i18n("Set here the earliest intended usable time zone of "
                                          "intellectual content."));

    slotSetTodayReleased();

    // --------------------------------------------------------

    d->dateExpiredCheck   = new QCheckBox(i18n("Expiration date"), this);
    d->timeExpiredCheck   = new QCheckBox(i18n("Expiration time"), this);
    d->zoneExpiredSel     = new TimeZoneComboBox(this);

    d->dateExpiredSel     = new QDateEdit(this);
    d->dateExpiredSel->setDisplayFormat(dateFormat);

    d->timeExpiredSel     = new QTimeEdit(this);
    d->timeExpiredSel->setDisplayFormat(timeFormat);

    d->setTodayExpiredBtn = new QPushButton();
    d->setTodayExpiredBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->setTodayExpiredBtn->setWhatsThis(i18n("Set expiration date to today"));

    d->dateExpiredSel->setWhatsThis(i18n("Set here the latest intended usable date of "
                                         "intellectual content."));
    d->timeExpiredSel->setWhatsThis(i18n("Set here the latest intended usable time of "
                                         "intellectual content."));
    d->zoneExpiredSel->setWhatsThis(i18n("Set here the latest intended usable time zone of "
                                         "intellectual content."));

    slotSetTodayExpired();

    // --------------------------------------------------------

    d->languageCheck = new MetadataCheckBox(i18n("Language:"), this);
    d->languageBtn   = new QComboBox(this);

    DMetadata::CountryCodeMap map = DMetadata::countryCodeMap();

    for (DMetadata::CountryCodeMap::Iterator it = map.begin() ; it != map.end() ; ++it)
    {
        d->languageBtn->addItem(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()), it.key());
    }

    d->languageBtn->setWhatsThis(i18n("Select here the language of content."));

    // --------------------------------------------------------

    d->priorityCheck = new MetadataCheckBox(i18n("Priority:"), this);
    d->priorityCB    = new QComboBox(this);
    d->priorityCB->insertItem(0, i18nc("editorial urgency of content", "0: None"));
    d->priorityCB->insertItem(1, i18nc("editorial urgency of content", "1: High"));
    d->priorityCB->insertItem(2, QLatin1String("2"));
    d->priorityCB->insertItem(3, QLatin1String("3"));
    d->priorityCB->insertItem(4, QLatin1String("4"));
    d->priorityCB->insertItem(5, i18nc("editorial urgency of content", "5: Normal"));
    d->priorityCB->insertItem(6, QLatin1String("6"));
    d->priorityCB->insertItem(7, QLatin1String("7"));
    d->priorityCB->insertItem(8, i18nc("editorial urgency of content", "8: Low"));
    d->priorityCB->insertItem(9, i18nc("editorial urgency of content", "9: User-defined"));
    d->priorityCB->setWhatsThis(i18n("Select here the editorial urgency of content."));

    // --------------------------------------------------------

    d->objectCycleCheck = new MetadataCheckBox(i18n("Cycle:"), this);
    d->objectCycleCB    = new QComboBox(this);
    d->objectCycleCB->insertItem(0, i18n("Morning"));
    d->objectCycleCB->insertItem(1, i18n("Afternoon"));
    d->objectCycleCB->insertItem(2, i18n("Evening"));
    d->objectCycleCB->setWhatsThis(i18n("Select here the editorial cycle of content."));

    // --------------------------------------------------------

    d->objectTypeCheck    = new MetadataCheckBox(i18n("Type:"), this);
    d->objectTypeCB       = new QComboBox(this);
    d->objectTypeDescEdit = new QLineEdit(this);
    d->objectTypeDescEdit->setClearButtonEnabled(true);
    d->objectTypeDescEdit->setValidator(asciiValidator);
    d->objectTypeDescEdit->setMaxLength(64);
    d->objectTypeCB->insertItem(0, i18n("News"));
    d->objectTypeCB->insertItem(1, i18n("Data"));
    d->objectTypeCB->insertItem(2, i18n("Advisory"));
    d->objectTypeCB->setWhatsThis(i18n("Select here the editorial type of content."));
    d->objectTypeDescEdit->setWhatsThis(i18n("Set here the editorial type description of content. "
                                             "This field is limited to 64 ASCII characters."));

    // --------------------------------------------------------

    d->objectAttribute = new ObjectAttributesEdit(this, true, 64);

    // --------------------------------------------------------

    d->originalTransCheck = new QCheckBox(i18n("Reference:"), this);
    d->originalTransEdit  = new QLineEdit(this);
    d->originalTransEdit->setClearButtonEnabled(true);
    d->originalTransEdit->setValidator(asciiValidator);
    d->originalTransEdit->setMaxLength(32);
    d->originalTransEdit->setWhatsThis(i18n("Set here the original content transmission "
                                            "reference. This field is limited to 32 ASCII characters."));

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<b><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a></b> "
                 "text tags only support the printable "
                 "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters and limit string sizes. "
                 "Use contextual help for details.</b>"), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->dateReleasedCheck,                   0, 0, 1, 2);
    grid->addWidget(d->timeReleasedCheck,                   0, 2, 1, 2);
    grid->addWidget(d->dateReleasedSel,                     1, 0, 1, 2);
    grid->addWidget(d->timeReleasedSel,                     1, 2, 1, 1);
    grid->addWidget(d->zoneReleasedSel,                     1, 3, 1, 1);
    grid->addWidget(d->setTodayReleasedBtn,                 1, 5, 1, 1);
    grid->addWidget(d->dateExpiredCheck,                    2, 0, 1, 2);
    grid->addWidget(d->timeExpiredCheck,                    2, 2, 1, 2);
    grid->addWidget(d->dateExpiredSel,                      3, 0, 1, 2);
    grid->addWidget(d->timeExpiredSel,                      3, 2, 1, 1);
    grid->addWidget(d->zoneExpiredSel,                      3, 3, 1, 1);
    grid->addWidget(d->setTodayExpiredBtn,                  3, 5, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  4, 0, 1, 6);
    grid->addWidget(d->languageCheck,                       5, 0, 1, 1);
    grid->addWidget(d->languageBtn,                         5, 1, 1, 1);
    grid->addWidget(d->priorityCheck,                       6, 0, 1, 1);
    grid->addWidget(d->priorityCB,                          6, 1, 1, 1);
    grid->addWidget(d->objectCycleCheck,                    7, 0, 1, 1);
    grid->addWidget(d->objectCycleCB,                       7, 1, 1, 1);
    grid->addWidget(d->objectTypeCheck,                     8, 0, 1, 1);
    grid->addWidget(d->objectTypeCB,                        8, 1, 1, 1);
    grid->addWidget(d->objectTypeDescEdit,                  8, 2, 1, 4);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  9, 0, 1, 6);
    grid->addWidget(d->objectAttribute,                    10, 0, 1, 6);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this), 11, 0, 1, 6);
    grid->addWidget(d->originalTransCheck,                 12, 0, 1, 1);
    grid->addWidget(d->originalTransEdit,                  12, 1, 1, 5);
    grid->addWidget(note,                                  13, 0, 1, 6);
    grid->setColumnStretch(4, 10);
    grid->setRowStretch(14, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->dateReleasedCheck, SIGNAL(toggled(bool)),
            d->dateReleasedSel, SLOT(setEnabled(bool)));

    connect(d->dateExpiredCheck, SIGNAL(toggled(bool)),
            d->dateExpiredSel, SLOT(setEnabled(bool)));

    connect(d->timeReleasedCheck, SIGNAL(toggled(bool)),
            d->timeReleasedSel, SLOT(setEnabled(bool)));

    connect(d->timeExpiredCheck, SIGNAL(toggled(bool)),
            d->timeExpiredSel, SLOT(setEnabled(bool)));

    connect(d->timeReleasedCheck, SIGNAL(toggled(bool)),
            d->zoneReleasedSel, SLOT(setEnabled(bool)));

    connect(d->timeExpiredCheck, SIGNAL(toggled(bool)),
            d->zoneExpiredSel, SLOT(setEnabled(bool)));

    connect(d->languageCheck, SIGNAL(toggled(bool)),
            d->languageBtn, SLOT(setEnabled(bool)));

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            d->priorityCB, SLOT(setEnabled(bool)));

    connect(d->objectCycleCheck, SIGNAL(toggled(bool)),
            d->objectCycleCB, SLOT(setEnabled(bool)));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            d->objectTypeCB, SLOT(setEnabled(bool)));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            d->objectTypeDescEdit, SLOT(setEnabled(bool)));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            d->originalTransEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->dateReleasedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateExpiredCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->timeReleasedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->timeExpiredCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->languageCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectCycleCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttribute, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->dateReleasedSel, SIGNAL(dateChanged(QDate)),
            this, SIGNAL(signalModified()));

    connect(d->dateExpiredSel, SIGNAL(dateChanged(QDate)),
            this, SIGNAL(signalModified()));

    connect(d->timeReleasedSel, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalModified()));

    connect(d->timeExpiredSel, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalModified()));

    connect(d->zoneReleasedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->zoneExpiredSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->setTodayReleasedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayReleased()));

    connect(d->setTodayExpiredBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayExpired()));

    // --------------------------------------------------------

    connect(d->languageBtn, SIGNAL(activated(QString)),
            this, SIGNAL(signalModified()));

    connect(d->priorityCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectCycleCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeDescEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->originalTransEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

IPTCProperties::~IPTCProperties()
{
    delete d;
}

void IPTCProperties::slotSetTodayReleased()
{
    d->dateReleasedSel->setDate(QDate::currentDate());
    d->timeReleasedSel->setTime(QTime::currentTime());
    d->zoneReleasedSel->setToUTC();
}

void IPTCProperties::slotSetTodayExpired()
{
    d->dateExpiredSel->setDate(QDate::currentDate());
    d->timeExpiredSel->setTime(QTime::currentTime());
    d->zoneExpiredSel->setToUTC();
}

void IPTCProperties::readMetadata(QByteArray& iptcData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setIptc(iptcData);

    QString     data;
    QStringList list;
    QDate       date;
    QTime       time;
    QString     dateStr, timeStr;

    dateStr = meta.getIptcTagString("Iptc.Application2.ReleaseDate", false);
    timeStr = meta.getIptcTagString("Iptc.Application2.ReleaseTime", false);

    d->dateReleasedSel->setDate(QDate::currentDate());
    d->dateReleasedCheck->setChecked(false);

    if (!dateStr.isEmpty())
    {
        date = QDate::fromString(dateStr, Qt::ISODate);

        if (date.isValid())
        {
            d->dateReleasedSel->setDate(date);
            d->dateReleasedCheck->setChecked(true);
        }
    }

    d->dateReleasedSel->setEnabled(d->dateReleasedCheck->isChecked());

    d->timeReleasedSel->setTime(QTime::currentTime());
    d->timeReleasedCheck->setChecked(false);
    d->zoneReleasedSel->setToUTC();

    if (!timeStr.isEmpty())
    {
        time = QTime::fromString(timeStr, Qt::ISODate);

        if (time.isValid())
        {
            d->timeReleasedSel->setTime(time);
            d->timeReleasedCheck->setChecked(true);
            d->zoneReleasedSel->setTimeZone(timeStr);
        }
    }

    d->timeReleasedSel->setEnabled(d->timeReleasedCheck->isChecked());
    d->zoneReleasedSel->setEnabled(d->timeReleasedCheck->isChecked());

    dateStr = meta.getIptcTagString("Iptc.Application2.ExpirationDate", false);
    timeStr = meta.getIptcTagString("Iptc.Application2.ExpirationTime", false);

    d->dateExpiredSel->setDate(QDate::currentDate());
    d->dateExpiredCheck->setChecked(false);

    if (!dateStr.isEmpty())
    {
        date = QDate::fromString(dateStr, Qt::ISODate);

        if (date.isValid())
        {
            d->dateExpiredSel->setDate(date);
            d->dateExpiredCheck->setChecked(true);
        }
    }

    d->dateExpiredSel->setEnabled(d->dateExpiredCheck->isChecked());

    d->timeExpiredSel->setTime(QTime::currentTime());
    d->timeExpiredCheck->setChecked(false);
    d->zoneExpiredSel->setToUTC();

    if (!timeStr.isEmpty())
    {
        time = QTime::fromString(timeStr, Qt::ISODate);

        if (time.isValid())
        {
            d->timeExpiredSel->setTime(time);
            d->timeExpiredCheck->setChecked(true);
            d->zoneExpiredSel->setTimeZone(timeStr);
        }
    }

    d->timeExpiredSel->setEnabled(d->timeExpiredCheck->isChecked());
    d->zoneExpiredSel->setEnabled(d->timeExpiredCheck->isChecked());

    d->languageCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Language", false);

    if (!data.isNull())
    {
        int index = d->languageBtn->findData(data);
        if (index != -1)
        {
            d->languageBtn->setCurrentIndex(index);
            d->languageCheck->setChecked(true);
        }
        else
        {
            d->languageCheck->setValid(false);
        }
    }

    d->languageBtn->setEnabled(d->languageCheck->isChecked());

    d->priorityCB->setCurrentIndex(0);
    d->priorityCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Urgency", false);

    if (!data.isNull())
    {
        const int val = data.toInt();

        if (val >= 0 && val <= 9)
        {
            d->priorityCB->setCurrentIndex(val);
            d->priorityCheck->setChecked(true);
        }
        else
        {
            d->priorityCheck->setValid(false);
        }
    }

    d->priorityCB->setEnabled(d->priorityCheck->isChecked());

    d->objectCycleCB->setCurrentIndex(0);
    d->objectCycleCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.ObjectCycle", false);

    if (!data.isNull())
    {
        if (data == QLatin1String("a"))
        {
            d->objectCycleCB->setCurrentIndex(0);
            d->objectCycleCheck->setChecked(true);
        }
        else if (data == QLatin1String("b"))
        {
            d->objectCycleCB->setCurrentIndex(1);
            d->objectCycleCheck->setChecked(true);
        }
        else if (data == QLatin1String("c"))
        {
            d->objectCycleCB->setCurrentIndex(2);
            d->objectCycleCheck->setChecked(true);
        }
        else
            d->objectCycleCheck->setValid(false);
    }

    d->objectCycleCB->setEnabled(d->objectCycleCheck->isChecked());

    d->objectTypeCB->setCurrentIndex(0);
    d->objectTypeDescEdit->clear();
    d->objectTypeCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.ObjectType", false);

    if (!data.isNull())
    {
        QString typeSec = data.section(QLatin1Char(':'), 0, 0);

        if (!typeSec.isEmpty())
        {
            int type = typeSec.toInt()-1;

            if (type >= 0 && type < 3)
            {
                d->objectTypeCB->setCurrentIndex(type);
                d->objectTypeDescEdit->setText(data.section(QLatin1Char(':'), -1));
                d->objectTypeCheck->setChecked(true);
            }
            else
            {
                d->objectTypeCheck->setValid(false);
            }
        }
    }

    d->objectTypeCB->setEnabled(d->objectTypeCheck->isChecked());
    d->objectTypeDescEdit->setEnabled(d->objectTypeCheck->isChecked());

    list = meta.getIptcTagsStringList("Iptc.Application2.ObjectAttribute", false);
    d->objectAttribute->setValues(list);

    d->originalTransEdit->clear();
    d->originalTransCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.TransmissionReference", false);

    if (!data.isNull())
    {
        d->originalTransEdit->setText(data);
        d->originalTransCheck->setChecked(true);
    }

    d->originalTransEdit->setEnabled(d->originalTransCheck->isChecked());

    blockSignals(false);
}

void IPTCProperties::applyMetadata(QByteArray& iptcData)
{
    DMetadata meta;
    meta.setIptc(iptcData);

    if (d->dateReleasedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ReleaseDate",
                                    d->dateReleasedSel->date().toString(Qt::ISODate));
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ReleaseDate");
    }

    if (d->dateExpiredCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ExpirationDate",
                                    d->dateExpiredSel->date().toString(Qt::ISODate));
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ExpirationDate");
    }

    if (d->timeReleasedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ReleaseTime",
                                    d->timeReleasedSel->time().toString(Qt::ISODate) +
                                    d->zoneReleasedSel->getTimeZone());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ReleaseTime");
    }

    if (d->timeExpiredCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ExpirationTime",
                                    d->timeExpiredSel->time().toString(Qt::ISODate) +
                                    d->zoneExpiredSel->getTimeZone());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ExpirationTime");
    }

    if (d->languageCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Language", d->languageBtn->currentData().toString());
    }
    else if (d->languageCheck->isValid())
    {
        meta.removeIptcTag("Iptc.Application2.Language");
    }

    if (d->priorityCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Urgency", QString::number(d->priorityCB->currentIndex()));
    }
    else if (d->priorityCheck->isValid())
    {
        meta.removeIptcTag("Iptc.Application2.Urgency");
    }

    if (d->objectCycleCheck->isChecked())
    {
        switch (d->objectCycleCB->currentIndex())
        {
            case(0):
                meta.setIptcTagString("Iptc.Application2.ObjectCycle", QLatin1String("a"));
                break;

            case(1):
                meta.setIptcTagString("Iptc.Application2.ObjectCycle", QLatin1String("b"));
                break;

            case(2):
                meta.setIptcTagString("Iptc.Application2.ObjectCycle", QLatin1String("c"));
                break;
        }
    }
    else if (d->objectCycleCheck->isValid())
    {
        meta.removeIptcTag("Iptc.Application2.ObjectCycle");
    }

    if (d->objectTypeCheck->isChecked())
    {
        QString objectType;
        objectType.sprintf("%2d", d->objectTypeCB->currentIndex()+1);
        objectType.append(QString::fromUtf8(":%1").arg(d->objectTypeDescEdit->text()));
        meta.setIptcTagString("Iptc.Application2.ObjectType", objectType);
    }
    else if (d->objectTypeCheck->isValid())
    {
        meta.removeIptcTag("Iptc.Application2.ObjectType");
    }

    QStringList oldList, newList;

    if (d->objectAttribute->getValues(oldList, newList))
        meta.setIptcTagsStringList("Iptc.Application2.ObjectAttribute", 64, oldList, newList);
    else if (d->objectAttribute->isValid())
        meta.removeIptcTag("Iptc.Application2.ObjectAttribute");

    if (d->originalTransCheck->isChecked())
        meta.setIptcTagString("Iptc.Application2.TransmissionReference", d->originalTransEdit->text());
    else
        meta.removeIptcTag("Iptc.Application2.TransmissionReference");

    iptcData = meta.getIptc();
}

} // namespace GenericDigikamMetadataEditPlugin
