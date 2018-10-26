/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - video metadata helper.
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

void ItemScanner::fillVideoMetadataContainer(qlonglong imageid, VideoMetadataContainer* const container)
{
    // read from database
    QVariantList fields      = CoreDbAccess().db()->getVideoMetadata(imageid);
    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
    {
        return;
    }

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allVideoMetadataFields());
    // associate with hard-coded variables
    container->aspectRatio                  = strings.at(0);
    container->audioBitRate                 = strings.at(1);
    container->audioChannelType             = strings.at(2);
    container->audioCodec                   = strings.at(3);
    container->duration                     = strings.at(4);
    container->frameRate                    = strings.at(5);
    container->videoCodec                   = strings.at(6);
}

QString ItemScanner::detectVideoFormat() const
{
    QString suffix = d->fileInfo.suffix().toUpper();

    if (suffix == QLatin1String("MPEG") || suffix == QLatin1String("MPG") || suffix == QLatin1String("MPO") || suffix == QLatin1String("MPE"))
    {
        return QLatin1String("MPEG");
    }

    if (suffix == QLatin1String("ASF") || suffix == QLatin1String("WMV"))
    {
        return QLatin1String("WMV");
    }

    if (suffix == QLatin1String("AVI") || suffix == QLatin1String("DIVX") )
    {
        return QLatin1String("AVI");
    }

    if (suffix == QLatin1String("MKV") || suffix == QLatin1String("MKS"))
    {
        return QLatin1String("MKV");
    }

    if (suffix == QLatin1String("M4V") || suffix == QLatin1String("MOV") || suffix == QLatin1String("M2V") )
    {
        return QLatin1String("MOV");
    }

    if (suffix == QLatin1String("3GP") || suffix == QLatin1String("3G2") )
    {
        return QLatin1String("3GP");
    }

    return suffix;
}

QString ItemScanner::detectAudioFormat() const
{
    return d->fileInfo.suffix().toUpper();
}

MetadataFields ItemScanner::allVideoMetadataFields()
{
    // This list must reflect the order required by CoreDB::addVideoMetadata
    MetadataFields fields;
    fields << MetadataInfo::AspectRatio
           << MetadataInfo::AudioBitRate
           << MetadataInfo::AudioChannelType
           << MetadataInfo::AudioCodec
           << MetadataInfo::Duration
           << MetadataInfo::FrameRate
           << MetadataInfo::VideoCodec;

    return fields;
}

void ItemScanner::scanVideoInformation()
{
    d->commit.commitItemInformation = true;

    if (d->scanMode == NewScan || d->scanMode == Rescan)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate
               << MetadataInfo::Orientation;
        QVariantList metadataInfos = d->metadata.getMetadataFields(fields);

        d->commit.imageInformationFields = DatabaseFields::Rating           |
                                           DatabaseFields::CreationDate     |
                                           DatabaseFields::DigitizationDate |
                                           DatabaseFields::Orientation;

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            d->commit.imageInformationFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        d->commit.imageInformationInfos = metadataInfos;
    }

    d->commit.imageInformationInfos << d->metadata.getMetadataField(MetadataInfo::VideoWidth)
                                    << d->metadata.getMetadataField(MetadataInfo::VideoHeight);
    d->commit.imageInformationFields |= DatabaseFields::Width | DatabaseFields::Height;

    // TODO: Please check / improve / rewrite detectVideoFormat().
    // The format strings shall be uppercase, and a clearly defined set
    // (all format strings used in the database should be defined in advance)
    if (d->scanInfo.category == DatabaseItem::Video)
        d->commit.imageInformationInfos << detectVideoFormat();
    else
        d->commit.imageInformationInfos << detectAudioFormat();

    d->commit.imageInformationFields |= DatabaseFields::Format;

    d->commit.imageInformationInfos << d->metadata.getMetadataField(MetadataInfo::VideoBitDepth);
    d->commit.imageInformationFields |= DatabaseFields::ColorDepth;

    d->commit.imageInformationInfos << d->metadata.getMetadataField(MetadataInfo::VideoColorSpace);
    d->commit.imageInformationFields |= DatabaseFields::ColorModel;
}

// commitItemInformation method is reused

void ItemScanner::scanVideoMetadata()
{
    QVariantList metadataInfos = d->metadata.getMetadataFields(allVideoMetadataFields());

    if (hasValidField(metadataInfos))
    {
        d->commit.commitVideoMetadata = true;
        // reuse imageMetadataInfos field
        d->commit.imageMetadataInfos  = metadataInfos;
    }
}

void ItemScanner::commitVideoMetadata()
{
    CoreDbAccess().db()->addVideoMetadata(d->scanInfo.id, d->commit.imageMetadataInfos);
}

} // namespace Digikam
