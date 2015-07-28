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
    this->unifyReadWrite = true;

    defaultTagValues();
    defaultRatingValues();
    defaultCommentValues();

}

void DMetadataSettingsContainer::defaultTagValues()
{
    this->readTagNamespaces.clear();
    this->writeTagNamespaces.clear();

    // Default tag namespaces
    NamespaceEntry tagNs1 (QLatin1String("Xmp.digiKam.TagsList"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("/"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               0);
    tagNs1.specialOpts = NamespaceEntry::TAG_XMPSEQ;
    tagNs1.subspace    = NamespaceEntry::XMP;

    NamespaceEntry tagNs2 (QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("/"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               1);
    tagNs2.specialOpts = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace    = NamespaceEntry::XMP;

    NamespaceEntry tagNs3 (QLatin1String("Xmp.lr.hierarchicalSubject"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("|"),
                                               QString(),
                                                NamespaceEntry::TAGS,
                                                2);

    tagNs3.specialOpts = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace    = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts = NamespaceEntry::TAG_XMPSEQ;

    NamespaceEntry tagNs4 (QLatin1String("Xmp.mediapro.CatalogSets"),
                                               NamespaceEntry::TAGPATH,
                                               QLatin1String("|"),
                                               QString(),
                                               NamespaceEntry::TAGS,
                                               3);
    tagNs4.specialOpts = NamespaceEntry::TAG_XMPBAG;
    tagNs4.subspace    = NamespaceEntry::XMP;

    readTagNamespaces.append(tagNs1);
    readTagNamespaces.append(tagNs2);
    readTagNamespaces.append(tagNs3);
    readTagNamespaces.append(tagNs4);

    writeTagNamespaces = QList<NamespaceEntry>(readTagNamespaces);
}

void DMetadataSettingsContainer::defaultRatingValues()
{
    this->readRatingNamespaces.clear();
    this->writeRatingNamespaces.clear();

    QList<int> defaultVal, microsoftMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;

    NamespaceEntry ratingNs1 (QLatin1String("Xmp.xmp.Rating"),
                                                   defaultVal,
                                                   NamespaceEntry::RATING, 0);
    ratingNs1.subspace = NamespaceEntry::XMP;

    NamespaceEntry ratingNs2(QLatin1String("Xmp.acdsee.rating"),
                                                   defaultVal,
                                                   NamespaceEntry::RATING, 1);
    ratingNs2.subspace = NamespaceEntry::XMP;

    NamespaceEntry ratingNs3(QLatin1String("Xmp.MicrosoftPhoto.Rating"),
                                                   microsoftMappings,
                                                   NamespaceEntry::RATING, 2);
    ratingNs3.subspace = NamespaceEntry::XMP;

    readRatingNamespaces.append(ratingNs1);
    readRatingNamespaces.append(ratingNs2);
    readRatingNamespaces.append(ratingNs3);

    writeRatingNamespaces = QList<NamespaceEntry>(readRatingNamespaces);
}

void DMetadataSettingsContainer::defaultCommentValues()
{
    this->readCommentNamespaces.clear();
    this->writeCommentNamespaces.clear();

    NamespaceEntry commNs1 (QLatin1String("Xmp.dc.description"),
                                                    NamespaceEntry::COMMENT,
                                                    NamespaceEntry::COMMENT_ATLLANGLIST,
                                                    0);
    commNs1.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs2 (QLatin1String("Xmp.exif.UserComment"),
                                                    NamespaceEntry::COMMENT,
                                                    NamespaceEntry::COMMENT_ALTLANG,
                                                    1);
    commNs1.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs3 (QLatin1String("Xmp.tiff.ImageDescription"),
                                                    NamespaceEntry::COMMENT,
                                                    NamespaceEntry::COMMENT_ALTLANG,
                                                    2);
    commNs1.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs4 (QLatin1String("Xmp.acdsee.notes"),
                                                    NamespaceEntry::COMMENT,
                                                    NamespaceEntry::COMMENT_XMP,
                                                    3);
    commNs1.subspace = NamespaceEntry::XMP;

    readCommentNamespaces.append(commNs1);
    readCommentNamespaces.append(commNs2);

    readCommentNamespaces.append(commNs3);
    readCommentNamespaces.append(commNs4);

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
                    (NamespaceEntry::TagType)gr.readEntry("tagPaths").toInt(),
                    gr.readEntry("separator"),
                    gr.readEntry("extraXml"),
                    (NamespaceEntry::NamespaceType)gr.readEntry("nsType").toInt(),
                    gr.readEntry("index").toInt());

        ns.subspace = (NamespaceEntry::NsSubspace)gr.readEntry("subspace").toInt();
        ns.alternativeName = gr.readEntry("alternativeName");
        ns.specialOpts = (NamespaceEntry::SpecialOptions)gr.readEntry("specialOpts").toInt();
        ns.secondNameOpts = (NamespaceEntry::SpecialOptions)gr.readEntry("secondNameOpts").toInt();
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
        tmp.writeEntry("alternativeName", e.alternativeName);
        tmp.writeEntry("subspace",(int)e.subspace);
        tmp.writeEntry("tagPaths",(int)e.tagPaths);
        tmp.writeEntry("separator", e.separator);
        tmp.writeEntry("extraXml",e.extraXml);
        tmp.writeEntry("nsType",(int)e.nsType);
        tmp.writeEntry("convertRatio", e.convertRatio);
        tmp.writeEntry("specialOpts",(int)e.specialOpts);
        tmp.writeEntry("secondNameOpts",(int)e.secondNameOpts);
        tmp.writeEntry("index",e.index);
    }

}

}  // namespace Digikam
