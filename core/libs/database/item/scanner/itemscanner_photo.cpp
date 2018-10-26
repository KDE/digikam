/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - photo metadata helper.
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemscanner_p.h"

namespace Digikam
{

void ItemScanner::fillMetadataContainer(qlonglong imageid, ImageMetadataContainer* const container)
{
    // read from database
    QVariantList fields      = CoreDbAccess().db()->getImageMetadata(imageid);
    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
    {
        return;
    }

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allImageMetadataFields());

    // associate with hard-coded variables
    container->make                         = strings.at(0);
    container->model                        = strings.at(1);
    container->lens                         = strings.at(2);
    container->aperture                     = strings.at(3);
    container->focalLength                  = strings.at(4);
    container->focalLength35                = strings.at(5);
    container->exposureTime                 = strings.at(6);
    container->exposureProgram              = strings.at(7);
    container->exposureMode                 = strings.at(8);
    container->sensitivity                  = strings.at(9);
    container->flashMode                    = strings.at(10);
    container->whiteBalance                 = strings.at(11);
    container->whiteBalanceColorTemperature = strings.at(12);
    container->meteringMode                 = strings.at(13);
    container->subjectDistance              = strings.at(14);
    container->subjectDistanceCategory      = strings.at(15);
}

QString ItemScanner::iptcCorePropertyName(MetadataInfo::Field field)
{
    // These strings are specified in DBSCHEMA.ods
    switch (field)
    {
            // copyright table
        case MetadataInfo::IptcCoreCopyrightNotice:
            return QLatin1String("copyrightNotice");
        case MetadataInfo::IptcCoreCreator:
            return QLatin1String("creator");
        case MetadataInfo::IptcCoreProvider:
            return QLatin1String("provider");
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return QLatin1String("rightsUsageTerms");
        case MetadataInfo::IptcCoreSource:
            return QLatin1String("source");
        case MetadataInfo::IptcCoreCreatorJobTitle:
            return QLatin1String("creatorJobTitle");
        case MetadataInfo::IptcCoreInstructions:
            return QLatin1String("instructions");

            // ImageProperties table
        case MetadataInfo::IptcCoreCountryCode:
            return QLatin1String("countryCode");
        case MetadataInfo::IptcCoreCountry:
            return QLatin1String("country");
        case MetadataInfo::IptcCoreCity:
            return QLatin1String("city");
        case MetadataInfo::IptcCoreLocation:
            return QLatin1String("location");
        case MetadataInfo::IptcCoreProvinceState:
            return QLatin1String("provinceState");
        case MetadataInfo::IptcCoreIntellectualGenre:
            return QLatin1String("intellectualGenre");
        case MetadataInfo::IptcCoreJobID:
            return QLatin1String("jobId");
        case MetadataInfo::IptcCoreScene:
            return QLatin1String("scene");
        case MetadataInfo::IptcCoreSubjectCode:
            return QLatin1String("subjectCode");
        case MetadataInfo::IptcCoreContactInfoCity:
            return QLatin1String("creatorContactInfo.city");
        case MetadataInfo::IptcCoreContactInfoCountry:
            return QLatin1String("creatorContactInfo.country");
        case MetadataInfo::IptcCoreContactInfoAddress:
            return QLatin1String("creatorContactInfo.address");
        case MetadataInfo::IptcCoreContactInfoPostalCode:
            return QLatin1String("creatorContactInfo.postalCode");
        case MetadataInfo::IptcCoreContactInfoProvinceState:
            return QLatin1String("creatorContactInfo.provinceState");
        case MetadataInfo::IptcCoreContactInfoEmail:
            return QLatin1String("creatorContactInfo.email");
        case MetadataInfo::IptcCoreContactInfoPhone:
            return QLatin1String("creatorContactInfo.phone");
        case MetadataInfo::IptcCoreContactInfoWebUrl:
            return QLatin1String("creatorContactInfo.webUrl");
        default:
            return QString();
    }
}

QString ItemScanner::detectImageFormat() const
{
    DImg::FORMAT dimgFormat = d->img.detectedFormat();

    switch (dimgFormat)
    {
        case DImg::JPEG:
            return QLatin1String("JPG");
        case DImg::PNG:
            return QLatin1String("PNG");
        case DImg::TIFF:
            return QLatin1String("TIFF");
        case DImg::PPM:
            return QLatin1String("PPM");
        case DImg::JP2K:
            return QLatin1String("JP2");
        case DImg::PGF:
            return QLatin1String("PGF");
        case DImg::RAW:
        {
            QString format = QLatin1String("RAW-");
            format += d->fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            QByteArray format = QImageReader::imageFormat(d->fileInfo.filePath());

            if (!format.isEmpty())
            {
                return QString::fromUtf8(format).toUpper();
            }

            break;
        }
    }

    // See BUG #339341: Take file name suffix instead type mime analyze.
    return d->fileInfo.suffix().toUpper();
}

