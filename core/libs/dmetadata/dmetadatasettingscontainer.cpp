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

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "dmetadatasettings.h"
#include "digikam_debug.h"

namespace Digikam
{

bool dmcompare(NamespaceEntry& e1, NamespaceEntry e2)
{
    return  e1.index < e2.index;
}

// -------------------------------------------------------------------------------------------------

class DMetadataSettingsContainer::Private
{
public:

    Private()
    {
        unifyReadWrite = false;
    }

public:

    QMap<QLatin1String, QList<NamespaceEntry> > readMappings;
    QMap<QLatin1String, QList<NamespaceEntry> > writeMappings;
    bool                                        unifyReadWrite;
};

DMetadataSettingsContainer::DMetadataSettingsContainer()
    : d(new Private)
{
    addMapping(QLatin1String(DM_TAG_CONTAINER));
    addMapping(QLatin1String(DM_RATING_CONTAINER));
    addMapping(QLatin1String(DM_COMMENT_CONTAINER));
}

DMetadataSettingsContainer::DMetadataSettingsContainer(const DMetadataSettingsContainer& other)
    : d(new Private)
{
    *d = *other.d;
}

DMetadataSettingsContainer& DMetadataSettingsContainer::operator=(const DMetadataSettingsContainer& other)
{
    *d = *other.d;
    return *this;
}

DMetadataSettingsContainer::~DMetadataSettingsContainer()
{
    delete d;
}

bool DMetadataSettingsContainer::unifyReadWrite() const
{
    return d->unifyReadWrite;
}

void DMetadataSettingsContainer::setUnifyReadWrite(bool b)
{
    d->unifyReadWrite = b;
}

void DMetadataSettingsContainer::readFromConfig(KConfigGroup& group)
{
    bool valid = true;

    foreach(const QLatin1String& str, mappingKeys())
    {
        if (!group.hasGroup(QLatin1String("read") + str + QLatin1String("Namespaces")))
        {
            valid = false;
            qCDebug(DIGIKAM_GENERAL_LOG) << "Does not contain " << str << " Namespace";
            break;
        }

        if (!group.hasGroup(QLatin1String("write") + str + QLatin1String("Namespaces")))
        {
            valid = false;
            qCDebug(DIGIKAM_GENERAL_LOG) << "Does not contain " << str << " Namespace";
            break;
        }
    }

    if (valid)
    {
        foreach(const QLatin1String& str, mappingKeys())
        {
            readOneGroup(group, QLatin1String("read")  + str + QLatin1String("Namespaces"), getReadMapping(str));
            readOneGroup(group, QLatin1String("write") + str + QLatin1String("Namespaces"), getWriteMapping(str));
        }
    }
    else
    {
        defaultValues();
    }
//    if (group.hasGroup("readTagNamespaces")     &&
//        group.hasGroup("readRatingNamespaces")  &&
//        group.hasGroup("readCommentNamespaces") &&
//        group.hasGroup("writeTagNamespaces")    &&
//        group.hasGroup("writeRatingNamespaces") &&
//        group.hasGroup("writeCommentNamespaces"))
//    {
//        readOneGroup(group, QLatin1String("readTagNamespaces"),      readTagNamespaces);
//        readOneGroup(group, QLatin1String("readRatingNamespaces"),   readRatingNamespaces);
//        readOneGroup(group, QLatin1String("readCommentNamespaces"),  readCommentNamespaces);
//        readOneGroup(group, QLatin1String("writeTagNamespaces"),     writeTagNamespaces);
//        readOneGroup(group, QLatin1String("writeRatingNamespaces"),  writeRatingNamespaces);
//        readOneGroup(group, QLatin1String("writeCommentNamespaces"), writeCommentNamespaces);
//    }
//    else
//    {
//        defaultValues();
//    }

}

void DMetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    foreach(const QLatin1String& str, mappingKeys())
    {
        writeOneGroup(group, QLatin1String("read")  + str + QLatin1String("Namespaces"), getReadMapping(str));
        writeOneGroup(group, QLatin1String("write") + str + QLatin1String("Namespaces"), getWriteMapping(str));
    }
//    writeOneGroup(group, QLatin1String("readTagNamespaces"),      readTagNamespaces);
//    writeOneGroup(group, QLatin1String("readRatingNamespaces"),   readRatingNamespaces);
//    writeOneGroup(group, QLatin1String("readCommentNamespaces"),  readCommentNamespaces);
//    writeOneGroup(group, QLatin1String("writeTagNamespaces"),     writeTagNamespaces);
//    writeOneGroup(group, QLatin1String("writeRatingNamespaces"),  writeRatingNamespaces);
//    writeOneGroup(group, QLatin1String("writeCommentNamespaces"), writeCommentNamespaces);

