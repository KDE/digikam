/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow status settings page.
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

#include "xmpstatus.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "altlangstringedit.h"
#include "multistringsedit.h"
#include "dmetadata.h"

namespace Digikam
{

class XMPStatus::Private
{
public:

    Private()
    {
        objectNameEdit          = 0;
        specialInstructionEdit  = 0;
        specialInstructionCheck = 0;
        nicknameEdit            = 0;
        nicknameCheck           = 0;
        identifiersEdit         = 0;
    }

    QCheckBox*          nicknameCheck;
    QCheckBox*          specialInstructionCheck;

    QLineEdit*          nicknameEdit;

    QTextEdit*          specialInstructionEdit;

    MultiStringsEdit*   identifiersEdit;

    AltLangStringsEdit* objectNameEdit;
};

XMPStatus::XMPStatus(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid  = new QGridLayout(this);

    // --------------------------------------------------------

    d->objectNameEdit  = new AltLangStringsEdit(this, i18nc("short title for the content", "Title:"),
                                                i18n("Set here a shorthand reference for the content."));

    // --------------------------------------------------------

    d->nicknameCheck = new QCheckBox(i18n("Nickname:"), this);
    d->nicknameEdit  = new QLineEdit(this);
    d->nicknameEdit->setClearButtonEnabled(true);
    d->nicknameEdit->setWhatsThis(i18n("A short informal name for the resource."));

    // --------------------------------------------------------

    d->identifiersEdit = new MultiStringsEdit(this, i18n("Identifiers:"),
                             i18n("Set here the strings that identify content that recurs."),
                             false);

    // --------------------------------------------------------

    d->specialInstructionCheck = new QCheckBox(i18n("Special Instructions:"), this);
    d->specialInstructionEdit  = new QTextEdit(this);
    d->specialInstructionEdit->setWhatsThis(i18n("Enter the editorial usage instructions."));

    // --------------------------------------------------------

    grid->addWidget(d->objectNameEdit,          0, 0, 1, 3);
    grid->addWidget(d->nicknameCheck,           1, 0, 1, 1);
    grid->addWidget(d->nicknameEdit,            1, 1, 1, 2);
    grid->addWidget(d->identifiersEdit,         2, 0, 1, 3);
    grid->addWidget(d->specialInstructionCheck, 3, 0, 1, 3);
    grid->addWidget(d->specialInstructionEdit,  4, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            d->specialInstructionEdit, SLOT(setEnabled(bool)));

    connect(d->nicknameCheck, SIGNAL(toggled(bool)),
            d->nicknameEdit, SLOT(setEnabled(bool)));

    connect(d->identifiersEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->objectNameEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->nicknameCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->objectNameEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->nicknameEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->specialInstructionEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));
}

XMPStatus::~XMPStatus()
{
    delete d;
}

void XMPStatus::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);

    QString            data;
    QStringList        list;
    DMetadata::AltLangMap map;

    d->objectNameEdit->setValues(map);
    d->objectNameEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.title", false);

    if (!map.isEmpty())
    {
        d->objectNameEdit->setValues(map);
        d->objectNameEdit->setValid(true);
    }

    d->nicknameEdit->clear();
    d->nicknameCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.xmp.Nickname", false);

    if (!data.isNull())
    {
        d->nicknameEdit->setText(data);
        d->nicknameCheck->setChecked(true);
    }

    d->nicknameEdit->setEnabled(d->nicknameCheck->isChecked());

    list = meta.getXmpTagStringSeq("Xmp.xmp.Identifier", false);
    d->identifiersEdit->setValues(list);

    d->specialInstructionEdit->clear();
    d->specialInstructionCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Instructions", false);

    if (!data.isNull())
    {
        d->specialInstructionEdit->setText(data);
        d->specialInstructionCheck->setChecked(true);
    }

    d->specialInstructionEdit->setEnabled(d->specialInstructionCheck->isChecked());

    blockSignals(false);
}

void XMPStatus::applyMetadata(QByteArray& xmpData)
{
    QStringList oldList, newList;
    DMetadata  meta;
    meta.setXmp(xmpData);

    DMetadata::AltLangMap oldAltLangMap, newAltLangMap;

    if (d->objectNameEdit->getValues(oldAltLangMap, newAltLangMap))
        meta.setXmpTagStringListLangAlt("Xmp.dc.title", newAltLangMap);
    else if (d->objectNameEdit->isValid())
        meta.removeXmpTag("Xmp.dc.title");

    if (d->nicknameCheck->isChecked())
        meta.setXmpTagString("Xmp.xmp.Nickname", d->nicknameEdit->text());
    else
        meta.removeXmpTag("Xmp.xmp.Nickname");

    if (d->identifiersEdit->getValues(oldList, newList))
        meta.setXmpTagStringSeq("Xmp.xmp.Identifier", newList);
    else
        meta.removeXmpTag("Xmp.xmp.Identifier");

    if (d->specialInstructionCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.Instructions", d->specialInstructionEdit->toPlainText());
    else
        meta.removeXmpTag("Xmp.photoshop.Instructions");

    xmpData = meta.getXmp();
}

}  // namespace Digikam
