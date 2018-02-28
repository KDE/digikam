/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow status properties settings page.
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

#include "xmpproperties.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "squeezedcombobox.h"
#include "digikam_debug.h"
#include "altlangstringedit.h"
#include "metadatacheckbox.h"
#include "multivaluesedit.h"
#include "objectattributesedit.h"
#include "iptcproperties.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

namespace Digikam
{

class XMPProperties::Private
{
public:

    Private()
    {
        priorityCB           = 0;
        objectTypeCB         = 0;
        priorityCheck        = 0;
        objectAttributeCheck = 0;
        sceneEdit            = 0;
        objectTypeEdit       = 0;
        objectAttributeEdit  = 0;
        objectAttributeCB    = 0;
        languageEdit         = 0;
        originalTransEdit    = 0;
        originalTransCheck   = 0;

        sceneCodeMap.insert( QLatin1String("010100"), i18n("Headshot") );
        sceneCodeMap.insert( QLatin1String("010200"), i18n("Half-length") );
        sceneCodeMap.insert( QLatin1String("010300"), i18n("Full-length") );
        sceneCodeMap.insert( QLatin1String("010400"), i18nc("Side view of a person", "Profile") );
        sceneCodeMap.insert( QLatin1String("010500"), i18n("Rear view") );
        sceneCodeMap.insert( QLatin1String("010600"), i18n("Single") );
        sceneCodeMap.insert( QLatin1String("010700"), i18n("Couple") );
        sceneCodeMap.insert( QLatin1String("010800"), i18n("Two") );
        sceneCodeMap.insert( QLatin1String("010900"), i18nc("group of people", "Group") );
        sceneCodeMap.insert( QLatin1String("011000"), i18n("General view") );
        sceneCodeMap.insert( QLatin1String("011100"), i18n("Panoramic view") );
        sceneCodeMap.insert( QLatin1String("011200"), i18n("Aerial view") );
        sceneCodeMap.insert( QLatin1String("011300"), i18n("Under-water") );
        sceneCodeMap.insert( QLatin1String("011400"), i18n("Night scene") );
        sceneCodeMap.insert( QLatin1String("011500"), i18n("Satellite") );
        sceneCodeMap.insert( QLatin1String("011600"), i18n("Exterior view") );
        sceneCodeMap.insert( QLatin1String("011700"), i18n("Interior view") );
        sceneCodeMap.insert( QLatin1String("011800"), i18n("Close-up") );
        sceneCodeMap.insert( QLatin1String("011900"), i18n("Action") );
        sceneCodeMap.insert( QLatin1String("012000"), i18n("Performing") );
        sceneCodeMap.insert( QLatin1String("012100"), i18n("Posing") );
        sceneCodeMap.insert( QLatin1String("012200"), i18n("Symbolic") );
        sceneCodeMap.insert( QLatin1String("012300"), i18n("Off-beat") );
        sceneCodeMap.insert( QLatin1String("012400"), i18n("Movie scene") );

        typeCodeMap.insert( QLatin1String("Advisory"),           i18n("Advisory") );
        typeCodeMap.insert( QLatin1String("Alert"),              i18n("Alert") );
        typeCodeMap.insert( QLatin1String("Catalog"),            i18n("Catalog") );
        typeCodeMap.insert( QLatin1String("Data"),               i18n("Data") );
        typeCodeMap.insert( QLatin1String("Document"),           i18nc("type is a document", "Document") );
        typeCodeMap.insert( QLatin1String("DTD"),                i18n("DTD") );
        typeCodeMap.insert( QLatin1String("Maintenance"),        i18n("Maintenance") );
        typeCodeMap.insert( QLatin1String("News"),               i18n("News") );
        typeCodeMap.insert( QLatin1String("NewsManagementMode"), i18n("News Management Mode") );
        typeCodeMap.insert( QLatin1String("Package"),            i18n("Package") );
        typeCodeMap.insert( QLatin1String("Schema"),             i18n("Schema") );
        typeCodeMap.insert( QLatin1String("Topic"),              i18n("Topic") );
        typeCodeMap.insert( QLatin1String("TopicSet"),           i18n("Topic Set") );

        IPTCProperties::CountryCodeMap map = IPTCProperties::countryCodeMap();

        for ( IPTCProperties::CountryCodeMap::Iterator it = map.begin(); it != map.end(); ++it)
        {
            languageCodeMap.insert(it.key(), it.value());
        }
    }

