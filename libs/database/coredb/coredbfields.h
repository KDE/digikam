/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-22
 * Description : Core database field enumerations.
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef COREDATABASEFIELDS_H
#define COREDATABASEFIELDS_H

#include "digikam_config.h"

// C++ includes

#include <stdint.h>

// Qt includes

#include <QFlags>
#include <QHash>

// Local includes

#include "digikam_export.h"

#ifdef HAVE_DBUS
class QDBusArgument;
#endif

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
    ImagesAll          = Album            |
                         Name             |
                         Status           |
                         Category         |
                         ModificationDate |
                         FileSize         |
                         UniqueHash,
    ImagesFirst        = Album,
    ImagesLast         = UniqueHash
};
typedef uint8_t ImagesMinSizeType;

enum ImageInformationField
{
    ImageInformationNone  = 0,
    Rating                = 1 << 0,
    CreationDate          = 1 << 1,
    DigitizationDate      = 1 << 2,
    Orientation           = 1 << 3,
    Width                 = 1 << 4,
    Height                = 1 << 5,
    Format                = 1 << 6,
    ColorDepth            = 1 << 7,
    ColorModel            = 1 << 8,
    ColorLabel            = 1 << 9,
    PickLabel             = 1 << 10,
    ImageInformationAll   = Rating           |
                            CreationDate     |
                            DigitizationDate |
                            Orientation      |
                            Width            |
                            Height           |
                            Format           |
                            ColorDepth       |
                            ColorModel       |
                            ColorLabel       |
                            PickLabel,
    ImageInformationFirst = Rating,
    ImageInformationLast  = PickLabel
};
typedef uint16_t ImageInformationMinSizeType;

enum ImageMetadataField
{
    ImageMetadataNone            = 0,
    Make                         = 1 << 0,
    Model                        = 1 << 1,
    Lens                         = 1 << 2,
    Aperture                     = 1 << 3,
    FocalLength                  = 1 << 4,
    FocalLength35                = 1 << 5,
    ExposureTime                 = 1 << 6,
    ExposureProgram              = 1 << 7,
    ExposureMode                 = 1 << 8,
    Sensitivity                  = 1 << 9,
    FlashMode                    = 1 << 10,
    WhiteBalance                 = 1 << 11,
    WhiteBalanceColorTemperature = 1 << 12,
    MeteringMode                 = 1 << 13,
    SubjectDistance              = 1 << 14,
    SubjectDistanceCategory      = 1 << 15,
    ImageMetadataAll             = Make                         |
                                   Model                        |
                                   Lens                         |
                                   Aperture                     |
                                   FocalLength                  |
                                   FocalLength35                |
                                   ExposureTime                 |
                                   ExposureProgram              |
                                   ExposureMode                 |
                                   Sensitivity                  |
                                   FlashMode                    |
                                   WhiteBalance                 |
                                   WhiteBalanceColorTemperature |
                                   MeteringMode                 |
                                   SubjectDistance              |
                                   SubjectDistanceCategory,
    ImageMetadataFirst           = Make,
    ImageMetadataLast            = SubjectDistanceCategory
};
typedef uint16_t ImageMetadataMinSizeType;

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
    PositionAccuracy    = 1 << 8,
    PositionDescription = 1 << 9,
    ImagePositionsAll   = Latitude            |
                          LatitudeNumber      |
                          Longitude           |
                          LongitudeNumber     |
                          Altitude            |
                          PositionOrientation |
                          PositionRoll        |
                          PositionTilt        |
                          PositionAccuracy    |
                          PositionDescription,
    ImagePositionsFirst = Latitude,
    ImagePositionsLast  = PositionDescription
};
typedef uint16_t ImagePositionsMinSizeType;

enum ImageCommentsField
{
    ImageCommentsNone  = 0,
    CommentType        = 1 << 0,
    CommentLanguage    = 1 << 1,
    CommentAuthor      = 1 << 2,
    CommentDate        = 1 << 3,
    Comment            = 1 << 4,
    ImageCommentsAll   = CommentType     |
                         CommentAuthor   |
                         CommentLanguage |
                         CommentDate     |
                         Comment,
    ImageCommentsFirst = CommentType,
    ImageCommentsLast  = Comment
};
typedef uint8_t ImageCommentsMinSizeType;

