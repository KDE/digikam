/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface - template helpers.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "template.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::setMetadataTemplate(const Template& t) const
{
    if (t.isNull())
    {
        return false;
    }

    QStringList authors               = t.authors();
    QString authorsPosition           = t.authorsPosition();
    QString credit                    = t.credit();
    QString source                    = t.source();
    MetaEngine::AltLangMap copyright  = t.copyright();
    MetaEngine::AltLangMap rightUsage = t.rightUsageTerms();
    QString instructions              = t.instructions();

    //qCDebug(DIGIKAM_METAENGINE_LOG) << "Applying Metadata Template: " << t.templateTitle() << " :: " << authors;

    // Set XMP tags. XMP<->IPTC Schema from Photoshop 7.0

    if (supportXmp())
    {
        if (!setXmpTagStringSeq("Xmp.dc.creator", authors))
        {
            return false;
        }

        if (!setXmpTagStringSeq("Xmp.tiff.Artist", authors))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.AuthorsPosition", authorsPosition))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Credit", credit))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Source", source))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.dc.source", source))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.dc.rights", copyright))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.tiff.Copyright", copyright))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", rightUsage))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Instructions", instructions))
        {
            return false;
        }
    }

    // Set IPTC tags.

    if (!setIptcTagsStringList("Iptc.Application2.Byline", 32,
                               getIptcTagsStringList("Iptc.Application2.Byline"),
                               authors))
    {
        return false;
    }

    if (!setIptcTag(authorsPosition,        32,  "Authors Title", "Iptc.Application2.BylineTitle"))
    {
        return false;
    }

    if (!setIptcTag(credit,                 32,  "Credit",        "Iptc.Application2.Credit"))
    {
        return false;
    }

    if (!setIptcTag(source,                 32,  "Source",        "Iptc.Application2.Source"))
    {
        return false;
    }

    if (!setIptcTag(copyright[QLatin1String("x-default")], 128, "Copyright",     "Iptc.Application2.Copyright"))
    {
        return false;
    }

    if (!setIptcTag(instructions,           256, "Instructions",  "Iptc.Application2.SpecialInstructions"))
    {
        return false;
    }

    if (!setIptcCoreLocation(t.locationInfo()))
    {
        return false;
    }

    if (!setCreatorContactInfo(t.contactInfo()))
    {
        return false;
    }

    if (supportXmp())
    {
        if (!setXmpSubjects(t.IptcSubjects()))
        {
            return false;
        }
    }

    // Synchronize Iptc subjects tags with Xmp subjects tags.
    QStringList list = t.IptcSubjects();
    QStringList newList;

    foreach (QString str, list) // krazy:exclude=foreach
    {
        if (str.startsWith(QLatin1String("XMP")))
        {
            str.replace(0, 3, QLatin1String("IPTC"));
        }

        newList.append(str);
    }

    if (!setIptcSubjects(getIptcSubjects(), newList))
    {
        return false;
    }

    return true;
}

bool DMetadata::removeMetadataTemplate() const
{
    // Remove Rights info.

    removeXmpTag("Xmp.dc.creator");
    removeXmpTag("Xmp.tiff.Artist");
    removeXmpTag("Xmp.photoshop.AuthorsPosition");
    removeXmpTag("Xmp.photoshop.Credit");
    removeXmpTag("Xmp.photoshop.Source");
    removeXmpTag("Xmp.dc.source");
    removeXmpTag("Xmp.dc.rights");
    removeXmpTag("Xmp.tiff.Copyright");
    removeXmpTag("Xmp.xmpRights.UsageTerms");
    removeXmpTag("Xmp.photoshop.Instructions");

    removeIptcTag("Iptc.Application2.Byline");
    removeIptcTag("Iptc.Application2.BylineTitle");
    removeIptcTag("Iptc.Application2.Credit");
    removeIptcTag("Iptc.Application2.Source");
    removeIptcTag("Iptc.Application2.Copyright");
    removeIptcTag("Iptc.Application2.SpecialInstructions");

    // Remove Location info.

    removeXmpTag("Xmp.photoshop.Country");
    removeXmpTag("Xmp.iptc.CountryCode");
    removeXmpTag("Xmp.photoshop.City");
    removeXmpTag("Xmp.iptc.Location");
    removeXmpTag("Xmp.photoshop.State");

    removeIptcTag("Iptc.Application2.CountryName");
    removeIptcTag("Iptc.Application2.CountryCode");
    removeIptcTag("Iptc.Application2.City");
    removeIptcTag("Iptc.Application2.SubLocation");
    removeIptcTag("Iptc.Application2.ProvinceState");

    // Remove Contact info.

    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork");
    removeXmpTag("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork");

    // Remove IPTC Subjects.

    removeXmpTag("Xmp.iptc.SubjectCode");
    removeIptcTag("Iptc.Application2.Subject");

    return true;
}

Template DMetadata::getMetadataTemplate() const
{
    Template t;

    getCopyrightInformation(t);

    t.setLocationInfo(getIptcCoreLocation());
    t.setIptcSubjects(getIptcCoreSubjects()); // get from XMP or Iptc

    return t;
}

bool DMetadata::getCopyrightInformation(Template& t) const
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreCopyrightNotice
           << MetadataInfo::IptcCoreCreator
           << MetadataInfo::IptcCoreProvider
           << MetadataInfo::IptcCoreRightsUsageTerms
           << MetadataInfo::IptcCoreSource
           << MetadataInfo::IptcCoreCreatorJobTitle
           << MetadataInfo::IptcCoreInstructions;

    QVariantList metadataInfos = getMetadataFields(fields);
    IptcCoreContactInfo contactInfo = getCreatorContactInfo();

    if (!hasValidField(metadataInfos) && contactInfo.isNull())
    {
        return false;
    }

    t.setCopyright(toAltLangMap(metadataInfos.at(0)));
    t.setAuthors(metadataInfos.at(1).toStringList());
    t.setCredit(metadataInfos.at(2).toString());
    t.setRightUsageTerms(toAltLangMap(metadataInfos.at(3)));
    t.setSource(metadataInfos.at(4).toString());
    t.setAuthorsPosition(metadataInfos.at(5).toString());
    t.setInstructions(metadataInfos.at(6).toString());

    t.setContactInfo(contactInfo);

    return true;
}

} // namespace Digikam
