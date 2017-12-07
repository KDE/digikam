/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP credits settings page.
 * Modified    : 2014-04-22 Alan Pater
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

#include "xmpcredits.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "multistringsedit.h"
#include "dmetadata.h"

namespace Digikam
{

class XMPCredits::Private
{
public:

    Private()
    {
        bylineEdit       = 0;
        bylineTitleEdit  = 0;
        creditEdit       = 0;
        sourceEdit       = 0;
        emailEdit        = 0;
        urlEdit          = 0;
        phoneEdit        = 0;
        addressEdit      = 0;
        postalCodeEdit   = 0;
        cityEdit         = 0;
        countryEdit      = 0;
        bylineTitleCheck = 0;
        creditCheck      = 0;
        sourceCheck      = 0;
        emailCheck       = 0;
        urlCheck         = 0;
        phoneCheck       = 0;
        addressCheck     = 0;
        postalCodeCheck  = 0;
        cityCheck        = 0;
        countryCheck     = 0;
        contactCheck     = 0;
    }

    QCheckBox*        bylineTitleCheck;
    QCheckBox*        creditCheck;
    QCheckBox*        sourceCheck;
    QCheckBox*        contactCheck;
    QCheckBox*        emailCheck;
    QCheckBox*        urlCheck;
    QCheckBox*        phoneCheck;
    QCheckBox*        addressCheck;
    QCheckBox*        postalCodeCheck;
    QCheckBox*        cityCheck;
    QCheckBox*        countryCheck;

    QLineEdit*        bylineTitleEdit;
    QLineEdit*        creditEdit;
    QLineEdit*        sourceEdit;
    QLineEdit*        emailEdit;
    QLineEdit*        urlEdit;
    QLineEdit*        phoneEdit;
    QLineEdit*        addressEdit;
    QLineEdit*        postalCodeEdit;
    QLineEdit*        cityEdit;
    QLineEdit*        countryEdit;

    MultiStringsEdit* bylineEdit;
};

XMPCredits::XMPCredits(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->bylineEdit = new MultiStringsEdit(this, i18n("Byline:"),
                                         i18n("Set here the name of content creator."),
                                         false);

    // --------------------------------------------------------

    d->bylineTitleCheck = new QCheckBox(i18n("Byline Title:"), this);
    d->bylineTitleEdit  = new QLineEdit(this);
    d->bylineTitleEdit->setClearButtonEnabled(true);
    d->bylineTitleEdit->setWhatsThis(i18n("Set here the title of content creator."));

    // --------------------------------------------------------

    QGroupBox* const contactBox = new QGroupBox(i18n("Contact"), this);
    QGridLayout* const grid2    = new QGridLayout(contactBox);

    d->emailCheck = new QCheckBox(i18nc("contact email address", "E-mail:"), contactBox);
    d->emailEdit  = new QLineEdit(contactBox);
    d->emailEdit->setClearButtonEnabled(true);
    d->emailEdit->setWhatsThis(i18n("Set here the contact e-mail."));

    d->urlCheck = new QCheckBox(i18n("URL:"), contactBox);
    d->urlEdit  = new QLineEdit(contactBox);
    d->urlEdit->setClearButtonEnabled(true);
    d->urlEdit->setWhatsThis(i18n("Set here the contact URL."));

    d->phoneCheck = new QCheckBox(i18n("Phone:"), contactBox);
    d->phoneEdit  = new QLineEdit(contactBox);
    d->phoneEdit->setClearButtonEnabled(true);
    d->phoneEdit->setWhatsThis(i18n("Set here the contact 'phone number."));

    d->addressCheck = new QCheckBox(i18nc("Street address", "Address:"), contactBox);
    d->addressEdit  = new QLineEdit(contactBox);
    d->addressEdit->setClearButtonEnabled(true);
    d->addressEdit->setWhatsThis(i18n("Set here the contact address."));

    d->postalCodeCheck = new QCheckBox(i18n("Postal code:"), contactBox);
    d->postalCodeEdit  = new QLineEdit(contactBox);
    d->postalCodeEdit->setClearButtonEnabled(true);
    d->postalCodeEdit->setWhatsThis(i18n("Set here the contact postal code."));

    d->cityCheck = new QCheckBox(i18n("City:"), contactBox);
    d->cityEdit  = new QLineEdit(contactBox);
    d->cityEdit->setClearButtonEnabled(true);
    d->cityEdit->setWhatsThis(i18n("Set here the contact city."));

    d->countryCheck = new QCheckBox(i18n("Country:"), contactBox);
    d->countryEdit  = new QLineEdit(contactBox);
    d->countryEdit->setClearButtonEnabled(true);
    d->countryEdit->setWhatsThis(i18n("Set here the contact country."));

    grid2->addWidget(d->emailCheck,         0, 0, 1, 1);
    grid2->addWidget(d->emailEdit,          0, 1, 1, 2);
    grid2->addWidget(d->urlCheck,           1, 0, 1, 1);
    grid2->addWidget(d->urlEdit,            1, 1, 1, 2);
    grid2->addWidget(d->phoneCheck,         2, 0, 1, 1);
    grid2->addWidget(d->phoneEdit,          2, 1, 1, 2);
    grid2->addWidget(d->addressCheck,       3, 0, 1, 1);
    grid2->addWidget(d->addressEdit,        3, 1, 1, 2);
    grid2->addWidget(d->postalCodeCheck,    4, 0, 1, 1);
    grid2->addWidget(d->postalCodeEdit,     4, 1, 1, 2);
    grid2->addWidget(d->cityCheck,          5, 0, 1, 1);
    grid2->addWidget(d->cityEdit,           5, 1, 1, 2);
    grid2->addWidget(d->countryCheck,       6, 0, 1, 1);
    grid2->addWidget(d->countryEdit,        6, 1, 1, 2);
    grid2->setColumnStretch(2, 10);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);