enum ImageHistoryInfoField
{
    ImageHistoryInfoNone  = 0,
    ImageUUID             = 1 << 0,
    ImageHistory          = 1 << 1,
    ImageRelations        = 1 << 2,
    ImageHistoryInfoAll   = ImageUUID    |
                            ImageHistory |
                            ImageRelations,
    ImageHistoryInfoFirst = ImageUUID,
    ImageHistoryInfoLast  = ImageRelations
};
typedef uint8_t ImageHistoryInfoMinSizeType;

enum VideoMetadataField
{
    VideoMetadataNone            = 0,
    AspectRatio                  = 1 << 0,
    AudioBitRate                 = 1 << 1,
    AudioChannelType             = 1 << 2,
    AudioCodec                   = 1 << 3,
    Duration                     = 1 << 4,
    FrameRate                    = 1 << 5,
    VideoCodec                   = 1 << 6,
    VideoMetadataAll             = AspectRatio      |
                                   AudioBitRate     |
                                   AudioChannelType |
                                   AudioCodec  |
                                   Duration         |
                                   FrameRate        |
                                   VideoCodec,
    VideoMetadataFirst           = AspectRatio,
    VideoMetadataLast            = VideoCodec
};
typedef uint8_t VideoMetadataMinSizeType;

Q_DECLARE_FLAGS(Images,           ImagesField)
Q_DECLARE_FLAGS(ImageInformation, ImageInformationField)
Q_DECLARE_FLAGS(ImageMetadata,    ImageMetadataField)
Q_DECLARE_FLAGS(ImageComments,    ImageCommentsField)
Q_DECLARE_FLAGS(ImagePositions,   ImagePositionsField)
Q_DECLARE_FLAGS(ImageHistoryInfo, ImageHistoryInfoField)
Q_DECLARE_FLAGS(VideoMetadata,    VideoMetadataField)

template<typename FieldName> class FieldMetaInfo
{
};

#define DECLARE_FIELDMETAINFO(FieldName)                                                                    \
    template<> class DIGIKAM_DATABASE_EXPORT FieldMetaInfo <FieldName>                                      \
    {                                                                                                       \
        public:                                                                                             \
            static const FieldName##Field DIGIKAM_DATABASE_EXPORT First = FieldName##First;                 \
            static const FieldName##Field DIGIKAM_DATABASE_EXPORT Last  = FieldName##Last;                  \
            typedef FieldName##MinSizeType MinSizeType;                                                     \
            inline static MinSizeType toMinSizeType(const FieldName value)   { return MinSizeType(value); } \
            inline static FieldName fromMinSizeType(const MinSizeType value) { return FieldName(value);   } \
    };

DECLARE_FIELDMETAINFO(Images)
DECLARE_FIELDMETAINFO(ImageInformation)
DECLARE_FIELDMETAINFO(ImageMetadata)
DECLARE_FIELDMETAINFO(ImageComments)
DECLARE_FIELDMETAINFO(ImagePositions)
DECLARE_FIELDMETAINFO(ImageHistoryInfo)
DECLARE_FIELDMETAINFO(VideoMetadata)

/**
 * You can iterate over each of the Enumerations defined above:
 * ImagesIterator, ImageMetadataIterator etc.
 * for (ImagesIterator it; !it.atEnd(); ++it) {}
 */

template<typename FieldName> class DatabaseFieldsEnumIterator
{

public:

    DatabaseFieldsEnumIterator()
      : i(FieldMetaInfo<FieldName>::First)
    {
    }

    bool atEnd() const
    {
        return i > FieldMetaInfo<FieldName>::Last;
    }

    void operator++()
    {
        i = (i << 1);
    }

    FieldName operator*() const
    {
        return FieldName(i);
    }

private:

    int i;
};

/**
 * An iterator that iterates only over the flags which are set
 */
template<typename FieldName> class DatabaseFieldsEnumIteratorSetOnly
{

public:

    DatabaseFieldsEnumIteratorSetOnly(const FieldName setValues)
      : i(),
        values(setValues)
    {
        if (! (*i & values) )
        {
            operator++();
        }
    }

    bool atEnd() const
    {
        return i.atEnd();
    }

    void operator++()
    {
        while (!i.atEnd())
        {
            ++i;

            if (*i & values)
            {
                break;
            }
        }
    }

    FieldName operator*() const
    {
        return *i;
    }

private:

    DatabaseFieldsEnumIterator<FieldName> i;
    const FieldName                       values;
};

#define DATABASEFIELDS_ENUM_ITERATOR(Flag)                                      \
    typedef DatabaseFieldsEnumIterator<Flag> Flag##Iterator;                    \
    typedef DatabaseFieldsEnumIteratorSetOnly<Flag> Flag##IteratorSetOnly;