    typedef QMap<QString, QString>  SceneCodeMap;
    typedef QMap<QString, QString>  TypeCodeMap;
    typedef QMap<QString, QString>  LanguageCodeMap;

    SceneCodeMap                    sceneCodeMap;
    TypeCodeMap                     typeCodeMap;
    LanguageCodeMap                 languageCodeMap;

    QCheckBox*                      originalTransCheck;

    QComboBox*                      priorityCB;
    QComboBox*                      objectTypeCB;

    QLineEdit*                      objectAttributeEdit;
    QLineEdit*                      originalTransEdit;

    MetadataCheckBox*               priorityCheck;
    MetadataCheckBox*               objectAttributeCheck;

    MultiValuesEdit*                sceneEdit;
    MultiValuesEdit*                objectTypeEdit;
    MultiValuesEdit*                languageEdit;

    SqueezedComboBox*               objectAttributeCB;
};

XMPProperties::XMPProperties(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->languageEdit = new MultiValuesEdit(this, i18n("Language:"),
                          i18n("Select here the language of content."));

    QStringList list;

    for (Private::LanguageCodeMap::Iterator it = d->languageCodeMap.begin();
         it != d->languageCodeMap.end(); ++it)
        list.append(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()));

    d->languageEdit->setData(list);

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

    d->sceneEdit = new MultiValuesEdit(this, i18n("Scene:"),
                       i18n("Select here the scene type of the content."));

    QStringList list2;

    for (Private::SceneCodeMap::Iterator it = d->sceneCodeMap.begin();
         it != d->sceneCodeMap.end(); ++it)
        list2.append(QString::fromUtf8("%1 - %2").arg(it.key()).arg(it.value()));

    d->sceneEdit->setData(list2);

    // --------------------------------------------------------

    d->objectTypeEdit = new MultiValuesEdit(this, i18n("Type:"),
                            i18n("Select here the editorial type of the content."));

    QStringList list3;

    for (Private::TypeCodeMap::Iterator it = d->typeCodeMap.begin();
         it != d->typeCodeMap.end(); ++it)
        list3.append(it.value());

    d->objectTypeEdit->setData(list3);

    // --------------------------------------------------------

    d->objectAttributeCheck = new MetadataCheckBox(i18n("Attribute:"), this);
    d->objectAttributeCB    = new SqueezedComboBox(this);
    d->objectAttributeEdit  = new QLineEdit(this);
    d->objectAttributeEdit->setClearButtonEnabled(true);
    d->objectAttributeEdit->setWhatsThis(i18n("Set here the editorial attribute description of the content."));