void ItemScanner::scanImageMetadata()
{
    QVariantList metadataInfos = d->metadata.getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
    {
        d->commit.commitImageMetadata = true;
        d->commit.imageMetadataInfos  = metadataInfos;
    }
}

void ItemScanner::commitImageMetadata()
{
    CoreDbAccess().db()->addImageMetadata(d->scanInfo.id, d->commit.imageMetadataInfos);
}

void ItemScanner::scanImagePosition()
{
    // This list must reflect the order required by CoreDB::addImagePosition
    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionAccuracy
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
    {
        d->commit.commitImagePosition = true;
        d->commit.imagePositionInfos  = metadataInfos;
    }
}

void ItemScanner::commitImagePosition()
{
    CoreDbAccess().db()->addImagePosition(d->scanInfo.id, d->commit.imagePositionInfos);
}

void ItemScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Headline
           << MetadataInfo::Title;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    // handles all possible fields, multi-language, author, date
    CaptionsMap captions = d->metadata.getImageComments();

    if (captions.isEmpty() && !hasValidField(metadataInfos))
    {
        return;
    }

    d->commit.commitImageComments  = true;
    d->commit.captions             = captions;

    // Headline
    if (!metadataInfos.at(0).isNull())
    {
        d->commit.headline = metadataInfos.at(0).toString();
    }

    // Title
    if (!metadataInfos.at(1).isNull())
    {
        d->commit.title = metadataInfos.at(1).toMap()[QLatin1String("x-default")].toString();
    }
}

void ItemScanner::commitImageComments()
{
    CoreDbAccess access;
    ImageComments comments(access, d->scanInfo.id);

    // Description
    if (!d->commit.captions.isEmpty())
    {
        comments.replaceComments(d->commit.captions);
    }

    // Headline
    if (!d->commit.headline.isNull())
    {
        comments.addHeadline(d->commit.headline);
    }

    // Title
    if (!d->commit.title.isNull())
    {
        comments.addTitle(d->commit.title);
    }
}

void ItemScanner::scanImageCopyright()
{
    Template t;

    if (!d->metadata.getCopyrightInformation(t))
    {
        return;
    }

    d->commit.commitImageCopyright = true;
    d->commit.copyrightTemplate    = t;
}

void ItemScanner::commitImageCopyright()
{
    ImageCopyright copyright(d->scanInfo.id);
    // It is not clear if removeAll() should be called if d->scanMode == Rescan
    copyright.removeAll();
    copyright.setFromTemplate(d->commit.copyrightTemplate);
}

void ItemScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreLocationInfo
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
    {
        return;
    }

    d->commit.commitIPTCCore        = true;
    d->commit.iptcCoreMetadataInfos = metadataInfos;
}

