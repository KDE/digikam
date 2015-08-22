/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : DMetadata Settings Container.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

// Qt includes

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
    if (group.hasGroup("readTagNamespaces")     &&
        group.hasGroup("readRatingNamespaces")  &&
        group.hasGroup("readCommentNamespaces") &&
        group.hasGroup("writeTagNamespaces")    &&
        group.hasGroup("writeRatingNamespaces") &&
        group.hasGroup("writeCommentNamespaces"))
    {
        readOneGroup(group, QLatin1String("readTagNamespaces"),      readTagNamespaces);
        readOneGroup(group, QLatin1String("readRatingNamespaces"),   readRatingNamespaces);
        readOneGroup(group, QLatin1String("readCommentNamespaces"),  readCommentNamespaces);
        readOneGroup(group, QLatin1String("writeTagNamespaces"),     writeTagNamespaces);
        readOneGroup(group, QLatin1String("writeRatingNamespaces"),  writeRatingNamespaces);
        readOneGroup(group, QLatin1String("writeCommentNamespaces"), writeCommentNamespaces);
    }
    else
    {
        defaultValues();
    }

}

void DMetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    writeOneGroup(group, QLatin1String("readTagNamespaces"),      readTagNamespaces);
    writeOneGroup(group, QLatin1String("readRatingNamespaces"),   readRatingNamespaces);
    writeOneGroup(group, QLatin1String("readCommentNamespaces"),  readCommentNamespaces);
    writeOneGroup(group, QLatin1String("writeTagNamespaces"),     writeTagNamespaces);
    writeOneGroup(group, QLatin1String("writeRatingNamespaces"),  writeRatingNamespaces);
    writeOneGroup(group, QLatin1String("writeCommentNamespaces"), writeCommentNamespaces);

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

    NamespaceEntry tagNs5 (QLatin1String("Xmp.acdsee.categories"),
                           NamespaceEntry::TAGPATH,
                           QLatin1String("/"),
                           QString(),
                           NamespaceEntry::TAGS,
                           4);
    tagNs5.specialOpts = NamespaceEntry::TAG_ACDSEE;
    tagNs5.subspace    = NamespaceEntry::XMP;

    NamespaceEntry tagNs6 (QLatin1String("Xmp.dc.subject"),
                           NamespaceEntry::TAG,
                           QLatin1String("/"),
                           QString(),
                           NamespaceEntry::TAGS,
                           5);
    tagNs6.specialOpts = NamespaceEntry::TAG_XMPBAG;
    tagNs6.subspace    = NamespaceEntry::XMP;

    NamespaceEntry tagNs7 (QLatin1String("Iptc.Application2.Keywords"),
                           NamespaceEntry::TAGPATH,
                           QLatin1String("."),
                           QString(),
                           NamespaceEntry::TAGS,
                           6);
    tagNs7.subspace    = NamespaceEntry::IPTC;

    NamespaceEntry tagNs8 (QLatin1String("Exif.Image.XPKeywords"),
                           NamespaceEntry::TAGPATH,
                           QLatin1String(";"),
                           QString(),
                           NamespaceEntry::TAGS,
                           7);
    tagNs8.subspace    = NamespaceEntry::EXIV;

    readTagNamespaces.append(tagNs1);
    readTagNamespaces.append(tagNs2);
    readTagNamespaces.append(tagNs3);
    readTagNamespaces.append(tagNs4);
    readTagNamespaces.append(tagNs5);
    readTagNamespaces.append(tagNs6);
    readTagNamespaces.append(tagNs7);
    readTagNamespaces.append(tagNs8);

    writeTagNamespaces = QList<NamespaceEntry>(readTagNamespaces);
}