    group.sync();
}

void DMetadataSettingsContainer::defaultValues()
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading default values ++++++++++++++++";
    d->unifyReadWrite = true;
    d->writeMappings.clear();
    d->readMappings.clear();

    defaultTagValues();
    defaultRatingValues();
    defaultCommentValues();
}

void DMetadataSettingsContainer::addMapping(const QLatin1String& key)
{
    d->readMappings[key]  = QList<NamespaceEntry>();
    d->writeMappings[key] = QList<NamespaceEntry>();
}

QList<NamespaceEntry> &DMetadataSettingsContainer::getReadMapping(const QLatin1String& key) const
{
    return d->readMappings[key];
}

QList<NamespaceEntry> &DMetadataSettingsContainer::getWriteMapping(const QLatin1String& key) const
{
    return d->writeMappings[key];
}

QList<QLatin1String> DMetadataSettingsContainer::mappingKeys() const
{
    return d->readMappings.keys();
}

void DMetadataSettingsContainer::defaultTagValues()
{
    // Default tag namespaces
    NamespaceEntry tagNs1;
    tagNs1.namespaceName    = QLatin1String("Xmp.digiKam.TagsList");
    tagNs1.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs1.separator        = QLatin1String("/");
    tagNs1.nsType           = NamespaceEntry::TAGS;
    tagNs1.index            = 0;
    tagNs1.specialOpts      = NamespaceEntry::TAG_XMPSEQ;
    tagNs1.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName    = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs2.separator        = QLatin1String("/");
    tagNs2.nsType           = NamespaceEntry::TAGS;
    tagNs2.index            = 1;
    tagNs2.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName    = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs3.separator        = QLatin1String("|");
    tagNs3.nsType           = NamespaceEntry::TAGS;
    tagNs3.index            = 2;
    tagNs3.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace         = NamespaceEntry::XMP;
    tagNs3.alternativeName  = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts   = NamespaceEntry::TAG_XMPSEQ;

    NamespaceEntry tagNs4;
    tagNs4.namespaceName    = QLatin1String("Xmp.mediapro.CatalogSets");
    tagNs4.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs4.separator        = QLatin1String("|");
    tagNs4.nsType           = NamespaceEntry::TAGS;
    tagNs4.index            = 3;
    tagNs4.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs4.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs5;
    tagNs5.namespaceName    = QLatin1String("Xmp.acdsee.categories");
    tagNs5.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs5.separator        = QLatin1String("/");
    tagNs5.nsType           = NamespaceEntry::TAGS;
    tagNs5.index            = 4;
    tagNs5.specialOpts      = NamespaceEntry::TAG_ACDSEE;
    tagNs5.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs6;
    tagNs6.namespaceName    = QLatin1String("Xmp.dc.subject");
    tagNs6.tagPaths         = NamespaceEntry::TAG;
    tagNs6.separator        = QLatin1String("/");
    tagNs6.nsType           = NamespaceEntry::TAGS;
    tagNs6.index            = 5;
    tagNs6.specialOpts      = NamespaceEntry::TAG_XMPBAG;
    tagNs6.subspace         = NamespaceEntry::XMP;

    NamespaceEntry tagNs7;
    tagNs7.namespaceName    = QLatin1String("Iptc.Application2.Keywords");
    tagNs7.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs7.separator        = QLatin1String(".");
    tagNs7.nsType           = NamespaceEntry::TAGS;
    tagNs7.index            = 6;
    tagNs7.subspace         = NamespaceEntry::IPTC;

    NamespaceEntry tagNs8;
    tagNs8.namespaceName    = QLatin1String("Exif.Image.XPKeywords");
    tagNs8.tagPaths         = NamespaceEntry::TAGPATH;
    tagNs8.separator        = QLatin1String(";");
    tagNs8.nsType           = NamespaceEntry::TAGS;
    tagNs8.index            = 7;
    tagNs8.subspace         = NamespaceEntry::EXIF;

    getReadMapping(QLatin1String(DM_TAG_CONTAINER)) << tagNs1
                                                    << tagNs2
                                                    << tagNs3
                                                    << tagNs4
                                                    << tagNs5
                                                    << tagNs6
                                                    << tagNs7
                                                    << tagNs8;

    d->writeMappings[QLatin1String(DM_TAG_CONTAINER)] = QList<NamespaceEntry>(getReadMapping(QLatin1String(DM_TAG_CONTAINER)));
}

