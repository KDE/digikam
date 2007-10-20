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

enum ImagesField
{
    ImagesNone         = 0,
    Album              = 1 << 0,
    Name               = 1 << 1,
    Status             = 1 << 2,
    Category           = 1 << 3,
    ModificationDate   = 1 << 4,
    FileSize           = 1 << 5,
    UniqueHash         = 1 << 6,
    ImagesAll          =
            Album | Name | Status | Category |
            ModificationDate | FileSize | UniqueHash
};

enum ImageInformationField
{
    ImageInformationNone = 0,
    Rating               = 1 << 0,
    CreationDate         = 1 << 1,
    DigitizationDate     = 1 << 2,
    Orientation          = 1 << 3,
    Width                = 1 << 4,
    Height               = 1 << 5,
    Format               = 1 << 6,
    ColorDepth           = 1 << 7,
    ColorModel           = 1 << 8,
    ImageInformationAll  =
            Rating | CreationDate | DigitizationDate | Orientation |
            Width | Height | Format | ColorDepth | ColorModel
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

Q_DECLARE_FLAGS(Images, ImagesField);
Q_DECLARE_FLAGS(ImageInformation, ImageInformationField);
Q_DECLARE_FLAGS(ImageMetadata, ImageMetadataField);
Q_DECLARE_FLAGS(ImageComments, ImageCommentsField);
Q_DECLARE_FLAGS(ImagePositions, ImagePositionsField);



#define DATABASEFIELDS_SET_DECLARE_METHODS(Flag, variable) \
    inline Flag &operator=(const Flag &f) { return variable.operator=(f); } \
    inline Flag &operator|=(Flag f) { return variable.operator|=(f); } \
    inline Flag &operator|=(Flag::enum_type f) { return variable.operator|=(f); } \
    inline Flag &operator^=(Flag f) { return variable.operator^=(f); } \
    inline Flag &operator^=(Flag::enum_type f) { return variable.operator^=(f); } \
    inline Flag operator|(Flag f) const { return variable.operator|(f); } \
    inline Flag operator|(Flag::enum_type f) const { return variable.operator|(f); } \
    inline Flag operator^(Flag f) const { return variable.operator^(f); } \
    inline Flag operator^(Flag::enum_type f) const { return variable.operator^(f); } \
    inline Flag operator&(Flag::enum_type f) const { return variable.operator&(f); } \

/**
 * This class provides a set of all DatabasFields enums,
 * without resorting to a QSet.
*/
class Set
{
public:

    Set()
    {
        images = ImagesNone;
        imageInformation = ImageInformationNone;
        imageMetadata = ImageMetadataNone;
        imageComments = ImageCommentsNone;
        imagePositions = ImagePositionsNone;
    }


    DATABASEFIELDS_SET_DECLARE_METHODS(Images, images)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageInformation, imageInformation)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageMetadata, imageMetadata)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageComments, imageComments)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImagePositions, imagePositions)

private:
    Images images;
    ImageInformation imageInformation;
    ImageMetadata imageMetadata;
    ImageComments imageComments;
    ImagePositions imagePositions;
};



#define DATABASEFIELDS_HASH_DECLARE_METHODS(Key, method) \
    int remove(const Key &key) { return remove(method(key)); } \
    T take(const Key &key) { return take(method(key)); } \
    \
    bool contains(const Key &key) const { return contains(method(key)); } \
    const T value(const Key &key) const { return value(method(key)); } \
    const T value(const Key &key, const T &defaultValue) const { return value(method(key), defaultValue); } \
    T &operator[](const Key &key) { return operator[](method(key)); } \
    const T operator[](const Key &key) const { return operator[](method(key)); } \
    \
    QList<T> values(const Key &key) const { return value(method(key)); } \
    int count(const Key &key) const { return count(method(key)); }

/**
 * This class provides a hash on all DatabaseFields enums,
 * allowing to use the enum values as independent keys.
 * You can use the class like a normal QHash with the value type defined
 * by you, and as keys the members of the DatabaseFields enums.
 * You can only use single enum members as keys, not or'ed numbers.
 */

template <class T>
class Hash : public QHash<int, T>
{
public:

    static inline int uniqueKey(ImagesField f)            { return f + 0 * 256 + 0; }
    static inline int uniqueKey(ImageInformationField f) { return f + 1 * 256 + 1; }
    static inline int uniqueKey(ImageMetadataField f)    { return f + 2 * 256 + 2; }
    static inline int uniqueKey(ImageCommentsField f)    { return f + 3 * 256 + 3; }
    static inline int uniqueKey(ImagePositionsField f)    { return f + 4 * 256 + 4; }

    // override relevant methods from QHash
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImagesField, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageInformationField, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageMetadataField, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageCommentsField, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImagePositionsField, uniqueKey);

private:
    // make pure int methods private
    DATABASEFIELDS_HASH_DECLARE_METHODS(int, int);
};


} // end of namespace DatabaseFields

} // end of namespace Digikam

// must be outside the namespace!
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::Images);
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageInformation);
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageMetadata);
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageComments);
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImagePositions);


#endif