void ItemScanner::commitIPTCCore()
{
    ItemExtendedProperties props(d->scanInfo.id);

    if (!d->commit.iptcCoreMetadataInfos.at(0).isNull())
    {
        IptcCoreLocationInfo loc = d->commit.iptcCoreMetadataInfos.at(0).value<IptcCoreLocationInfo>();

        if (!loc.isNull())
        {
            props.setLocation(loc);
        }
    }

    if (!d->commit.iptcCoreMetadataInfos.at(1).isNull())
    {
        props.setIntellectualGenre(d->commit.iptcCoreMetadataInfos.at(1).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(2).isNull())
    {
        props.setJobId(d->commit.iptcCoreMetadataInfos.at(2).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(3).isNull())
    {
        props.setScene(d->commit.iptcCoreMetadataInfos.at(3).toStringList());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(4).isNull())
    {
        props.setSubjectCode(d->commit.iptcCoreMetadataInfos.at(4).toStringList());
    }
}

void ItemScanner::scanTags()
{
    // Check Keywords tag paths.

    QVariant var         = d->metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();
    QStringList filteredKeywords;

    // Extra empty tags check, empty tag = root tag which is not asignable
    for (int index = 0 ; index < keywords.size() ; index++)
    {
        QString keyword = keywords.at(index);

        if (!keyword.isEmpty())
        {

            // _Digikam_root_tag_ is present in some photos tagged with older
            // version of digiKam, must be removed
            if (keyword.contains(QRegExp(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)"))))
            {
                keyword = keyword.replace(QRegExp(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)")),
                                          QLatin1String(""));
            }

            filteredKeywords.append(keyword);
        }
    }

    if (!filteredKeywords.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(filteredKeywords);
        d->commit.tagIds += tagIds;
    }

    // Check Pick Label tag.

    int pickId = d->metadata.getImagePickLabel();

    if (pickId != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Pick Label found : " << pickId;

        int tagId = TagsCache::instance()->tagForPickLabel((PickLabel)pickId);

        if (tagId)
        {
            d->commit.tagIds << tagId;
            d->commit.hasPickTag = true;
            qCDebug(DIGIKAM_DATABASE_LOG) << "Assigned Pick Label Tag  : " << tagId;
        }
        else
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Cannot find Pick Label Tag for : " << pickId;
        }
    }

    // Check Color Label tag.

    int colorId = d->metadata.getImageColorLabel();

    if (colorId != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Color Label found : " << colorId;

        int tagId = TagsCache::instance()->tagForColorLabel((ColorLabel)colorId);

        if (tagId)
        {
            d->commit.tagIds << tagId;
            d->commit.hasColorTag = true;
            qCDebug(DIGIKAM_DATABASE_LOG) << "Assigned Color Label Tag  : " << tagId;
        }
        else
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Cannot find Color Label Tag for : " << colorId;
        }
    }
}

void ItemScanner::commitTags()
{
    QList<int> currentTags = CoreDbAccess().db()->getItemTagIDs(d->scanInfo.id);
    QVector<int> colorTags = TagsCache::instance()->colorLabelTags();
    QVector<int> pickTags  = TagsCache::instance()->pickLabelTags();
    QList<int> removeTags;

    foreach (int cTag, currentTags)
    {
        if ((d->commit.hasColorTag && colorTags.contains(cTag)) ||
            (d->commit.hasPickTag && pickTags.contains(cTag)))
        {
            removeTags << cTag;
        }
    }

    if (!removeTags.isEmpty())
    {
        CoreDbAccess().db()->removeTagsFromItems(QList<qlonglong>() << d->scanInfo.id, removeTags);
    }

    CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << d->scanInfo.id, d->commit.tagIds);
}

void ItemScanner::scanFaces()
{
    QSize size = d->img.size();

    if (!size.isValid())
    {
        return;
    }

    QMultiMap<QString,QVariant> metadataFacesMap;

    if (!d->metadata.getImageFacesMap(metadataFacesMap))
    {
        return;
    }

    d->commit.commitFaces = true;
    d->commit.metadataFacesMap = metadataFacesMap;
}

void ItemScanner::commitFaces()
{
    QSize size = d->img.size();
    QMap<QString, QVariant>::const_iterator it;

    for (it = d->commit.metadataFacesMap.constBegin() ; it != d->commit.metadataFacesMap.constEnd() ; ++it)
    {
        QString name = it.key();
        QRectF rect  = it.value().toRectF();

        if (name.isEmpty() || !rect.isValid())
        {
            continue;
        }

        int tagId = FaceTags::getOrCreateTagForPerson(name);

        if (!tagId)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Failed to create a person tag for name" << name;
        }

        TagRegion region(TagRegion::relativeToAbsolute(rect, size));

        FaceTagsEditor editor;

        editor.add(d->scanInfo.id, tagId, region, false);
    }
}

void ItemScanner::checkCreationDateFromMetadata(QVariant& dateFromMetadata) const
{
    // creation date: fall back to file system property
    if (dateFromMetadata.isNull() || !dateFromMetadata.toDateTime().isValid())
    {
        dateFromMetadata = creationDateFromFilesystem(d->fileInfo);
    }
}

bool ItemScanner::checkRatingFromMetadata(const QVariant& ratingFromMetadata) const
{
    // should only be overwritten if set in metadata
    if (d->scanMode == Rescan)
    {
        if (ratingFromMetadata.isNull() || ratingFromMetadata.toInt() == -1)
        {
            return false;
        }
    }

    return true;
}

MetadataFields ItemScanner::allImageMetadataFields()
{
    // This list must reflect the order required by CoreDB::addImageMetadata
    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
           << MetadataInfo::Lens
           << MetadataInfo::Aperture
           << MetadataInfo::FocalLength
           << MetadataInfo::FocalLengthIn35mm
           << MetadataInfo::ExposureTime
           << MetadataInfo::ExposureProgram
           << MetadataInfo::ExposureMode
           << MetadataInfo::Sensitivity
           << MetadataInfo::FlashMode
           << MetadataInfo::WhiteBalance
           << MetadataInfo::WhiteBalanceColorTemperature
           << MetadataInfo::MeteringMode
           << MetadataInfo::SubjectDistance
           << MetadataInfo::SubjectDistanceCategory;
    return fields;
}

} // namespace Digikam