    // --------------------------------------------------------

    d->creditCheck = new QCheckBox(i18n("Credit:"), this);
    d->creditEdit  = new QLineEdit(this);
    d->creditEdit->setClearButtonEnabled(true);
    d->creditEdit->setWhatsThis(i18n("Set here the content provider."));

    // --------------------------------------------------------

    d->sourceCheck = new QCheckBox(i18nc("original owner of content", "Source:"), this);
    d->sourceEdit  = new QLineEdit(this);
    d->sourceEdit->setClearButtonEnabled(true);
    d->sourceEdit->setWhatsThis(i18n("Set here the original owner of content."));

    // --------------------------------------------------------

    grid->addWidget(d->bylineEdit,          0, 0, 1, 3);
    grid->addWidget(d->bylineTitleCheck,    1, 0, 1, 1);
    grid->addWidget(d->bylineTitleEdit,     1, 1, 1, 2);
    grid->addWidget(contactBox,             2, 0, 1, 3);
    grid->addWidget(d->creditCheck,         3, 0, 1, 1);
    grid->addWidget(d->creditEdit,          3, 1, 1, 2);
    grid->addWidget(d->sourceCheck,         4, 0, 1, 1);
    grid->addWidget(d->sourceEdit,          4, 1, 1, 2);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->bylineTitleCheck, SIGNAL(toggled(bool)),
            d->bylineTitleEdit, SLOT(setEnabled(bool)));

    connect(d->emailCheck, SIGNAL(toggled(bool)),
            d->emailEdit, SLOT(setEnabled(bool)));

    connect(d->urlCheck, SIGNAL(toggled(bool)),
            d->urlEdit, SLOT(setEnabled(bool)));

    connect(d->phoneCheck, SIGNAL(toggled(bool)),
            d->phoneEdit, SLOT(setEnabled(bool)));

    connect(d->addressCheck, SIGNAL(toggled(bool)),
            d->addressEdit, SLOT(setEnabled(bool)));

    connect(d->postalCodeCheck, SIGNAL(toggled(bool)),
            d->postalCodeEdit, SLOT(setEnabled(bool)));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            d->cityEdit, SLOT(setEnabled(bool)));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            d->countryEdit, SLOT(setEnabled(bool)));

    connect(d->creditCheck, SIGNAL(toggled(bool)),
            d->creditEdit, SLOT(setEnabled(bool)));

    connect(d->sourceCheck, SIGNAL(toggled(bool)),
            d->sourceEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->bylineEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->bylineTitleCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->emailCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->urlCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->phoneCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addressCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->postalCodeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->creditCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sourceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->bylineTitleEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->emailEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->urlEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->phoneEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->addressEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->postalCodeEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->cityEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->countryEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->creditEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->sourceEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

XMPCredits::~XMPCredits()
{
    delete d;
}

void XMPCredits::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);
    QString     data;
    QStringList list;

    list = meta.getXmpTagStringSeq("Xmp.dc.creator", false);
    d->bylineEdit->setValues(list);

    d->bylineTitleEdit->clear();
    d->bylineTitleCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.AuthorsPosition", false);

    if (!data.isNull())
    {
        d->bylineTitleEdit->setText(data);
        d->bylineTitleCheck->setChecked(true);
    }

    d->bylineTitleEdit->setEnabled(d->bylineTitleCheck->isChecked());

    // --------------------------------------------------------

    d->emailEdit->clear();
    d->emailCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiEmailWork", false);

    if (!data.isNull())
    {
        d->emailEdit->setText(data);
        d->emailCheck->setChecked(true);
    }

    d->emailEdit->setEnabled(d->emailCheck->isChecked());

    // --------------------------------------------------------

    d->urlEdit->clear();
    d->urlCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiUrlWork", false);

    if (!data.isNull())
    {
        d->urlEdit->setText(data);
        d->urlCheck->setChecked(true);
    }

    d->urlEdit->setEnabled(d->urlCheck->isChecked());

    // --------------------------------------------------------

    d->phoneEdit->clear();
    d->phoneCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiTelWork", false);

    if (!data.isNull())
    {
        d->phoneEdit->setText(data);
        d->phoneCheck->setChecked(true);
    }

    d->phoneEdit->setEnabled(d->phoneCheck->isChecked());

    // --------------------------------------------------------

    d->addressEdit->clear();
    d->addressCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiAdrExtadr", false);

    if (!data.isNull())
    {
        d->addressEdit->setText(data);
        d->addressCheck->setChecked(true);
    }

    d->addressEdit->setEnabled(d->addressCheck->isChecked());

    // --------------------------------------------------------

    d->postalCodeEdit->clear();
    d->postalCodeCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiAdrPcode", false);

    if (!data.isNull())
    {
        d->postalCodeEdit->setText(data);
        d->postalCodeCheck->setChecked(true);
    }

    d->postalCodeEdit->setEnabled(d->postalCodeCheck->isChecked());

    // --------------------------------------------------------

    d->cityEdit->clear();
    d->cityCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiAdrCity", false);

    if (!data.isNull())
    {
        d->cityEdit->setText(data);
        d->cityCheck->setChecked(true);
    }

    d->cityEdit->setEnabled(d->cityCheck->isChecked());

    // --------------------------------------------------------

    d->countryEdit->clear();
    d->countryCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.iptc.CiAdrCtry", false);

    if (!data.isNull())
    {
        d->countryEdit->setText(data);
        d->countryCheck->setChecked(true);
    }

    d->countryEdit->setEnabled(d->countryCheck->isChecked());

    // --------------------------------------------------------

    d->creditEdit->clear();
    d->creditCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Credit", false);

    if (!data.isNull())
    {
        d->creditEdit->setText(data);
        d->creditCheck->setChecked(true);
    }

    d->creditEdit->setEnabled(d->creditCheck->isChecked());

    // --------------------------------------------------------

    d->sourceEdit->clear();
    d->sourceCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Source", false);

    if (data.isNull())
        data = meta.getXmpTagString("Xmp.dc.source", false);

    if (!data.isNull())
    {
        d->sourceEdit->setText(data);
        d->sourceCheck->setChecked(true);
    }

    d->sourceEdit->setEnabled(d->sourceCheck->isChecked());

    blockSignals(false);
}

