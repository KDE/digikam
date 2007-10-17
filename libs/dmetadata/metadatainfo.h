/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-12
 * Description : Schema update
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

#ifndef METADATAINFO_H
#define METADATAINFO_H

namespace Digikam
{

namespace MetadataInfo
{

enum Field
{
    Rating,                           /// Int
    CreationDate,                     /// DateTime
    DigitizationDate,                 /// DateTime
    Orientation,                      /// Int, enum from libkexiv2

    Make,                             /// String
    Model,                            /// String
    Aperture,                         /// Double, FNumber
    FocalLength,                      /// Double, mm
    FocalLengthIn35mm,                /// Double, mm
    ExposureTime,                     /// Double, s
    ExposureProgram,                  /// Int, enum from Exif
    ExposureMode,                     /// Int, enum from Exif
    Sensitivity,                      /// Int, ISO sensitivity
    FlashMode,                        /// Int, bit mask from Exif
    WhiteBalance,                     /// Int, enum from Exif
    MeteringMode,                     /// Int, enum from Exif
    SubjectDistance,                  /// double, m
    SubjectDistanceCategory,          /// int, enum from Exif
    WhiteBalanceColorTemperature,     /// double, color temperature in K

    Longitude,                        /// String (as XMP GPSCoordinate)
    LongitudeNumber,                  /// double, degrees
    Latitude,                         /// String (as XMP GPSCoordinate)
    LatitudeNumber,                   /// double, degrees
    Altitude,                         /// double, m
    GeographicOrientation,            /// ?
    CameraTilt,                       /// ?
    CameraRoll,                       /// ?
    PositionDescription,              /// String

    IPTCCoreCopyrightNotice,          /// Map language -> String
    IPTCCoreCreator,                  /// List of type String
    IPTCCoreProvider,                 /// String
    IPTCCoreRightUsageTerms,          /// Map language -> String
    IPTCCoreSource,                   /// String

    IPTCCoreCreatorJobTitle,          /// String
    IPTCCoreInstructions,             /// String

    IPTCCoreCountryCode,              /// String
    IPTCCoreCountry,                  /// String
    IPTCCoreCity,                     /// String
    IPTCCoreLocation,                 /// String
    IPTCCoreProvinceState,            /// String
    IPTCCoreIntellectualGenre,        /// String
    IPTCCoreJobID,                    /// String
    IPTCCoreScene,                    /// List of type String
    IPTCCoreSubjectCode,              /// List of type String

    IPTCCoreDescription,              /// Map language -> String
    IPTCCoreDescriptionWriter,        /// String
    IPTCCoreHeadline,                 /// String
    IPTCCoreTitle                     /// Map language -> String
    // not supported: CreatorContactInfo
    // not handled here: DateCreated, Keywords
            

    //Dublin Core??
};

}

typedef QList<MetadataInfo::Field> MetadataFields;


}

#endif