void DMetadataSettingsContainer::defaultRatingValues()
{
    QList<int> defaultVal;
    QList<int> microsoftMappings;
    QList<int> iptcMappings;

    defaultVal        << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings      << 8 << 6 << 5 << 4 << 2 << 1;

    NamespaceEntry ratingNs1;
    ratingNs1.namespaceName = QLatin1String("Xmp.xmp.Rating");
    ratingNs1.convertRatio  = defaultVal;
    ratingNs1.nsType        = NamespaceEntry::RATING;
    ratingNs1.index         = 0;
    ratingNs1.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace      = NamespaceEntry::XMP;

    NamespaceEntry ratingNs4;
    ratingNs4.namespaceName = QLatin1String("Exif.Image.0x4746");
    ratingNs4.convertRatio  = defaultVal;
    ratingNs4.nsType        = NamespaceEntry::RATING;
    ratingNs4.index         = 3;
    ratingNs4.subspace      = NamespaceEntry::EXIF;

    NamespaceEntry ratingNs5;
    ratingNs5.namespaceName = QLatin1String("Exif.Image.0x4749");
    ratingNs5.convertRatio  = microsoftMappings;
    ratingNs5.nsType        = NamespaceEntry::RATING;
    ratingNs5.index         = 4;
    ratingNs5.subspace      = NamespaceEntry::EXIF;

    NamespaceEntry ratingNs6;
    ratingNs6.namespaceName = QLatin1String("Iptc.Application2.Urgency");
    ratingNs6.convertRatio  = iptcMappings;
    ratingNs6.nsType        = NamespaceEntry::RATING;
    ratingNs6.index         = 5;
    ratingNs6.subspace      = NamespaceEntry::IPTC;

    getReadMapping(QLatin1String(DM_RATING_CONTAINER))  << ratingNs1
                                                        << ratingNs2
                                                        << ratingNs3
                                                        << ratingNs4
                                                        << ratingNs5
                                                        << ratingNs6;

    d->writeMappings[QLatin1String(DM_RATING_CONTAINER)] = QList<NamespaceEntry>(getReadMapping(QLatin1String(DM_RATING_CONTAINER)));
}