    d->objectAttributeCB->setWhatsThis(i18n("Select here the editorial attribute of the content."));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("001 - ") + i18nc("editorial content attribute", "Current"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("002 - ") + i18nc("editorial content attribute", "Analysis"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("003 - ") + i18nc("editorial content attribute", "Archive material"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("004 - ") + i18nc("editorial content attribute", "Background"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("005 - ") + i18nc("editorial content attribute", "Feature"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("006 - ") + i18nc("editorial content attribute", "Forecast"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("007 - ") + i18nc("editorial content attribute", "History"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("008 - ") + i18nc("editorial content attribute", "Obituary"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("009 - ") + i18nc("editorial content attribute", "Opinion"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("010 - ") + i18nc("editorial content attribute", "Polls & Surveys"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("011 - ") + i18nc("editorial content attribute", "Profile"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("012 - ") + i18nc("editorial content attribute", "Results Listings & Table"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("013 - ") + i18nc("editorial content attribute", "Side bar & Supporting information"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("014 - ") + i18nc("editorial content attribute", "Summary"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("015 - ") + i18nc("editorial content attribute", "Transcript & Verbatim"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("016 - ") + i18nc("editorial content attribute", "Interview"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("017 - ") + i18nc("editorial content attribute", "From the Scene"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("018 - ") + i18nc("editorial content attribute", "Retrospective"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("019 - ") + i18nc("editorial content attribute", "Statistics"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("020 - ") + i18nc("editorial content attribute", "Update"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("021 - ") + i18nc("editorial content attribute", "Wrap-up"));
    d->objectAttributeCB->addSqueezedItem(QLatin1String("022 - ") + i18nc("editorial content attribute", "Press Release"));

    // --------------------------------------------------------

    d->originalTransCheck = new QCheckBox(i18n("Reference:"), this);
    d->originalTransEdit  = new QLineEdit(this);
    d->originalTransEdit->setClearButtonEnabled(true);
    d->originalTransEdit->setWhatsThis(i18n("Set here the original content transmission reference."));

    // --------------------------------------------------------

    grid->addWidget(d->languageEdit,                        0, 0, 1, 5);
    grid->addWidget(d->priorityCheck,                       1, 0, 1, 1);
    grid->addWidget(d->priorityCB,                          1, 1, 1, 1);
    grid->addWidget(d->sceneEdit,                           2, 0, 1, 5);
    grid->addWidget(d->objectTypeEdit,                      3, 0, 1, 5);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  4, 0, 1, 5);
    grid->addWidget(d->objectAttributeCheck,                5, 0, 1, 1);
    grid->addWidget(d->objectAttributeCB,                   5, 1, 1, 2);
    grid->addWidget(d->objectAttributeEdit,                 5, 3, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  6, 0, 1, 5);
    grid->addWidget(d->originalTransCheck,                  7, 0, 1, 1);
    grid->addWidget(d->originalTransEdit,                   7, 1, 1, 4);
    grid->setRowStretch(8, 10);
    grid->setColumnStretch(4, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            d->priorityCB, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeCB, SLOT(setEnabled(bool)));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            d->objectAttributeEdit, SLOT(setEnabled(bool)));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            d->originalTransEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->languageEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->priorityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sceneEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->objectTypeEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->originalTransCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->priorityCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->objectAttributeEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->originalTransEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

XMPProperties::~XMPProperties()
{
    delete d;
}

void XMPProperties::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);

    int         val;
    QString     data;
    QStringList code, list, list2;
    QString     dateStr, timeStr;
    DMetadata::AltLangMap map;

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.dc.language", false);

    for (QStringList::Iterator it = code.begin(); it != code.end(); ++it)
    {
        QStringList data = d->languageEdit->getData();
        QStringList::Iterator it2;

        for (it2 = data.begin(); it2 != data.end(); ++it2)
        {
            if ((*it2).left(2) == (*it))
            {
                list.append(*it2);
                break;
            }
        }

        if (it2 == data.end())
            d->languageEdit->setValid(false);
    }

    d->languageEdit->setValues(list);

    // ---------------------------------------------------------------

    d->priorityCB->setCurrentIndex(0);
    d->priorityCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Urgency", false);

    if (!data.isNull())
    {
        val = data.toInt();

        if (val >= 0 && val <= 9)
        {
            d->priorityCB->setCurrentIndex(val);
            d->priorityCheck->setChecked(true);
        }
        else
            d->priorityCheck->setValid(false);
    }

    d->priorityCB->setEnabled(d->priorityCheck->isChecked());

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.iptc.Scene", false);

    for (QStringList::Iterator it = code.begin(); it != code.end(); ++it)
    {
        QStringList data = d->sceneEdit->getData();
        QStringList::Iterator it2;

        for (it2 = data.begin(); it2 != data.end(); ++it2)
        {
            if ((*it2).left(6) == (*it))
            {
                list.append(*it2);
                break;
            }
        }

        if (it2 == data.end())
            d->sceneEdit->setValid(false);
    }

    d->sceneEdit->setValues(list);

    // ---------------------------------------------------------------

    code = meta.getXmpTagStringBag("Xmp.dc.type", false);

    for (QStringList::Iterator it3 = code.begin(); it3 != code.end(); ++it3)
    {
        QStringList data = d->objectTypeEdit->getData();
        QStringList::Iterator it4;

        for (it4 = data.begin(); it4 != data.end(); ++it4)
        {
            if ((*it4) == (*it3))
            {
                list2.append(*it4);
                break;
            }
        }

        if (it4 == data.end())
            d->objectTypeEdit->setValid(false);
    }

    d->objectTypeEdit->setValues(list2);

    // ---------------------------------------------------------------

    d->objectAttributeCB->setCurrentIndex(0);
    d->objectAttributeEdit->clear();
    d->objectAttributeCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.IntellectualGenre", false);

    if (!data.isNull())
    {
        QString attrSec = data.section(QLatin1Char(':'), 0, 0);

        if (!attrSec.isEmpty())
        {
            int attr = attrSec.toInt()-1;

            if (attr >= 0 && attr < 23)
            {
                d->objectAttributeCB->setCurrentIndex(attr);
                d->objectAttributeEdit->setText(data.section(QLatin1Char(':'), -1));
                d->objectAttributeCheck->setChecked(true);
            }
            else
                d->objectAttributeCheck->setValid(false);
        }
    }

    d->objectAttributeCB->setEnabled(d->objectAttributeCheck->isChecked());
    d->objectAttributeEdit->setEnabled(d->objectAttributeCheck->isChecked());

    // ---------------------------------------------------------------

    d->originalTransEdit->clear();
    d->originalTransCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.TransmissionReference", false);

    if (!data.isNull())
    {
        d->originalTransEdit->setText(data);
        d->originalTransCheck->setChecked(true);
    }

    d->originalTransEdit->setEnabled(d->originalTransCheck->isChecked());

    // ---------------------------------------------------------------

    blockSignals(false);
}

void XMPProperties::applyMetadata(QByteArray& xmpData)
{
    QStringList oldList, newList;
    DMetadata meta;
    meta.setXmp(xmpData);

    // ---------------------------------------------------------------

    if (d->languageEdit->getValues(oldList, newList))
    {
        QStringList newCode;

        for (QStringList::Iterator it2 = newList.begin(); it2 != newList.end(); ++it2)
            newCode.append((*it2).left(2));

        meta.setXmpTagStringBag("Xmp.dc.language", newCode);
    }
    else
    {
        meta.removeXmpTag("Xmp.dc.language");
    }

    // ---------------------------------------------------------------

    if (d->priorityCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.Urgency", QString::number(d->priorityCB->currentIndex()));
    else if (d->priorityCheck->isValid())
        meta.removeXmpTag("Xmp.photoshop.Urgency");

    // ---------------------------------------------------------------

    if (d->sceneEdit->getValues(oldList, newList))
    {
        QStringList newCode;

        for (QStringList::Iterator it2 = newList.begin(); it2 != newList.end(); ++it2)
            newCode.append((*it2).left(6));

        meta.setXmpTagStringBag("Xmp.iptc.Scene", newCode);
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.Scene");
    }

    // ---------------------------------------------------------------

    if (d->objectTypeEdit->getValues(oldList, newList))
        meta.setXmpTagStringBag("Xmp.dc.type", newList);
    else
        meta.removeXmpTag("Xmp.dc.type");

    // ---------------------------------------------------------------

    if (d->objectAttributeCheck->isChecked())
    {
        QString objectAttribute;
        objectAttribute.sprintf("%3d", d->objectAttributeCB->currentIndex()+1);
        objectAttribute.append(QString::fromUtf8(":%1").arg(d->objectAttributeEdit->text()));
        meta.setXmpTagString("Xmp.iptc.IntellectualGenre", objectAttribute);
    }
    else if (d->objectAttributeCheck->isValid())
    {
        meta.removeXmpTag("Xmp.iptc.IntellectualGenre");
    }

    // ---------------------------------------------------------------

    if (d->originalTransCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.TransmissionReference", d->originalTransEdit->text());
    else
        meta.removeXmpTag("Xmp.photoshop.TransmissionReference");

    xmpData = meta.getXmp();
}

}  // namespace Digikam
