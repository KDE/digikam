/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-22
 * Description : Enums for database fields
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEFIELDS_H
#define DATABASEFIELDS_H

// Qt includes

#include <QFlags>


namespace Digikam
{

namespace DatabaseFields
{

enum ImageInformationField
{
    ImageInformationNone = 0,
    Rating               = 1 << 0,
    CreationDate         = 1 << 1,
    DigitizationDate     = 1 << 2,
    Orientation          = 1 << 3,
    SizeX                = 1 << 4,
    SizeY                = 1 << 5,
    ColorDepth           = 1 << 6,
    ColorModel           = 1 << 7,
    ImageInformationAll  =
            Rating | CreationDate | DigitizationDate | Orientation |
            SizeX | SizeY | ColorDepth | ColorModel
};

enum ImageMetadataField
{
    ImageMetadataNone            = 0,
    Make                         = 1 << 0,
    Model                        = 1 << 1,
    Aperture                     = 1 << 2,
    FocalLength                  = 1 << 3,
    FocalLength35                = 1 << 4,
    ExposureTime                 = 1 << 5,
    ExposureProgram              = 1 << 6,
    ExposureMode                 = 1 << 7,
    Sensitivity                  = 1 << 8,
    FlashMode                    = 1 << 9,
    WhiteBalance                 = 1 << 10,
    WhiteBalanceColorTemperature = 1 << 11,
    MeteringMode                 = 1 << 12,
    SubjectDistance              = 1 << 13,
    SubjectDistanceCategory      = 1 << 14,
    ImageMetadataAll             =
            Make | Model | Aperture | FocalLength | FocalLength35 |
            ExposureTime | ExposureProgram | ExposureMode | Sensitivity |
            FlashMode | WhiteBalance | WhiteBalanceColorTemperature |
            MeteringMode | SubjectDistance | SubjectDistanceCategory
};

enum ImagePositionsField
{
    ImagePositionsNone  = 0,
    Latitude            = 1 << 0,
    LatitudeNumber      = 1 << 1,
    Longitude           = 1 << 2,
    LongitudeNumber     = 1 << 3,
    Altitude            = 1 << 4,
    PositionOrientation = 1 << 5,
    PositionTilt        = 1 << 6,
    PositionRoll        = 1 << 7,
    PositionDescription = 1 << 8,
    ImagePositionsAll   =
            Latitude | LatitudeNumber | Longitude | LongitudeNumber | Altitude |
            PositionOrientation | PositionRoll | PositionTilt | PositionDescription
};

enum ImageCommentsField
{
    ImageCommentsNone = 0,
    CommentType       = 1 << 0,
    CommentLanguage   = 1 << 1,
    CommentAuthor     = 1 << 2,
    CommentDate       = 1 << 3,
    Comment           = 1 << 4,
    ImageCommentsAll  =
            CommentType | CommentAuthor | CommentLanguage | CommentDate | Comment
};

Q_DECLARE_FLAGS(ImageInformation, ImageInformationField);
Q_DECLARE_FLAGS(ImageMetadata, ImageMetadataField);
Q_DECLARE_FLAGS(ImageComments, ImageCommentsField);
Q_DECLARE_FLAGS(ImagePositions, ImagePositionsField);


} // end of namespace DatabaseFields

Q_DECLARE_OPERATORS_FOR_FLAGS(DatabaseFields::ImageInformation);
Q_DECLARE_OPERATORS_FOR_FLAGS(DatabaseFields::ImageMetadata);
Q_DECLARE_OPERATORS_FOR_FLAGS(DatabaseFields::ImageComments);
Q_DECLARE_OPERATORS_FOR_FLAGS(DatabaseFields::ImagePositions);


} // end of namespace Digikam


#endif