void DMetadataSettingsContainer::defaultCommentValues()
{
    NamespaceEntry commNs1;
    commNs1.namespaceName   = QLatin1String("Xmp.dc.description");
    commNs1.nsType          = NamespaceEntry::COMMENT;
    commNs1.specialOpts     = NamespaceEntry::COMMENT_ATLLANGLIST;
    commNs1.index           = 0;
    commNs1.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs2;
    commNs2.namespaceName   = QLatin1String("Xmp.exif.UserComment");
    commNs2.nsType          = NamespaceEntry::COMMENT;
    commNs2.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs2.index           = 1;
    commNs2.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs3;
    commNs3.namespaceName   = QLatin1String("Xmp.tiff.ImageDescription");
    commNs3.nsType          = NamespaceEntry::COMMENT;
    commNs3.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs3.index           = 2;
    commNs3.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs4;
    commNs4.namespaceName   = QLatin1String("Xmp.acdsee.notes");
    commNs4.nsType          = NamespaceEntry::COMMENT;
    commNs4.specialOpts     = NamespaceEntry::COMMENT_XMP;
    commNs4.index           = 3;
    commNs4.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs5;
    commNs5.namespaceName   = QLatin1String("JPEG/TIFF Comments");
    commNs5.nsType          = NamespaceEntry::COMMENT;
    commNs5.specialOpts     = NamespaceEntry::COMMENT_JPEG;
    commNs5.index           = 4;
    commNs5.subspace        = NamespaceEntry::XMP;

    NamespaceEntry commNs6;
    commNs6.namespaceName   = QLatin1String("Exif.Image.ImageDescription");
    commNs6.nsType          = NamespaceEntry::COMMENT;
    commNs6.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs6.index           = 5;
    commNs6.alternativeName = QLatin1String("Exif.Photo.UserComment");
    commNs6.subspace        = NamespaceEntry::EXIF;

    NamespaceEntry commNs7;
    commNs7.namespaceName   = QLatin1String("Iptc.Application2.Caption");
    commNs7.nsType          = NamespaceEntry::COMMENT;
    commNs7.specialOpts     = NamespaceEntry::NO_OPTS;
    commNs7.index           = 6;
    commNs7.subspace        = NamespaceEntry::IPTC;


     getReadMapping(QLatin1String(DM_COMMENT_CONTAINER)) << commNs1
                                                         << commNs2
                                                         << commNs3
                                                         << commNs4
                                                         << commNs5
                                                         << commNs6
                                                         << commNs7;

    d->writeMappings[QLatin1String(DM_COMMENT_CONTAINER)] = QList<NamespaceEntry>(getReadMapping(QLatin1String(DM_COMMENT_CONTAINER)));
}

void DMetadataSettingsContainer::readOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container)
{
    KConfigGroup myItems = group.group(name);

    for (QString element : myItems.groupList())
    {
        KConfigGroup gr     = myItems.group(element);
        NamespaceEntry ns;

        ns.namespaceName    = element;
        ns.tagPaths         = (NamespaceEntry::TagType)gr.readEntry("tagPaths").toInt();
        ns.separator        = gr.readEntry("separator");
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

        container.append(ns);
    }

    std::sort(container.begin(), container.end(), Digikam::dmcompare);
}

void DMetadataSettingsContainer::writeOneGroup(KConfigGroup& group, const QString& name, QList<NamespaceEntry>& container) const
{
    KConfigGroup namespacesGroup = group.group(name);

    for (NamespaceEntry e : container)
    {
        KConfigGroup tmp = namespacesGroup.group(e.namespaceName);
        tmp.writeEntry("alternativeName", e.alternativeName);
        tmp.writeEntry("subspace",        (int)e.subspace);
        tmp.writeEntry("tagPaths",        (int)e.tagPaths);
        tmp.writeEntry("separator",       e.separator);
        tmp.writeEntry("nsType",          (int)e.nsType);
        tmp.writeEntry("convertRatio",    e.convertRatio);
        tmp.writeEntry("specialOpts",     (int)e.specialOpts);
        tmp.writeEntry("secondNameOpts",  (int)e.secondNameOpts);
        tmp.writeEntry("index",           e.index);
        tmp.writeEntry("isDisabled",      e.isDisabled);
        tmp.writeEntry("isDefault",       e.isDefault);
    }
}

} // namespace Digikam