void DMetadataSettingsContainer::defaultRatingValues()
{
    this->readRatingNamespaces.clear();
    this->writeRatingNamespaces.clear();

    QList<int> defaultVal, microsoftMappings, iptcMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings << 8 << 6 << 5 << 4 << 2 << 1;

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

    NamespaceEntry ratingNs4(QLatin1String("Exif.Image.0x4746"),
                             defaultVal,
                             NamespaceEntry::RATING, 3);
    ratingNs4.subspace = NamespaceEntry::EXIV;

    NamespaceEntry ratingNs5(QLatin1String("Exif.Image.0x4749"),
                             microsoftMappings,
                             NamespaceEntry::RATING, 4);
    ratingNs5.subspace = NamespaceEntry::EXIV;

    NamespaceEntry ratingNs6(QLatin1String("Iptc.Application2.Urgency"),
                             iptcMappings,
                             NamespaceEntry::RATING, 5);
    ratingNs6.subspace = NamespaceEntry::IPTC;

    readRatingNamespaces.append(ratingNs1);
    readRatingNamespaces.append(ratingNs2);
    readRatingNamespaces.append(ratingNs3);
    readRatingNamespaces.append(ratingNs4);
    readRatingNamespaces.append(ratingNs5);
    readRatingNamespaces.append(ratingNs6);

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
    commNs2.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs3 (QLatin1String("Xmp.tiff.ImageDescription"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_ALTLANG,
                            2);
    commNs3.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs4 (QLatin1String("Xmp.acdsee.notes"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_XMP,
                            3);
    commNs4.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs5 (QLatin1String("JPEG/TIFF Comments"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_JPEG,
                            5);
    commNs5.subspace = NamespaceEntry::XMP;

    NamespaceEntry commNs6 (QLatin1String("Exif.Image.ImageDescription"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::NO_OPTS,
                            4);
    commNs6.alternativeName = QLatin1String("Exif.Photo.UserComment");
    commNs6.subspace = NamespaceEntry::EXIV;

    NamespaceEntry commNs7 (QLatin1String("Iptc.Application2.Caption"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::NO_OPTS,
                            6);
    commNs7.subspace = NamespaceEntry::IPTC;

    readCommentNamespaces.append(commNs1);
    readCommentNamespaces.append(commNs2);

    readCommentNamespaces.append(commNs3);
    readCommentNamespaces.append(commNs4);
    readCommentNamespaces.append(commNs5);
    readCommentNamespaces.append(commNs6);
    readCommentNamespaces.append(commNs7);

    writeCommentNamespaces = QList<NamespaceEntry>(readCommentNamespaces);
}

void DMetadataSettingsContainer::readOneGroup(KConfigGroup &group, QString name, QList<NamespaceEntry>& container)
{
    KConfigGroup myItems = group.group(name);

    for (QString element : myItems.groupList())
    {
        KConfigGroup gr = myItems.group(element);
        NamespaceEntry ns;

        ns.namespaceName    = element;
        ns.tagPaths         = (NamespaceEntry::TagType)gr.readEntry("tagPaths").toInt();
        ns.separator        = gr.readEntry("separator");
        ns.extraXml         = gr.readEntry("extraXml");
        ns.nsType           = (NamespaceEntry::NamespaceType)gr.readEntry("nsType").toInt();
        ns.index            = gr.readEntry("index").toInt();
        ns.subspace         = (NamespaceEntry::NsSubspace)gr.readEntry("subspace").toInt();
        ns.alternativeName  = gr.readEntry("alternativeName");
        ns.specialOpts      = (NamespaceEntry::SpecialOptions)gr.readEntry("specialOpts").toInt();
        ns.secondNameOpts   = (NamespaceEntry::SpecialOptions)gr.readEntry("secondNameOpts").toInt();
        ns.isDefault        = gr.readEntry(QLatin1String("isDefault"), QVariant(true)).toBool();
        ns.isDisabled       = gr.readEntry(QLatin1String("isDisabled"), QVariant(false)).toBool();
        QString conversion  = gr.readEntry("convertRatio");

        for (QString str : conversion.split(QLatin1String(",")))
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

    for (NamespaceEntry e : container)
    {
        KConfigGroup tmp = namespacesGroup.group(e.namespaceName);
        tmp.writeEntry("alternativeName",   e.alternativeName);
        tmp.writeEntry("subspace",          (int)e.subspace);
        tmp.writeEntry("tagPaths",          (int)e.tagPaths);
        tmp.writeEntry("separator",         e.separator);
        tmp.writeEntry("extraXml",          e.extraXml);
        tmp.writeEntry("nsType",            (int)e.nsType);
        tmp.writeEntry("convertRatio",      e.convertRatio);
        tmp.writeEntry("specialOpts",       (int)e.specialOpts);
        tmp.writeEntry("secondNameOpts",    (int)e.secondNameOpts);
        tmp.writeEntry("index",             e.index);
        tmp.writeEntry("isDisabled",        e.isDisabled);
        tmp.writeEntry("isDefault",         e.isDefault);
    }
}

} // namespace Digikam