DATABASEFIELDS_ENUM_ITERATOR(Images)
DATABASEFIELDS_ENUM_ITERATOR(ImageInformation)
DATABASEFIELDS_ENUM_ITERATOR(ImageMetadata)
DATABASEFIELDS_ENUM_ITERATOR(VideoMetadata)
DATABASEFIELDS_ENUM_ITERATOR(ImagePositions)
DATABASEFIELDS_ENUM_ITERATOR(ImageComments)
DATABASEFIELDS_ENUM_ITERATOR(ImageHistoryInfo)

/**
 * For your custom enum, you need to use the CustomEnum class.
 * You need to do an explicit cast.
 */
enum CustomEnumFlags
{
};
Q_DECLARE_FLAGS(CustomEnum, CustomEnumFlags)

#define DATABASEFIELDS_SET_DECLARE_METHODS(Flag, variable)                     \
    Set(const Flag& f)                      { initialize(); variable = f;    } \
    Set(const Flag##Field& f)               { initialize(); variable = f;    } \
    inline Flag& operator=(const Flag& f)   { return variable.operator=(f);  } \
    inline Flag& operator|=(Flag f)         { return variable.operator|=(f); } \
    inline Flag& operator^=(Flag f)         { return variable.operator^=(f); } \
    inline Flag operator|(Flag f) const     { return variable.operator|(f);  } \
    inline Flag operator^(Flag f) const     { return variable.operator^(f);  } \
    inline Flag operator&(Flag f) const     { return variable.operator&(f);  } \
    inline operator Flag() const            { return variable;               } \
    inline bool hasFieldsFrom##Flag() const { return variable & Flag##All;   } \
    inline Flag get##Flag() const           { return variable;               }

/**
 * This class provides a set of all DatabasFields enums,
 * without resorting to a QSet.
 */
class Set
{
public:

    Set()
    {
        initialize();
    }

    void initialize()
    {
        images           = ImagesNone;
        imageInformation = ImageInformationNone;
        imageMetadata    = ImageMetadataNone;
        imageComments    = ImageCommentsNone;
        imagePositions   = ImagePositionsNone;
        imageHistory     = ImageHistoryInfoNone;
        videoMetadata    = VideoMetadataNone;
        customEnum       = (CustomEnum)0;
    }

public:

    DATABASEFIELDS_SET_DECLARE_METHODS(Images,           images)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageInformation, imageInformation)
    DATABASEFIELDS_SET_DECLARE_METHODS(VideoMetadata,    videoMetadata)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageMetadata,    imageMetadata)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageComments,    imageComments)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImagePositions,   imagePositions)
    DATABASEFIELDS_SET_DECLARE_METHODS(ImageHistoryInfo, imageHistory)

    inline bool operator&(const Set& other)
    {
        return (images & other.images)                     ||
               (imageInformation & other.imageInformation) ||
               (imageMetadata & other.imageMetadata)       ||
               (imageComments & other.imageComments)       ||
               (imagePositions & other.imagePositions)     ||
               (imageHistory & other.imageHistory)         ||
               (customEnum & other.customEnum)             ||
               (videoMetadata & other.videoMetadata);
    }

    // overloading operator|= creates ambiguity with the database fields'
    // operator|=, therefore we give it another name.
    inline Set& setFields(const Set& otherSet)
    {
        images|= otherSet.images;
        imageInformation|= otherSet.imageInformation;
        imageMetadata|= otherSet.imageMetadata;
        imageComments|= otherSet.imageComments;
        imagePositions|= otherSet.imagePositions;
        imageHistory|= otherSet.imageHistory;
        customEnum|= otherSet.customEnum;
        videoMetadata|= otherSet.videoMetadata;

        return *this;
    }

    inline CustomEnum& operator=(const CustomEnum& f)
    {
        return customEnum.operator=(f);
    }

    inline CustomEnum& operator|=(CustomEnum f)
    {
        return customEnum.operator|=(f);
    }

    inline CustomEnum& operator^=(CustomEnum f)
    {
        return customEnum.operator^=(f);
    }

    inline CustomEnum operator|(CustomEnum f) const
    {
        return customEnum.operator|(f);
    }

    inline CustomEnum operator^(CustomEnum f) const
    {
        return customEnum.operator^(f);
    }

    inline CustomEnum operator&(CustomEnum f) const
    {
        return customEnum.operator&(f);
    }