void XMPCredits::applyMetadata(QByteArray& xmpData)
{
    QStringList oldList, newList;
    DMetadata meta;
    meta.setXmp(xmpData);

    if (d->bylineEdit->getValues(oldList, newList))
        meta.setXmpTagStringSeq("Xmp.dc.creator", newList);
    else
        meta.removeXmpTag("Xmp.dc.creator");

    if (d->bylineTitleCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.AuthorsPosition", d->bylineTitleEdit->text());
    else
        meta.removeXmpTag("Xmp.photoshop.AuthorsPosition");

    // --------------------------------------------------------

    if (d->emailCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork", d->emailEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiEmailWork");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiEmailWork");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork");
    }

    if (d->urlCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork", d->urlEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiUrlWork");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiUrlWork");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork");
    }

    if (d->phoneCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork", d->phoneEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiTelWork");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiTelWork");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork");
    }

    if (d->addressCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr", d->addressEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiAdrExtadr");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiAdrExtadr");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr");
    }

    if (d->postalCodeCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode", d->postalCodeEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiAdrPcode");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiAdrPcode");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode");
    }

    if (d->cityCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", d->cityEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiAdrCity");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiAdrCity");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity");
    }

    if (d->countryCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", d->countryEdit->text());
        meta.removeXmpTag("Xmp.iptc.CiAdrCtry");
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.CiAdrCtry");
        meta.removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry");
    }

    // --------------------------------------------------------

    if (d->creditCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.Credit", d->creditEdit->text());
    else
        meta.removeXmpTag("Xmp.photoshop.Credit");

    if (d->sourceCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.Source", d->sourceEdit->text());
        meta.setXmpTagString("Xmp.dc.source", d->sourceEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.Source");
        meta.removeXmpTag("Xmp.dc.source");
    }

    xmpData = meta.getXmp();
}

}  // namespace Digikam
