/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-22
 * Description : Metadata Settings Container.
 *
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatasettingscontainer.h"

#include <QDebug>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "dmetadatasettings.h"

namespace Digikam
{

bool dmcompare(NamespaceEntry& e1, NamespaceEntry e2)
{
    return  e1.index < e2.index;
}

DMetadataSettingsContainer::DMetadataSettingsContainer()
{

}

void DMetadataSettingsContainer::readFromConfig(KConfigGroup& group)
{
    if(group.hasGroup("readTagNamespaces")
       && group.hasGroup("readRatingNamespaces")
       && group.hasGroup("readCommentNamespaces")
       && group.hasGroup("writeTagNamespaces")
       && group.hasGroup("writeRatingNamespaces")
       && group.hasGroup("writeCommentNamespaces"))
    {
        readOneGroup(group,QLatin1String("readTagNamespaces"),     readTagNamespaces);
        readOneGroup(group,QLatin1String("readRatingNamespaces"),  readRatingNamespaces);
        readOneGroup(group,QLatin1String("readCommentNamespaces"), readCommentNamespaces);
        readOneGroup(group,QLatin1String("writeTagNamespaces"),    writeTagNamespaces);
        readOneGroup(group,QLatin1String("writeRatingNamespaces"), writeRatingNamespaces);
        readOneGroup(group,QLatin1String("writeCommentNamespaces"),writeCommentNamespaces);
    }
    else
    {
        defaultValues();
    }

}

void DMetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    writeOneGroup(group,QLatin1String("readTagNamespaces"),     readTagNamespaces);
    writeOneGroup(group,QLatin1String("readRatingNamespaces"),  readRatingNamespaces);
    writeOneGroup(group,QLatin1String("readCommentNamespaces"), readCommentNamespaces);
    writeOneGroup(group,QLatin1String("writeTagNamespaces"),    writeTagNamespaces);
    writeOneGroup(group,QLatin1String("writeRatingNamespaces"), writeRatingNamespaces);
    writeOneGroup(group,QLatin1String("writeCommentNamespaces"),writeCommentNamespaces);

    group.sync();
}

void DMetadataSettingsContainer::defaultValues()
{
    qDebug() << "Loading default values ++++++++++++++++";
    this->readTagNamespaces.clear();
    this->readCommentNamespaces.clear();
    this->readRatingNamespaces.clear();
    this->writeTagNamespaces.clear();
    this->writeRatingNamespaces.clear();
    this->writeCommentNamespaces.clear();
    this->unifyReadWrite = true;

    // Default tag namespaces
    NamespaceEntry tagNs1 (QLatin1String("Xmp.digiKam.TagsList"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("/"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               0);
    tagNs1.specialOpts = NamespaceEntry::TAG_XMPSEQ;

    NamespaceEntry tagNs2 (QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("/"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               1);
    tagNs2.specialOpts = NamespaceEntry::TAG_XMPBAG;

    NamespaceEntry tagNs3 (QLatin1String("Xmp.lr.hierarchicalSubject"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("|"),
                                               QString(),
                                                NamespaceEntry::TAGS,
                                                2);

    tagNs3.specialOpts = NamespaceEntry::TAG_XMPBAG;
    NamespaceEntry tagNs4 (QLatin1String("Xmp.mediapro.CatalogSets"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("|"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               3);
    tagNs4.specialOpts = NamespaceEntry::TAG_XMPBAG;

    readTagNamespaces.append(tagNs1);
    readTagNamespaces.append(tagNs2);
    readTagNamespaces.append(tagNs3);
    readTagNamespaces.append(tagNs4);

    QList<int> defaultVal;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    readRatingNamespaces.append(NamespaceEntry(QLatin1String("Xmp.xmp.Rating"), defaultVal, NamespaceEntry::RATING, 0));
    readRatingNamespaces.append(NamespaceEntry(QLatin1String("Xmp.acdsee.rating"), defaultVal, NamespaceEntry::RATING, 1));

    QList<int> microsoftMappings;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    readRatingNamespaces.append(NamespaceEntry(QLatin1String("Xmp.MicrosoftPhoto.Rating"), microsoftMappings, NamespaceEntry::RATING, 2));

    readCommentNamespaces.append(NamespaceEntry(QLatin1String("Xmp.dc.description"),
                                                NamespaceEntry::COMMENT,
                                                NamespaceEntry::COMMENT_ATLLANGLIST,
                                                0));
    readCommentNamespaces.append(NamespaceEntry(QLatin1String("Xmp.exif.UserComment"),
                                                NamespaceEntry::COMMENT,
                                                NamespaceEntry::COMMENT_ALTLANG,
                                                1));

    readCommentNamespaces.append(NamespaceEntry(QLatin1String("Xmp.tiff.ImageDescription"),
                                                NamespaceEntry::COMMENT,
                                                NamespaceEntry::COMMENT_ALTLANG,
                                                2));
    readCommentNamespaces.append(NamespaceEntry(QLatin1String("Xmp.acdsee.notes"),
                                                NamespaceEntry::COMMENT,
                                                NamespaceEntry::COMMENT_XMP,
                                                3));

    writeTagNamespaces = QList<NamespaceEntry>(readTagNamespaces);
    writeRatingNamespaces = QList<NamespaceEntry>(readRatingNamespaces);
    writeCommentNamespaces = QList<NamespaceEntry>(readCommentNamespaces);
}

void DMetadataSettingsContainer::readOneGroup(KConfigGroup &group, QString name, QList<NamespaceEntry>& container)
{
    KConfigGroup myItems = group.group(name);

    for(QString element : myItems.groupList())
    {
        KConfigGroup gr = myItems.group(element);
        NamespaceEntry ns(
                    element,
                    (NamespaceEntry::TagType)gr.readEntry("Type").toInt(),
                    gr.readEntry("separator"),
                    gr.readEntry("extraXml"),
                    (NamespaceEntry::NamespaceType)gr.readEntry("NSType").toInt(),
                    gr.readEntry("index").toInt());

        ns.specialOpts = (NamespaceEntry::SpecialOptions)gr.readEntry("specialOpts").toInt();
        QString conversion = gr.readEntry("convertRatio");

        for(QString str : conversion.split(QLatin1String(",")))
        {
            ns.convertRatio.append(str.toInt());
        }

       qDebug() << "Reading element " << ns.namespaceName << " " << ns.index;
       container.append(ns);
    }
    qSort(container.begin(), container.end(),Digikam::dmcompare);
}

void DMetadataSettingsContainer::writeOneGroup(KConfigGroup &group, QString name, QList<NamespaceEntry> container) const
{
    KConfigGroup namespacesGroup = group.group(name);

    for(NamespaceEntry e : container)
    {
        KConfigGroup tmp = namespacesGroup.group(e.namespaceName);
        tmp.writeEntry("Type",(int)e.tagPaths);
        tmp.writeEntry("separator", e.separator);
        tmp.writeEntry("extraXml",e.extraXml);
        tmp.writeEntry("NSType",(int)e.nsType);
        tmp.writeEntry("convertRatio", e.convertRatio);
        tmp.writeEntry("specialOpts",(int)e.specialOpts);
        tmp.writeEntry("index",e.index);
    }

}

}  // namespace Digikam