#ifdef HAVE_DBUS
    // databasechangesets.cpp
    Set& operator<<(const QDBusArgument& argument);
    const Set& operator>>(QDBusArgument& argument) const;
#endif

private:

    Images           images;
    ImageInformation imageInformation;
    ImageMetadata    imageMetadata;
    VideoMetadata    videoMetadata;
    ImageComments    imageComments;
    ImagePositions   imagePositions;
    ImageHistoryInfo imageHistory;
    CustomEnum       customEnum;
};

#define DATABASEFIELDS_HASH_DECLARE_METHODS(Key, method)                                                                            \
    void insertField(const Key& key, const T& value)           { QHash<unsigned int, T>::insert(method(key), value);              } \
    int remove(const Key& key)                                 { return QHash<unsigned int, T>::remove(method(key));              } \
    int removeAllFields(const Key& key)                                                                                             \
    {                                                                                                                               \
        int removedCount = 0;                                                                                                       \
        for (DatabaseFieldsEnumIteratorSetOnly<Key> it(key); !it.atEnd(); ++it)                                                     \
        {                                                                                                                           \
            removedCount+=remove(*it);                                                                                              \
        }                                                                                                                           \
        return removedCount;                                                                                                        \
    }                                                                                                                               \
    T take(const Key& key)                                     { return QHash<unsigned int, T>::take(method(key));                } \
                                                                                                                                    \
    bool contains(const Key& key) const                        { return QHash<unsigned int, T>::contains(method(key));            } \
    const T value(const Key& key) const                        { return QHash<unsigned int, T>::value(method(key));               } \
    const T value(const Key& key, const T& defaultValue) const { return QHash<unsigned int, T>::value(method(key), defaultValue); } \
                                                                                                                                    \
    T& operator[](const Key& key)                              { return QHash<unsigned int, T>::operator[](method(key));          } \
    const T operator[](const Key& key) const                   { return QHash<unsigned int, T>::operator[](method(key));          } \
                                                                                                                                    \
    QList<T> values(const Key& key) const                      { return QHash<unsigned int, T>::value(method(key));               } \
    int count(const Key& key) const                            { return QHash<unsigned int, T>::count(method(key));               }

/**
 * This class provides a hash on all DatabaseFields enums,
 * allowing to use the enum values as independent keys.
 * You can use the class like a normal QHash with the value type defined
 * by you, and as keys the members of the DatabaseFields enums.
 * You can only use single enum members as keys, not or'ed numbers.
 * You can use one custom enum, cast to DatabaseFields::CustomEnum,
 * which can have at most 26 flag values (1 << 0 to 1 << 26).
 * Pass this as the optional second template parameter.
 */
template <class T>
class Hash : public QHash<unsigned int, T>
{
public:

    // We use the upper 6 bits to distinguish the enums, and give the lower 26 bits to the flags.
    // So we can store up to 64 enums, with 26 flags each.
    static inline unsigned int uniqueKey(Images f)
    {
        return (int)f | (0 << 26);
    }

    static inline unsigned int uniqueKey(ImageInformation f)
    {
        return (int)f | (1 << 26);
    }

    static inline unsigned int uniqueKey(ImageMetadata f)
    {
        return (int)f | (2 << 26);
    }

    static inline unsigned int uniqueKey(ImageComments f)
    {
        return (int)f | (3 << 26);
    }

    static inline unsigned int uniqueKey(ImagePositions f)
    {
        return (int)f | (4 << 26);
    }

    static inline unsigned int uniqueKey(ImageHistoryInfo f)
    {
        return (int)f | (5 << 26);
    }

    static inline unsigned int uniqueKey(VideoMetadata f)
    {
        return (int)f | (6 << 26);
    }

    static inline unsigned int uniqueKey(CustomEnum f)
    {
        return      f | (63 << 26);
    }

    // override relevant methods from QHash
    DATABASEFIELDS_HASH_DECLARE_METHODS(Images, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageInformation, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageMetadata, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(VideoMetadata, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageComments, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImagePositions, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(ImageHistoryInfo, uniqueKey);
    DATABASEFIELDS_HASH_DECLARE_METHODS(CustomEnum, uniqueKey);
};

} // end of namespace DatabaseFields

} // end of namespace Digikam

// must be outside the namespace!
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::Images)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageInformation)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageMetadata)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::VideoMetadata)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageComments)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImagePositions)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFields::ImageHistoryInfo)

#endif // COREDATABASEFIELDS_H
