/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
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

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QUuid>

// Local includes

#include "rawinfo.h"
#include "drawdecoder.h"
#include "filereadwritelock.h"
#include "metadatasettings.h"
#include "template.h"
#include "dimg.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

DMetadata::DMetadata()
    : MetaEngine()
{
    registerMetadataSettings();
}

DMetadata::DMetadata(const QString& filePath)
    : MetaEngine()
{
    registerMetadataSettings();
    load(filePath);
}

DMetadata::DMetadata(const MetaEngineData& data)
    : MetaEngine(data)
{
    registerMetadataSettings();
}

DMetadata::~DMetadata()
{
}

void DMetadata::registerMetadataSettings()
{
    setSettings(MetadataSettings::instance()->settings());
}

void DMetadata::setSettings(const MetadataSettingsContainer& settings)
{
    setUseXMPSidecar4Reading(settings.useXMPSidecar4Reading);
    setWriteRawFiles(settings.writeRawFiles);
    setMetadataWritingMode(settings.metadataWritingMode);
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
}

bool DMetadata::load(const QString& filePath)
{
    // In first, we trying to get metadata using Exiv2,
    // else we will use other engine to extract minimal information.

    FileReadLocker lock(filePath);

    if (!MetaEngine::load(filePath))
    {
        if (!loadUsingRawEngine(filePath))
        {
            if (!loadUsingFFmpeg(filePath))
            {
                return false;
            }
        }
    }

    return true;
}

bool DMetadata::save(const QString& filePath, bool setVersion) const
{
    FileWriteLocker lock(filePath);
    return MetaEngine::save(filePath, setVersion);
}

bool DMetadata::applyChanges() const
{
    FileWriteLocker lock(getFilePath());
    return MetaEngine::applyChanges();
}

bool DMetadata::loadUsingRawEngine(const QString& filePath)
{
    RawInfo identify;

    if (DRawDecoder::rawFileIdentify(identify, filePath))
    {
        long int num=1, den=1;

        if (!identify.model.isNull())
        {
            setExifTagString("Exif.Image.Model", identify.model);
        }

        if (!identify.make.isNull())
        {
            setExifTagString("Exif.Image.Make", identify.make);
        }

        if (!identify.owner.isNull())
        {
            setExifTagString("Exif.Image.Artist", identify.owner);
        }

        if (identify.sensitivity != -1)
        {
            setExifTagLong("Exif.Photo.ISOSpeedRatings", lroundf(identify.sensitivity));
        }

        if (identify.dateTime.isValid())
        {
            setImageDateTime(identify.dateTime, false);
        }

        if (identify.exposureTime != -1.0)
        {
            convertToRationalSmallDenominator(identify.exposureTime, &num, &den);
            setExifTagRational("Exif.Photo.ExposureTime", num, den);
        }

        if (identify.aperture != -1.0)
        {
            convertToRational(identify.aperture, &num, &den, 8);
            setExifTagRational("Exif.Photo.ApertureValue", num, den);
        }

        if (identify.focalLength != -1.0)
        {
            convertToRational(identify.focalLength, &num, &den, 8);
            setExifTagRational("Exif.Photo.FocalLength", num, den);
        }

        if (identify.imageSize.isValid())
        {
            setImageDimensions(identify.imageSize);
        }

        // A RAW image is always uncalibrated. */
        setImageColorWorkSpace(WORKSPACE_UNCALIBRATED);

        return true;
    }

    return false;
}

int DMetadata::getMSecsInfo() const
{
    int ms  = 0;
    bool ok = mSecTimeStamp("Exif.Photo.SubSecTime", ms);
    if (ok) return ms;

    ok      = mSecTimeStamp("Exif.Photo.SubSecTimeOriginal", ms);
    if (ok) return ms;

    ok = mSecTimeStamp("Exif.Photo.SubSecTimeDigitized", ms);
    if (ok) return ms;

    return 0;
}

bool DMetadata::mSecTimeStamp(const char* const exifTagName, int& ms) const
{
    bool ok     = false;
    QString val = getExifTagString(exifTagName);

    if (!val.isEmpty())
    {
        int sub = val.toUInt(&ok);

        if (ok)
        {
            int _ms = (int)(QString::fromLatin1("0.%1").arg(sub).toFloat(&ok) * 1000.0);

            if (ok)
            {
                ms = _ms;
                qCDebug(DIGIKAM_METAENGINE_LOG) << "msec timestamp: " << ms;
            }
        }
    }

    return ok;
}

CaptionsMap DMetadata::getImageComments(const DMetadataSettingsContainer& settings) const
{
    if (getFilePath().isEmpty())
    {
        return CaptionsMap();
    }

    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap commentsMap;
    QString                commonAuthor;

    // In first try to get captions properties from digiKam XMP namespace

    if (supportXmp())
    {
        authorsMap = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames",    false);
        datesMap   = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", false);

        if (authorsMap.isEmpty() && commonAuthor.isEmpty())
        {
            QString xmpAuthors = getXmpTagString("Xmp.acdsee.author", false);

            if (!xmpAuthors.isEmpty())
            {
                authorsMap.insert(QLatin1String("x-default"), xmpAuthors);
            }
        }
    }

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.
    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
    {
        commonAuthor = descriptionWriter.toString();
    }

    // In first, we check XMP alternative language tags to create map of values.

    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (NamespaceEntry entry : settings.getReadMapping(QLatin1String(DM_COMMENT_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        QString commentString;
        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                switch(entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                        if (xmpSupported)
                            commentString = getXmpTagStringLangAlt(nameSpace, QString(), false);
                        break;
                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                        if (xmpSupported)
                            commentsMap = getXmpTagStringListLangAlt(nameSpace, false);
                        break;
                    case NamespaceEntry::COMMENT_XMP:
                        if (xmpSupported)
                            commentString = getXmpTagString("Xmp.acdsee.notes", false);
                        break;
                    case NamespaceEntry::COMMENT_JPEG:
                        // Now, we trying to get image comments, outside of XMP.
                        // For JPEG, string is extracted from JFIF Comments section.
                        // For PNG, string is extracted from iTXt chunk.
                        commentString = getCommentsDecoded();
                    default:
                        break;
                }
                break;
            case NamespaceEntry::IPTC:
                if (iptcSupported)
                    commentString = getIptcTagString(nameSpace, false);
                break;
            case NamespaceEntry::EXIF:
                if (exivSupported)
                    commentString = getExifComment();
                break;
            default:
                break;
        }

        if (!commentString.isEmpty() &&!commentString.trimmed().isEmpty())
        {
            commentsMap.insert(QLatin1String("x-default"), commentString);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        if (!commentsMap.isEmpty())
        {
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageComments(const CaptionsMap& comments, const DMetadataSettingsContainer &settings) const
{
/*
    // See bug #139313: An empty string is also a valid value
    if (comments.isEmpty())
          return false;
*/

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Comment: " << comments;

    // In first, set captions properties to digiKam XMP namespace

    if (supportXmp())
    {
        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames", comments.authorsList()))
        {
            return false;
        }

        QString defaultAuthor  = comments.value(QLatin1String("x-default")).author;
        removeXmpTag("Xmp.acdsee.author");

        if (!defaultAuthor.isNull())
        {
            if (!setXmpTagString("Xmp.acdsee.author", defaultAuthor))
            {
                return false;
            }
        }

        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", comments.datesList()))
        {
            return false;
        }
    }

    QString defaultComment        = comments.value(QLatin1String("x-default")).caption;
    QList<NamespaceEntry> toWrite = settings.getReadMapping(QLatin1String(DM_COMMENT_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QLatin1String(DM_COMMENT_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (entry.namespaceName.contains(QLatin1String("Xmp.")))
                    removeXmpTag(nameSpace);

                switch(entry.specialOpts)
                {
                    case NamespaceEntry::COMMENT_ALTLANG:
                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagStringLangAlt(nameSpace, defaultComment, QString()))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image comment failed" << nameSpace;
                                return false;
                            }
                        }
                        break;

                    case NamespaceEntry::COMMENT_ATLLANGLIST:
                        if (!setXmpTagStringListLangAlt(nameSpace, comments.toAltLangMap()))
                        {
                            return false;
                        }
                        break;

                    case NamespaceEntry::COMMENT_XMP:
                        if (!defaultComment.isNull())
                        {
                            if (!setXmpTagString(nameSpace, defaultComment))
                            {
                                return false;
                            }
                        }
                        break;

                    case NamespaceEntry::COMMENT_JPEG:
                        // In first we set image comments, outside of Exif, XMP, and IPTC.
                        if (!setComments(defaultComment.toUtf8()))
                        {
                            return false;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case NamespaceEntry::IPTC:
                removeIptcTag(nameSpace);

                if (!defaultComment.isNull())
                {
                    defaultComment.truncate(2000);

                    if (!setIptcTagString(nameSpace, defaultComment))
                    {
                        return false;
                    }
                }
                break;

            case NamespaceEntry::EXIF:
                if (!setExifComment(defaultComment))
                {
                    return false;
                }
                break;

            default:
                break;
        }
    }

    return true;
}

int DMetadata::getImagePickLabel() const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.PickLabel", false);

        if (!value.isEmpty())
        {
            bool ok     = false;
            long pickId = value.toLong(&ok);

            if (ok && pickId >= NoPickLabel && pickId <= AcceptedLabel)
            {
                return pickId;
            }
        }
    }

    return -1;
}

int DMetadata::getImageColorLabel() const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ColorLabel", false);

        if (value.isEmpty())
        {
            // Nikon NX use this XMP tags to store Color Labels
            value = getXmpTagString("Xmp.photoshop.Urgency", false);
        }

        if (!value.isEmpty())
        {
            bool ok      = false;
            long colorId = value.toLong(&ok);

            if (ok && colorId >= NoColorLabel && colorId <= WhiteLabel)
            {
                return colorId;
            }
        }

        // LightRoom use this tag to store color name as string.
        // Values are limited : see bug #358193.

        value = getXmpTagString("Xmp.xmp.Label", false);

        if (value == QLatin1String("Blue"))
        {
            return BlueLabel;
        }
        else if (value == QLatin1String("Green"))
        {
            return GreenLabel;
        }
        else if (value == QLatin1String("Red"))
        {
            return RedLabel;
        }
        else if (value == QLatin1String("Yellow"))
        {
            return YellowLabel;
        }
        else if (value == QLatin1String("Purple"))
        {
            return MagentaLabel;
        }
    }

    return -1;
}

CaptionsMap DMetadata::getImageTitles() const
{
    if (getFilePath().isEmpty())
        return CaptionsMap();

    CaptionsMap            captionsMap;
    MetaEngine::AltLangMap authorsMap;
    MetaEngine::AltLangMap datesMap;
    MetaEngine::AltLangMap titlesMap;
    QString                commonAuthor;

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.
    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
        commonAuthor = descriptionWriter.toString();

    // In first, we check XMP alternative language tags to create map of values.

    if (hasXmp())
    {
        titlesMap = getXmpTagStringListLangAlt("Xmp.dc.title", false);

        if (!titlesMap.isEmpty())
        {
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        QString xmpTitle = getXmpTagString("Xmp.acdsee.caption" ,false);

        if (!xmpTitle.isEmpty() && !xmpTitle.trimmed().isEmpty())
        {
            titlesMap.insert(QLatin1String("x-default"), xmpTitle);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    // We trying to get IPTC title

    if (hasIptc())
    {
        QString iptcTitle = getIptcTagString("Iptc.Application2.ObjectName", false);

        if (!iptcTitle.isEmpty() && !iptcTitle.trimmed().isEmpty())
        {
            titlesMap.insert(QLatin1String("x-default"), iptcTitle);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageTitles(const CaptionsMap& titles) const
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Title: " << titles;

    QString defaultTitle = titles[QLatin1String("x-default")].caption;

    // In First we write comments into XMP. Language Alternative rule is not yet used.

    if (supportXmp())
    {
        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values
        if (!setXmpTagStringListLangAlt("Xmp.dc.title", titles.toAltLangMap()))
        {
            return false;
        }

        removeXmpTag("Xmp.acdsee.caption");

        if (!defaultTitle.isEmpty())
        {
            if (!setXmpTagString("Xmp.acdsee.caption", defaultTitle))
            {
                return false;
            }
        }
    }

    // In Second we write comments into IPTC.
    // Note that Caption IPTC tag is limited to 64 char and ASCII charset.

    removeIptcTag("Iptc.Application2.ObjectName");

    if (!defaultTitle.isNull())
    {
        defaultTitle.truncate(64);

        // See if we have any non printable chars in there. If so, skip IPTC
        // to avoid confusing other apps and web services with invalid tags.
        bool hasInvalidChar = false;

        for (QString::const_iterator c = defaultTitle.constBegin(); c != defaultTitle.constEnd(); ++c)
        {
            if (!(*c).isPrint())
            {
                hasInvalidChar = true;
                break;
            }
        }

        if (!hasInvalidChar)
        {
            if (!setIptcTagString("Iptc.Application2.ObjectName", defaultTitle))
                return false;
        }
    }

    return true;
}

int DMetadata::getImageRating(const DMetadataSettingsContainer &settings) const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    long rating        = -1;
    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    for (NamespaceEntry entry : settings.getReadMapping(QLatin1String(DM_RATING_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();
        QString value;

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (xmpSupported)
                    value = getXmpTagString(nameSpace, false);
                break;
            case NamespaceEntry::IPTC:
                if (iptcSupported)
                    value = QString::fromUtf8(getIptcTagData(nameSpace));
                break;
            case NamespaceEntry::EXIF:
                if (exivSupported)
                    getExifTagLong(nameSpace, rating);
                break;
            default:
                break;
        }

        if (!value.isEmpty())
        {
            bool ok = false;
            rating  = value.toLong(&ok);

            if (!ok)
            {
                return -1;
            }

        }
        int index = entry.convertRatio.indexOf(rating);

        // Exact value was not found,but rating is in range,
        // so we try to aproximate it
        if ((index == -1)                         &&
            (rating > entry.convertRatio.first()) &&
            (rating < entry.convertRatio.last()))
        {
            for (int i = 0 ; i < entry.convertRatio.size() ; i++)
            {
                if (rating > entry.convertRatio.at(i))
                {
                    index = i;
                }
            }
        }

        if (index != -1)
        {
            return index;
        }
    }

    return -1;
}

bool DMetadata::setImagePickLabel(int pickId) const
{
    if (pickId < NoPickLabel || pickId > AcceptedLabel)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Pick Label value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Pick Label: " << pickId;

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.PickLabel", QString::number(pickId)))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::setImageColorLabel(int colorId) const
{
    if (colorId < NoColorLabel || colorId > WhiteLabel)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Color Label value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Color Label: " << colorId;

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ColorLabel", QString::number(colorId)))
        {
            return false;
        }

        // Nikon NX use this XMP tags to store Color Labels
        if (!setXmpTagString("Xmp.photoshop.Urgency", QString::number(colorId)))
        {
            return false;
        }

        // LightRoom use this XMP tags to store Color Labels name
        // Values are limited : see bug #358193.

        QString LRLabel;

        switch(colorId)
        {
            case BlueLabel:
                LRLabel = QLatin1String("Blue");
                break;
            case GreenLabel:
                LRLabel = QLatin1String("Green");
                break;
            case RedLabel:
                LRLabel = QLatin1String("Red");
                break;
            case YellowLabel:
                LRLabel = QLatin1String("Yellow");
                break;
            case MagentaLabel:
                LRLabel = QLatin1String("Purple");
                break;
        }

        if (!LRLabel.isEmpty())
        {
            if (!setXmpTagString("Xmp.xmp.Label", LRLabel))
            {
                return false;
            }
        }
    }

    return true;
}

bool DMetadata::setImageRating(int rating, const DMetadataSettingsContainer &settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Urgency to store Rating.
    // Now this way is obsolete, and we use standard XMP rating tag instead.

    if (rating < RatingMin || rating > RatingMax)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Rating value to write is out of range!";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Rating:" << rating;

    QList<NamespaceEntry> toWrite = settings.getReadMapping(QLatin1String(DM_RATING_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QLatin1String(DM_RATING_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:
                if (!setXmpTagString(nameSpace, QString::number(entry.convertRatio.at(rating))))
                {
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting rating failed" << nameSpace;
                    return false;
                }
                break;
            case NamespaceEntry::EXIF:
                if (!setExifTagLong(nameSpace, rating))
                {
                    return false;
                }
                break;
            case NamespaceEntry::IPTC: // IPTC rating deprecated
            default:
                break;
        }
    }

    // Set Exif rating tag used by Windows Vista.

    if (!setExifTagLong("Exif.Image.0x4746", rating))
    {
        return false;
    }

    // Wrapper around rating percents managed by Windows Vista.
    int ratePercents = 0;

    switch (rating)
    {
        case 0:
            ratePercents = 0;
            break;
        case 1:
            ratePercents = 1;
            break;
        case 2:
            ratePercents = 25;
            break;
        case 3:
            ratePercents = 50;
            break;
        case 4:
            ratePercents = 75;
            break;
        case 5:
            ratePercents = 99;
            break;
    }

    if (!setExifTagLong("Exif.Image.0x4749", ratePercents))
    {
        return false;
    }

    return true;
}

bool DMetadata::setImageHistory(QString& imageHistoryXml) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ImageHistory", imageHistoryXml))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

QString DMetadata::getImageHistory() const
{
    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ImageHistory", false);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading image history " << value;
        return value;
    }

    return QString();
}

bool DMetadata::hasImageHistoryTag() const
{
    if (hasXmp())
    {
        if (QString(getXmpTagString("Xmp.digiKam.ImageHistory", false)).length() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

QString DMetadata::getImageUniqueId() const
{
    QString exifUid;

    if (hasXmp())
    {
        QString uuid = getXmpTagString("Xmp.digiKam.ImageUniqueID");

        if (!uuid.isEmpty())
        {
            return uuid;
        }

        exifUid = getXmpTagString("Xmp.exif.ImageUniqueId");
    }

    if (exifUid.isEmpty())
    {
        exifUid = getExifTagString("Exif.Photo.ImageUniqueID");
    }

    // same makers may choose to use a "click counter" to generate the id,
    // which is then weak and not a universally unique id
    // The Exif ImageUniqueID is 128bit, or 32 hex digits.
    // If the first 20 are zero, it's probably a counter,
    // the left 12 are sufficient for more then 10^14 clicks.
    if (!exifUid.isEmpty() && !exifUid.startsWith(QLatin1String("00000000000000000000")))
    {
        if (getExifTagString("Exif.Image.Make").contains(QLatin1String("SAMSUNG"), Qt::CaseInsensitive))
        {
            // Generate for Samsung a new random 32 hex digits unique ID.
            QString imageUniqueID(QUuid::createUuid().toString());
            imageUniqueID.remove(QLatin1Char('-'));
            imageUniqueID.remove(0, 1).chop(1);

            return imageUniqueID;
        }

        return exifUid;
    }

    // Exif.Image.ImageID can also be a pathname, so it's not sufficiently unique

    QString dngUid = getExifTagString("Exif.Image.RawDataUniqueID");

    if (!dngUid.isEmpty())
    {
        return dngUid;
    }

    return QString();
}

bool DMetadata::setImageUniqueId(const QString& uuid) const
{
    if (supportXmp())
    {
        return setXmpTagString("Xmp.digiKam.ImageUniqueID", uuid);
    }

    return false;
}

PhotoInfoContainer DMetadata::getPhotographInformation() const
{
    PhotoInfoContainer photoInfo;

    if (hasExif() || hasXmp())
    {
        photoInfo.dateTime = getImageDateTime();

        // -----------------------------------------------------------------------------------

        photoInfo.make     = getExifTagString("Exif.Image.Make");

        if (photoInfo.make.isEmpty())
        {
            photoInfo.make = getXmpTagString("Xmp.tiff.Make");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.model    = getExifTagString("Exif.Image.Model");

        if (photoInfo.model.isEmpty())
        {
            photoInfo.model = getXmpTagString("Xmp.tiff.Model");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.lens     = getLensDescription();

        // -----------------------------------------------------------------------------------

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.FNumber");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.ApertureValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ExposureTime");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getXmpTagString("Xmp.exif.ExposureMode");
        }

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getExifTagString("Exif.CanonCs.MeteringMode");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getXmpTagString("Xmp.exif.ExposureProgram");
        }

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getExifTagString("Exif.CanonCs.ExposureProgram");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getXmpTagString("Xmp.exif.FocalLength");
        }

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getExifTagString("Exif.Canon.FocalLength");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");

        if (photoInfo.focalLength35mm.isEmpty())
        {
            photoInfo.focalLength35mm = getXmpTagString("Xmp.exif.FocalLengthIn35mmFilm");
        }

        // -----------------------------------------------------------------------------------

        QStringList ISOSpeedTags;

        ISOSpeedTags << QLatin1String("Exif.Photo.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Exif.Photo.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.Image.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.CanonSi.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.CanonCs.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon1.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon2.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon3.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO2");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsNew.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsOld.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs5D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs7D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1MltCsA100.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Pentax.ISO");
        ISOSpeedTags << QLatin1String("Exif.Olympus.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Samsung2.ISO");

        photoInfo.sensitivity = getExifTagStringFromTagsList(ISOSpeedTags);

        // -----------------------------------------------------------------------------------

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getXmpTagString("Xmp.exif.Flash");
        }

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getExifTagString("Exif.CanonCs.FlashActivity");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");

        if (photoInfo.whiteBalance.isEmpty())
        {
            photoInfo.whiteBalance = getXmpTagString("Xmp.exif.WhiteBalance");
        }

        // -----------------------------------------------------------------------------------

        double l, L, a;
        photoInfo.hasCoordinates = getGPSInfo(a, l, L);
    }

    return photoInfo;
}

VideoInfoContainer DMetadata::getVideoInformation() const
{
    VideoInfoContainer videoInfo;

    if (hasXmp())
    {
        if (videoInfo.aspectRatio.isEmpty())
        {
            videoInfo.aspectRatio = getMetadataField(MetadataInfo::AspectRatio).toString();
        }

        if (videoInfo.audioBitRate.isEmpty())
        {
            videoInfo.audioBitRate = getXmpTagString("Xmp.audio.SampleRate");
        }

        if (videoInfo.audioChannelType.isEmpty())
        {
            videoInfo.audioChannelType = getXmpTagString("Xmp.audio.ChannelType");
        }

        if (videoInfo.audioCodec.isEmpty())
        {
            videoInfo.audioCodec = getXmpTagString("Xmp.audio.Codec");
        }

        if (videoInfo.duration.isEmpty())
        {
            videoInfo.duration = getXmpTagString("Xmp.video.duration");
        }

        if (videoInfo.frameRate.isEmpty())
        {
            videoInfo.frameRate = getXmpTagString("Xmp.video.FrameRate");
        }

        if (videoInfo.videoCodec.isEmpty())
        {
            videoInfo.videoCodec = getXmpTagString("Xmp.video.Codec");
        }
    }

    return videoInfo;
}

bool DMetadata::getImageTagsPath(QStringList& tagsPath,
                                 const DMetadataSettingsContainer& settings) const
{
    for (NamespaceEntry entry : settings.getReadMapping(QLatin1String(DM_TAG_CONTAINER)))
    {
        if (entry.isDisabled)
            continue;

        int index                                  = 0;
        QString currentNamespace                   = entry.namespaceName;
        NamespaceEntry::SpecialOptions currentOpts = entry.specialOpts;

        // Some namespaces have altenative paths, we must search them both

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:

                while(index < 2)
                {
                    const std::string myStr = currentNamespace.toStdString();
                    const char* nameSpace   = myStr.data();

                    switch(currentOpts)
                    {
                        case NamespaceEntry::TAG_XMPBAG:
                            tagsPath = getXmpTagStringBag(nameSpace, false);
                            break;
                        case NamespaceEntry::TAG_XMPSEQ:
                            tagsPath = getXmpTagStringSeq(nameSpace, false);
                            break;
                        case NamespaceEntry::TAG_ACDSEE:
                            getACDSeeTagsPath(tagsPath);
                            break;
                        // not used here, to suppress warnings
                        case NamespaceEntry::COMMENT_XMP:
                        case NamespaceEntry::COMMENT_ALTLANG:
                        case NamespaceEntry::COMMENT_ATLLANGLIST:
                        case NamespaceEntry::NO_OPTS:
                        default:
                            break;
                    }

                    if (!tagsPath.isEmpty())
                    {
                        if (entry.separator != QLatin1String("/"))
                        {
                            tagsPath = tagsPath.replaceInStrings(entry.separator, QLatin1String("/"));
                        }

                        return true;
                    }
                    else if (!entry.alternativeName.isEmpty())
                    {
                        currentNamespace = entry.alternativeName;
                        currentOpts      = entry.secondNameOpts;
                    }
                    else
                    {
                        break; // no alternative namespace, go to next one
                    }

                    index++;
                }

                break;

            case NamespaceEntry::IPTC:
                // Try to get Tags Path list from IPTC keywords.
                // digiKam 0.9.x has used IPTC keywords to store Tags Path list.
                // This way is obsolete now since digiKam support XMP because IPTC
                // do not support UTF-8 and have strings size limitation. But we will
                // let the capability to import it for interworking issues.
                tagsPath = getIptcKeywords();

                if (!tagsPath.isEmpty())
                {
                    // Work around to Imach tags path list hosted in IPTC with '.' as separator.
                    QStringList ntp = tagsPath.replaceInStrings(entry.separator, QLatin1String("/"));

                    if (ntp != tagsPath)
                    {
                        tagsPath = ntp;
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from Imach: " << tagsPath;
                    }

                    return true;
                }

                break;

            case NamespaceEntry::EXIF:
            {
                // Try to get Tags Path list from Exif Windows keywords.
                QString keyWords = getExifTagString("Exif.Image.XPKeywords", false);

                if (!keyWords.isEmpty())
                {
                    tagsPath = keyWords.split(entry.separator);

                    if (!tagsPath.isEmpty())
                    {
                        return true;
                    }
                }

                break;
            }

            default:
                break;
        }
    }

    return false;
}

bool DMetadata::setImageTagsPath(const QStringList& tagsPath, const DMetadataSettingsContainer& settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Keywords for that.
    // Now this way is obsolete, and we use XMP instead.

    // Set the new Tags path list. This is set, not add-to like setXmpKeywords.
    // Unlike the other keyword fields, we do not need to merge existing entries.
    QList<NamespaceEntry> toWrite = settings.getReadMapping(QLatin1String(DM_TAG_CONTAINER));

    if (!settings.unifyReadWrite())
        toWrite = settings.getWriteMapping(QLatin1String(DM_TAG_CONTAINER));

    for (NamespaceEntry entry : toWrite)
    {
        if (entry.isDisabled)
            continue;

        QStringList newList;

        // get keywords from tags path, for type tag
        for (QString tagPath : tagsPath)
        {
            newList.append(tagPath.split(QLatin1String("/")).last());
        }

        switch(entry.subspace)
        {
            case NamespaceEntry::XMP:

                if (supportXmp())
                {
                    if (entry.tagPaths != NamespaceEntry::TAG)
                    {
                        newList = tagsPath;

                        if (entry.separator.compare(QLatin1String("/")) != 0)
                        {
                            newList = newList.replaceInStrings(QLatin1String("/"), entry.separator);
                        }
                    }

                    const std::string myStr = entry.namespaceName.toStdString();
                    const char* nameSpace   = myStr.data();

                    switch(entry.specialOpts)
                    {
                        case NamespaceEntry::TAG_XMPSEQ:

                            if (!setXmpTagStringSeq(nameSpace, newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                            break;

                        case NamespaceEntry::TAG_XMPBAG:

                            if (!setXmpTagStringBag(nameSpace, newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                            break;

                        case NamespaceEntry::TAG_ACDSEE:

                            if (!setACDSeeTagsPath(newList))
                            {
                                qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << nameSpace;
                                return false;
                            }

                        default:
                            break;
                    }
                }

                break;

            case NamespaceEntry::IPTC:

                if (entry.namespaceName == QLatin1String("Iptc.Application2.Keywords"))
                {
                    if (!setIptcKeywords(getIptcKeywords(), newList))
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting image paths failed" << entry.namespaceName;
                        return false;
                    }
                }

            default:
                break;
        }
    }

    return true;
}

bool DMetadata::getACDSeeTagsPath(QStringList &tagsPath) const
{
    // Try to get Tags Path list from ACDSee 8 Pro categories.
    QString xmlACDSee = getXmpTagString("Xmp.acdsee.categories", false);

    if (!xmlACDSee.isEmpty())
    {
        xmlACDSee.remove(QLatin1String("</Categories>"));
        xmlACDSee.remove(QLatin1String("<Categories>"));
        xmlACDSee.replace(QLatin1String("/"), QLatin1String("\\"));

        QStringList xmlTags = xmlACDSee.split(QLatin1String("<Category Assigned"));
        int category        = 0;

        foreach(const QString& tags, xmlTags)
        {
            if (!tags.isEmpty())
            {
                int count  = tags.count(QLatin1String("<\\Category>"));
                int length = tags.length() - (11 * count) - 5;

                if (category == 0)
                {
                    tagsPath << tags.mid(5, length);
                }
                else
                {
                    tagsPath.last().append(QLatin1String("/") + tags.mid(5, length));
                }

                category = category - count + 1;

                if (tags.left(5) == QLatin1String("=\"1\">") && category > 0)
                {
                    tagsPath << tagsPath.last().section(QLatin1String("/"), 0, category - 1);
                }
            }
        }

        if (!tagsPath.isEmpty())
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Tags Path imported from ACDSee: " << tagsPath;
            return true;
        }
    }

    return false;
}

bool DMetadata::setACDSeeTagsPath(const QStringList &tagsPath) const
{
    // Converting Tags path list to ACDSee 8 Pro categories.
    const QString category(QLatin1String("<Category Assigned=\"%1\">"));
    QStringList splitTags;
    QStringList xmlTags;

    foreach(const QString& tags, tagsPath)
    {
        splitTags   = tags.split(QLatin1String("/"));
        int current = 0;

        for (int index = 0; index < splitTags.size(); index++)
        {
            int tagIndex = xmlTags.indexOf(category.arg(0) + splitTags[index]);

            if (tagIndex == -1)
            {
                tagIndex = xmlTags.indexOf(category.arg(1) + splitTags[index]);
            }

            splitTags[index].insert(0, category.arg(index == splitTags.size() - 1 ? 1 : 0));

            if (tagIndex == -1)
            {
                if (index == 0)
                {
                    xmlTags << splitTags[index];
                    xmlTags << QLatin1String("</Category>");
                    current = xmlTags.size() - 1;
                }
                else
                {
                    xmlTags.insert(current, splitTags[index]);
                    xmlTags.insert(current + 1, QLatin1String("</Category>"));
                    current++;
                }
            }
            else
            {
                if (index == splitTags.size() - 1)
                {
                    xmlTags[tagIndex] = splitTags[index];
                }

                current = tagIndex + 1;
            }
        }
    }

    QString xmlACDSee = QLatin1String("<Categories>") + xmlTags.join(QLatin1String("")) + QLatin1String("</Categories>");
    qCDebug(DIGIKAM_METAENGINE_LOG) << "xmlACDSee" << xmlACDSee;
    removeXmpTag("Xmp.acdsee.categories");

    if (!xmlTags.isEmpty())
    {
        if (!setXmpTagString("Xmp.acdsee.categories", xmlACDSee))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::getImageFacesMap(QMultiMap<QString,QVariant>& faces) const
{
    faces.clear();
    // The example code for Exiv2 says:
    // > There are no specialized values for structures, qualifiers and nested
    // > types. However, these can be added by using an XmpTextValue and a path as
    // > the key.
    // I think that means I have to iterate over the WLPG face tags in the clunky
    // way below (guess numbers and look them up as strings). (Leif)
    const QString personPathTemplate = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:PersonDisplayName");
    const QString rectPathTemplate   = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:Rectangle");

    for (int i = 1; ; i++)
    {
        QString person = getXmpTagString(personPathTemplate.arg(i).toLatin1().constData(), false);

        if (person.isEmpty())
            break;

        // The WLPG tags have the format X.XX, Y.YY, W.WW, H.HH
        // That is, four decimal numbers ranging from 0-1.
        // The top left position is indicated by X.XX, Y.YY (as a
        // percentage of the width/height of the entire image).
        // Similarly the width and height of the face's box are
        // indicated by W.WW and H.HH.
        QString rectString = getXmpTagString(rectPathTemplate.arg(i).toLatin1().constData(), false);
        QStringList list   = rectString.split(QLatin1Char(','));

        if (list.size() < 4)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Cannot parse WLPG rectangle string" << rectString;
            continue;
        }

        QRectF rect(list.at(0).toFloat(),
                    list.at(1).toFloat(),
                    list.at(2).toFloat(),
                    list.at(3).toFloat());

        faces.insertMulti(person, rect);
    }

    /** Read face tags only if libkexiv can write them, otherwise
     *  garbage tags will be generated on image transformation
     */

    // Read face tags as saved by Picasa
    // http://www.exiv2.org/tags-xmp-mwg-rs.html
    const QString mwg_personPathTemplate  = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Name");
    const QString mwg_rect_x_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:x");
    const QString mwg_rect_y_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:y");
    const QString mwg_rect_w_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:w");
    const QString mwg_rect_h_PathTemplate = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:h");

    for (int i = 1; ; i++)
    {
        QString person = getXmpTagString(mwg_personPathTemplate.arg(i).toLatin1().constData(), false);

        if (person.isEmpty())
            break;

        // x and y is the center point
        float x = getXmpTagString(mwg_rect_x_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float y = getXmpTagString(mwg_rect_y_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float w = getXmpTagString(mwg_rect_w_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        float h = getXmpTagString(mwg_rect_h_PathTemplate.arg(i).toLatin1().constData(), false).toFloat();
        QRectF rect(x - w/2,
                    y - h/2,
                    w,
                    h);

        faces.insertMulti(person, rect);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Found new rect " << person << " "<< rect;
    }

    return !faces.isEmpty();
}

bool DMetadata::setImageFacesMap(QMultiMap< QString, QVariant >& facesPath, bool write) const
{
    QString qxmpTagName(QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList"));
    QString nameTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:unit");

    QString winQxmpTagName = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions");
    QString winRectTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:Rectangle");
    QString winNameTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:PersonDisplayName");

    if (!write)
    {
        QString check = getXmpTagString(nameTagKey.arg(1).toLatin1().constData());

        if (check.isEmpty())
            return true;
    }

    setXmpTagString(qxmpTagName.toLatin1().constData(),
                    QString(), MetaEngine::XmpTagType(1));

    setXmpTagString(winQxmpTagName.toLatin1().constData(),
                    QString(), MetaEngine::XmpTagType(1));

    QMap<QString, QVariant>::const_iterator it = facesPath.constBegin();
    int i   = 1;
    bool ok = true;

    while (it != facesPath.constEnd())
    {
        qreal x, y, w, h;
        it.value().toRectF().getRect(&x, &y, &w, &h);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Set face region:" << x << y << w << h;

        /** Write face tags in Windows Live Photo format **/

        QString rectString;

        rectString.append(QString::number(x) + QLatin1String(", "));
        rectString.append(QString::number(y) + QLatin1String(", "));
        rectString.append(QString::number(w) + QLatin1String(", "));
        rectString.append(QString::number(h));

        /** Set tag rect **/
        setXmpTagString(winRectTagKey.arg(i).toLatin1().constData(), rectString,
                             MetaEngine::XmpTagType(0));
        /** Set tag name **/

        setXmpTagString(winNameTagKey.arg(i).toLatin1().constData(),it.key(),
                             MetaEngine::XmpTagType(0));

        /** Writing rectangle in Metadata Group format **/
        x += w/2;
        y += h/2;

        /** Set tag name **/
        ok &= setXmpTagString(nameTagKey.arg(i).toLatin1().constData(),
                              it.key(),MetaEngine::XmpTagType(0));
        /** Set tag type as Face **/
        ok &= setXmpTagString(typeTagKey.arg(i).toLatin1().constData(),
                              QLatin1String("Face"), MetaEngine::XmpTagType(0));

        /** Set tag Area, with xmp type struct **/
        ok &= setXmpTagString(areaTagKey.arg(i).toLatin1().constData(),
                              QString(), MetaEngine::XmpTagType(2));

        /** Set stArea:x inside Area structure **/
        ok &= setXmpTagString(areaxTagKey.arg(i).toLatin1().constData(),
                              QString::number(x), MetaEngine::XmpTagType(0));

        /** Set stArea:y inside Area structure **/
        ok &= setXmpTagString(areayTagKey.arg(i).toLatin1().constData(),
                              QString::number(y), MetaEngine::XmpTagType(0));

        /** Set stArea:w inside Area structure **/
        ok &= setXmpTagString(areawTagKey.arg(i).toLatin1().constData(),
                              QString::number(w), MetaEngine::XmpTagType(0));

        /** Set stArea:h inside Area structure **/
        ok &= setXmpTagString(areahTagKey.arg(i).toLatin1().constData(),
                              QString::number(h), MetaEngine::XmpTagType(0));

        /** Set stArea:unit inside Area structure  as normalized **/
        ok &= setXmpTagString(areanormTagKey.arg(i).toLatin1().constData(),
                              QLatin1String("normalized"), MetaEngine::XmpTagType(0));

        ++it;
        ++i;
    }

    return ok;
}

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

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Applying Metadata Template: " << t.templateTitle() << " :: " << authors;

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

    foreach(QString str, list) // krazy:exclude=foreach
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

static bool hasValidField(const QVariantList& list)
{
    for (QVariantList::const_iterator it = list.constBegin();
         it != list.constEnd(); ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
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

IptcCoreContactInfo DMetadata::getCreatorContactInfo() const
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreContactInfoCity
           << MetadataInfo::IptcCoreContactInfoCountry
           << MetadataInfo::IptcCoreContactInfoAddress
           << MetadataInfo::IptcCoreContactInfoPostalCode
           << MetadataInfo::IptcCoreContactInfoProvinceState
           << MetadataInfo::IptcCoreContactInfoEmail
           << MetadataInfo::IptcCoreContactInfoPhone
           << MetadataInfo::IptcCoreContactInfoWebUrl;

    QVariantList metadataInfos = getMetadataFields(fields);

    IptcCoreContactInfo info;

    if (metadataInfos.size() == 8)
    {
        info.city          = metadataInfos.at(0).toString();
        info.country       = metadataInfos.at(1).toString();
        info.address       = metadataInfos.at(2).toString();
        info.postalCode    = metadataInfos.at(3).toString();
        info.provinceState = metadataInfos.at(4).toString();
        info.email         = metadataInfos.at(5).toString();
        info.phone         = metadataInfos.at(6).toString();
        info.webUrl        = metadataInfos.at(7).toString();
    }

    return info;
}

bool DMetadata::setCreatorContactInfo(const IptcCoreContactInfo& info) const
{
    if (!supportXmp())
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", info.city))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", info.country))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr", info.address))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode", info.postalCode))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion", info.provinceState))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork", info.email))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork", info.phone))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork", info.webUrl))
    {
        return false;
    }

    return true;
}

IptcCoreLocationInfo DMetadata::getIptcCoreLocation() const
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreCountry
           << MetadataInfo::IptcCoreCountryCode
           << MetadataInfo::IptcCoreCity
           << MetadataInfo::IptcCoreLocation
           << MetadataInfo::IptcCoreProvinceState;

    QVariantList metadataInfos = getMetadataFields(fields);

    IptcCoreLocationInfo location;

    if (fields.size() == 5)
    {
        location.country       = metadataInfos.at(0).toString();
        location.countryCode   = metadataInfos.at(1).toString();
        location.city          = metadataInfos.at(2).toString();
        location.location      = metadataInfos.at(3).toString();
        location.provinceState = metadataInfos.at(4).toString();
    }

    return location;
}

bool DMetadata::setIptcCoreLocation(const IptcCoreLocationInfo& location) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.photoshop.Country", location.country))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.CountryCode", location.countryCode))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.City", location.city))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.Location", location.location))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.State", location.provinceState))
        {
            return false;
        }
    }

    if (!setIptcTag(location.country,       64,  "Country",        "Iptc.Application2.CountryName"))
    {
        return false;
    }

    if (!setIptcTag(location.countryCode,    3,  "Country Code",   "Iptc.Application2.CountryCode"))
    {
        return false;
    }

    if (!setIptcTag(location.city,          32,  "City",           "Iptc.Application2.City"))
    {
        return false;
    }

    if (!setIptcTag(location.location,      32,  "SubLocation",    "Iptc.Application2.SubLocation"))
    {
        return false;
    }

    if (!setIptcTag(location.provinceState, 32,  "Province/State", "Iptc.Application2.ProvinceState"))
    {
        return false;
    }

    return true;
}

QStringList DMetadata::getIptcCoreSubjects() const
{
    QStringList list = getXmpSubjects();

    if (!list.isEmpty())
    {
        return list;
    }

    return getIptcSubjects();
}

QString DMetadata::getLensDescription() const
{
    QString     lens;
    QStringList lensExifTags;

    // In first, try to get Lens information from makernotes.

    lensExifTags.append(QLatin1String("Exif.CanonCs.LensType"));      // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.CanonCs.Lens"));          // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Canon.0x0095"));          // Alternative Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd1.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd2.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd3.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Minolta.LensID"));        // Minolta Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.LensModel"));       // Sony Cameras Makernote (and others?).
    lensExifTags.append(QLatin1String("Exif.Sony1.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sony2.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.SonyMinolta.LensID"));    // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Pentax.LensType"));       // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.PentaxDng.LensType"));    // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0051"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0310"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sigma.LensRange"));       // Sigma Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Samsung2.LensType"));     // Samsung Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.0xFDEA"));          // Non-standard Exif tag set by Camera Raw.
    lensExifTags.append(QLatin1String("Exif.OlympusEq.LensModel"));   // Olympus Cameras Makernote.

    // Olympus Cameras Makernote. FIXME is this necessary? exiv2 returns complete name, which doesn't match with lensfun information, see bug #311295
    //lensExifTags.append("Exif.OlympusEq.LensType");

    // TODO : add Fuji camera Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::const_iterator it = lensExifTags.constBegin(); it != lensExifTags.constEnd(); ++it)
    {
        lens = getExifTagString((*it).toLatin1().constData());

        if ( !lens.isEmpty() &&
             !(lens.startsWith(QLatin1Char('(')) &&
               lens.endsWith(QLatin1Char(')'))
              )
           )   // To prevent undecoded tag values from Exiv2 as "(65535)".
        {
            return lens;
        }
    }

    // -------------------------------------------------------------------
    // Try to get Lens Data information from XMP.
    // XMP aux tags.
    lens = getXmpTagString("Xmp.aux.Lens");

    if (lens.isEmpty())
    {
        // XMP M$ tags (Lens Maker + Lens Model).
        lens = getXmpTagString("Xmp.MicrosoftPhoto.LensManufacturer");

        if (!lens.isEmpty())
        {
            lens.append(QLatin1String(" "));
        }

        lens.append(getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
    }

    return lens;
}

IccProfile DMetadata::getIccProfile() const
{
    // Check if Exif data contains an ICC color profile.
    QByteArray data = getExifTagData("Exif.Image.InterColorProfile");

    if (!data.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Found an ICC profile in Exif metadata";
        return data;
    }

    // Else check the Exif color-space tag and use default profiles that we ship
    switch (getImageColorWorkSpace())
    {
        case DMetadata::WORKSPACE_SRGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is sRGB. Using default sRGB ICC profile.";
            return IccProfile::sRGB();
        }

        case DMetadata::WORKSPACE_ADOBERGB:
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif color-space tag is AdobeRGB. Using default AdobeRGB ICC profile.";
            return IccProfile::adobeRGB();
        }

        default:
            break;
    }

    return IccProfile();
}

bool DMetadata::setIccProfile(const IccProfile& profile)
{
    if (profile.isNull())
    {
        removeExifTag("Exif.Image.InterColorProfile");
    }
    else
    {
        QByteArray data = IccProfile(profile).data();

        if (!setExifTagData("Exif.Image.InterColorProfile", data))
        {
            return false;
        }
    }

    removeExifColorSpace();

    return true;
}

bool DMetadata::setIptcTag(const QString& text, int maxLength,
                           const char* const debugLabel, const char* const tagKey)  const
{
    QString truncatedText = text;
    truncatedText.truncate(maxLength);
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> " << debugLabel << ": " << truncatedText;
    return setIptcTagString(tagKey, truncatedText);    // returns false if failed
}

inline QVariant DMetadata::fromExifOrXmp(const char* const exifTagName, const char* const xmpTagName) const
{
    QVariant var;

    if (exifTagName)
    {
        var = getExifTagVariant(exifTagName, false);

        if (!var.isNull())
        {
            return var;
        }
    }

    if (xmpTagName)
    {
        var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

    return var;
}

inline QVariant DMetadata::fromIptcOrXmp(const char* const iptcTagName, const char* const xmpTagName) const
{
    if (iptcTagName)
    {
        QString iptcValue = getIptcTagString(iptcTagName);

        if (!iptcValue.isNull())
        {
            return iptcValue;
        }
    }

    if (xmpTagName)
    {
        QVariant var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

    return QVariant(QVariant::String);
}

inline QVariant DMetadata::fromIptcEmulateList(const char* const iptcTagName) const
{
    return toStringListVariant(getIptcTagsStringList(iptcTagName));
}

inline QVariant DMetadata::fromXmpList(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::StringList);
    }

    return var;
}

inline QVariant DMetadata::fromIptcEmulateLangAlt(const char* const iptcTagName) const
{
    QString str = getIptcTagString(iptcTagName);

    if (str.isNull())
    {
        return QVariant(QVariant::Map);
    }

    QMap<QString, QVariant> map;
    map[QLatin1String("x-default")] = str;
    return map;
}

inline QVariant DMetadata::fromXmpLangAlt(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::Map);
    }

    return var;
}

inline QVariant DMetadata::toStringListVariant(const QStringList& list) const
{
    if (list.isEmpty())
    {
        return QVariant(QVariant::StringList);
    }

    return list;
}

QVariant DMetadata::getMetadataField(MetadataInfo::Field field) const
{
    switch (field)
    {
        case MetadataInfo::Comment:
            return getImageComments()[QLatin1String("x-default")].caption;
        case MetadataInfo::CommentJfif:
            return getCommentsDecoded();
        case MetadataInfo::CommentExif:
            return getExifComment();
        case MetadataInfo::CommentIptc:
            return fromIptcOrXmp("Iptc.Application2.Caption", 0);

        case MetadataInfo::Description:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.description");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.ImageDescription");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Caption");
        }
        case MetadataInfo::Headline:
            return fromIptcOrXmp("Iptc.Application2.Headline", "Xmp.photoshop.Headline");
        case MetadataInfo::Title:
        {
            QString str = getImageTitles()[QLatin1String("x-default")].caption;

            if (str.isEmpty())
            {
                return QVariant(QVariant::Map);
            }

            QMap<QString, QVariant> map;
            map[QLatin1String("x-default")] = str;
            return map;
        }
        case MetadataInfo::DescriptionWriter:
            return fromIptcOrXmp("Iptc.Application2.Writer", "Xmp.photoshop.CaptionWriter");

        case MetadataInfo::Keywords:
        {
            QStringList list;
            getImageTagsPath(list);
            return toStringListVariant(list);
        }

        case MetadataInfo::Faces:
        {
            QMultiMap<QString,QVariant> faceMap;
            getImageFacesMap(faceMap);
            QVariant var(faceMap);
            return var;
        }

        case MetadataInfo::Rating:
            return getImageRating();
        case MetadataInfo::CreationDate:
            return getImageDateTime();
        case MetadataInfo::DigitizationDate:
            return getDigitizationDateTime(true);
        case MetadataInfo::Orientation:
            return (int)getImageOrientation();

        case MetadataInfo::Make:
        {
            QVariant var = fromExifOrXmp("Exif.Image.Make", "Xmp.tiff.Make");
            return QVariant(var.toString().trimmed());
        }
        case MetadataInfo::Model:
        {
            QVariant var = fromExifOrXmp("Exif.Image.Model", "Xmp.tiff.Model");
            return QVariant(var.toString().trimmed());
        }
        case MetadataInfo::Lens:
            return getLensDescription();
        case MetadataInfo::Aperture:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.FNumber", "Xmp.exif.FNumber");

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ApertureValue", "Xmp.exif.ApertureValue");

                if (!var.isNull())
                {
                    var = apexApertureToFNumber(var.toDouble());
                }
            }

            return var;
        }
        case MetadataInfo::FocalLength:
            return fromExifOrXmp("Exif.Photo.FocalLength", "Xmp.exif.FocalLength");
        case MetadataInfo::FocalLengthIn35mm:
            return fromExifOrXmp("Exif.Photo.FocalLengthIn35mmFilm", "Xmp.exif.FocalLengthIn35mmFilm");
        case MetadataInfo::ExposureTime:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.ExposureTime", "Xmp.exif.ExposureTime");

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ShutterSpeedValue", "Xmp.exif.ShutterSpeedValue");

                if (!var.isNull())
                {
                    var = apexShutterSpeedToExposureTime(var.toDouble());
                }
            }

            return var;
        }
        case MetadataInfo::ExposureProgram:
            return fromExifOrXmp("Exif.Photo.ExposureProgram", "Xmp.exif.ExposureProgram");
        case MetadataInfo::ExposureMode:
            return fromExifOrXmp("Exif.Photo.ExposureMode", "Xmp.exif.ExposureMode");
        case MetadataInfo::Sensitivity:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.ISOSpeedRatings", "Xmp.exif.ISOSpeedRatings");
            //if (var.isNull())
            // TODO: has this ISO format??? We must convert to the format of ISOSpeedRatings!
            //  var = fromExifOrXmp("Exif.Photo.ExposureIndex", "Xmp.exif.ExposureIndex");
            return var;
        }
        case MetadataInfo::FlashMode:
            return fromExifOrXmp("Exif.Photo.Flash", "Xmp.exif.Flash");
        case MetadataInfo::WhiteBalance:
            return fromExifOrXmp("Exif.Photo.WhiteBalance", "Xmp.exif.WhiteBalance");
        case MetadataInfo::MeteringMode:
            return fromExifOrXmp("Exif.Photo.MeteringMode", "Xmp.exif.MeteringMode");
        case MetadataInfo::SubjectDistance:
            return fromExifOrXmp("Exif.Photo.SubjectDistance", "Xmp.exif.SubjectDistance");
        case MetadataInfo::SubjectDistanceCategory:
            return fromExifOrXmp("Exif.Photo.SubjectDistanceRange", "Xmp.exif.SubjectDistanceRange");
        case MetadataInfo::WhiteBalanceColorTemperature:
            //TODO: ??
            return QVariant(QVariant::Int);

        case MetadataInfo::Longitude:
            return getGPSLongitudeString();
        case MetadataInfo::LongitudeNumber:
        {
            double longitude;

            if (getGPSLongitudeNumber(&longitude))
            {
                return longitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::Latitude:
            return getGPSLatitudeString();
        case MetadataInfo::LatitudeNumber:
        {
            double latitude;

            if (getGPSLatitudeNumber(&latitude))
            {
                return latitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::Altitude:
        {
            double altitude;

            if (getGPSAltitude(&altitude))
            {
                return altitude;
            }
            else
            {
                return QVariant(QVariant::Double);
            }
        }
        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
            // TODO or unsupported?
            return QVariant(QVariant::Double);
        case MetadataInfo::PositionDescription:
            // TODO or unsupported?
            return QVariant(QVariant::String);

        case MetadataInfo::IptcCoreCopyrightNotice:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.rights");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.Copyright");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Copyright");
        }
        case MetadataInfo::IptcCoreCreator:
        {
            QVariant var = fromXmpList("Xmp.dc.creator");

            if (!var.isNull())
            {
                return var;
            }

            QString artist = getXmpTagString("Xmp.tiff.Artist");

            if (!artist.isNull())
            {
                QStringList list;
                list << artist;
                return list;
            }

            return fromIptcEmulateList("Iptc.Application2.Byline");
        }
        case MetadataInfo::IptcCoreProvider:
            return fromIptcOrXmp("Iptc.Application2.Credit", "Xmp.photoshop.Credit");
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return fromXmpLangAlt("Xmp.xmpRights.UsageTerms");
        case MetadataInfo::IptcCoreSource:
            return fromIptcOrXmp("Iptc.Application2.Source", "Xmp.photoshop.Source");

        case MetadataInfo::IptcCoreCreatorJobTitle:
            return fromIptcOrXmp("Iptc.Application2.BylineTitle", "Xmp.photoshop.AuthorsPosition");
        case MetadataInfo::IptcCoreInstructions:
            return fromIptcOrXmp("Iptc.Application2.SpecialInstructions", "Xmp.photoshop.Instructions");

        case MetadataInfo::IptcCoreLocationInfo:
        {
            IptcCoreLocationInfo location = getIptcCoreLocation();

            if (location.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(location);
        }
        case MetadataInfo::IptcCoreCountryCode:
            return fromIptcOrXmp("Iptc.Application2.CountryCode", "Xmp.iptc.CountryCode");
        case MetadataInfo::IptcCoreCountry:
            return fromIptcOrXmp("Iptc.Application2.CountryName", "Xmp.photoshop.Country");
        case MetadataInfo::IptcCoreCity:
            return fromIptcOrXmp("Iptc.Application2.City", "Xmp.photoshop.City");
        case MetadataInfo::IptcCoreLocation:
            return fromIptcOrXmp("Iptc.Application2.SubLocation", "Xmp.iptc.Location");
        case MetadataInfo::IptcCoreProvinceState:
            return fromIptcOrXmp("Iptc.Application2.ProvinceState", "Xmp.photoshop.State");

        case MetadataInfo::IptcCoreIntellectualGenre:
            return fromIptcOrXmp("Iptc.Application2.ObjectAttribute", "Xmp.iptc.IntellectualGenre");
        case MetadataInfo::IptcCoreJobID:
            return fromIptcOrXmp("Iptc.Application2.TransmissionReference", "Xmp.photoshop.TransmissionReference");
        case MetadataInfo::IptcCoreScene:
            return fromXmpList("Xmp.iptc.Scene");
        case MetadataInfo::IptcCoreSubjectCode:
            return toStringListVariant(getIptcCoreSubjects());

        case MetadataInfo::IptcCoreContactInfo:
        {
            IptcCoreContactInfo info = getCreatorContactInfo();

            if (info.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(info);
        }
        case MetadataInfo::IptcCoreContactInfoCity:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity");
        case MetadataInfo::IptcCoreContactInfoCountry:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry");
        case MetadataInfo::IptcCoreContactInfoAddress:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr");
        case MetadataInfo::IptcCoreContactInfoPostalCode:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode");
        case MetadataInfo::IptcCoreContactInfoProvinceState:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion");
        case MetadataInfo::IptcCoreContactInfoEmail:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork");
        case MetadataInfo::IptcCoreContactInfoPhone:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork");
        case MetadataInfo::IptcCoreContactInfoWebUrl:
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork");

        case MetadataInfo::AspectRatio:
        {
            long num             = 0;
            long den             = 1;

            // NOTE: there is a bug in Exiv2 xmp::video tag definition as "Rational" value is defined as "Ratio"...
            //QList<QVariant> list = getXmpTagVariant("Xmp.video.AspectRatio").toList();

            QString ar       = getXmpTagString("Xmp.video.AspectRatio");
            QStringList list = ar.split(QLatin1Char('/'));

            if (list.size() >= 1)
                num = list[0].toInt();

            if (list.size() >= 2)
                den = list[1].toInt();

            return QString::number((double)num / (double)den);
        }
        case MetadataInfo::AudioBitRate:
            return fromXmpLangAlt("Xmp.audio.SampleRate");
        case MetadataInfo::AudioChannelType:
            return fromXmpLangAlt("Xmp.audio.ChannelType");
        case MetadataInfo::AudioCodec:
            return fromXmpLangAlt("Xmp.audio.Codec");
        case MetadataInfo::Duration:
            return fromXmpLangAlt("Xmp.video.duration"); // duration is in ms
        case MetadataInfo::FrameRate:
            return fromXmpLangAlt("Xmp.video.FrameRate");
        case MetadataInfo::VideoCodec:
            return fromXmpLangAlt("Xmp.video.Codec");
        case MetadataInfo::VideoBitDepth:
            return fromXmpLangAlt("Xmp.video.BitDepth");
        case MetadataInfo::VideoHeight:
            return fromXmpLangAlt("Xmp.video.Height");
        case MetadataInfo::VideoWidth:
            return fromXmpLangAlt("Xmp.video.Width");
        case MetadataInfo::VideoColorSpace:
        {
            QString cs = getXmpTagString("Xmp.video.ColorSpace");
            
            if (cs == QLatin1String("sRGB"))
                return QString::number(VIDEOCOLORMODEL_SRGB);
            else if (cs == QLatin1String("CCIR-601"))
                return QString::number(VIDEOCOLORMODEL_BT601);
            else if (cs == QLatin1String("CCIR-709"))
                return QString::number(VIDEOCOLORMODEL_BT709);
            else if (cs == QLatin1String("Other"))
                return QString::number(VIDEOCOLORMODEL_OTHER);
            else
                return QVariant(QVariant::Int);
        }
        default:
            return QVariant();
    }
}

QVariantList DMetadata::getMetadataFields(const MetadataFields& fields) const
{
    QVariantList list;

    foreach(MetadataInfo::Field field, fields) // krazy:exclude=foreach
    {
        list << getMetadataField(field);
    }

    return list;
}

QString DMetadata::valueToString(const QVariant& value, MetadataInfo::Field field)
{
    MetaEngine exiv2Iface;

    switch (field)
    {
        case MetadataInfo::Rating:
            return value.toString();
        case MetadataInfo::CreationDate:
        case MetadataInfo::DigitizationDate:
            return value.toDateTime().toString(Qt::LocaleDate);
        case MetadataInfo::Orientation:
        {
            switch (value.toInt())
            {
                    // Example why the English text differs from the enum names: ORIENTATION_ROT_90.
                    // Rotation by 90 degrees is right (clockwise) rotation.
                    // But: The enum names describe what needs to be done to get the image right again.
                    // And an image that needs to be rotated 90 degrees is currently rotated 270 degrees = left.

                case ORIENTATION_UNSPECIFIED:
                    return i18n("Unspecified");
                case ORIENTATION_NORMAL:
                    return i18nc("Rotation of an unrotated image", "Normal");
                case ORIENTATION_HFLIP:
                    return i18n("Flipped Horizontally");
                case ORIENTATION_ROT_180:
                    return i18n("Rotated by 180 Degrees");
                case ORIENTATION_VFLIP:
                    return i18n("Flipped Vertically");
                case ORIENTATION_ROT_90_HFLIP:
                    return i18n("Flipped Horizontally and Rotated Left");
                case ORIENTATION_ROT_90:
                    return i18n("Rotated Left");
                case ORIENTATION_ROT_90_VFLIP:
                    return i18n("Flipped Vertically and Rotated Left");
                case ORIENTATION_ROT_270:
                    return i18n("Rotated Right");
                default:
                    return i18n("Unknown");
            }
            break;
        }
        case MetadataInfo::Make:
            return exiv2Iface.createExifUserStringFromValue("Exif.Image.Make", value);
        case MetadataInfo::Model:
            return exiv2Iface.createExifUserStringFromValue("Exif.Image.Model", value);
        case MetadataInfo::Lens:
            // heterogeneous source, non-standardized string
            return value.toString();
        case MetadataInfo::Aperture:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FNumber", value);
        case MetadataInfo::FocalLength:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FocalLength", value);
        case MetadataInfo::FocalLengthIn35mm:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.FocalLengthIn35mmFilm", value);
        case MetadataInfo::ExposureTime:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureTime", value);
        case MetadataInfo::ExposureProgram:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureProgram", value);
        case MetadataInfo::ExposureMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ExposureMode", value);
        case MetadataInfo::Sensitivity:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.ISOSpeedRatings", value);
        case MetadataInfo::FlashMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.Flash", value);
        case MetadataInfo::WhiteBalance:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.WhiteBalance", value);
        case MetadataInfo::MeteringMode:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.MeteringMode", value);
        case MetadataInfo::SubjectDistance:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.SubjectDistance", value);
        case MetadataInfo::SubjectDistanceCategory:
            return exiv2Iface.createExifUserStringFromValue("Exif.Photo.SubjectDistanceRange", value);
        case MetadataInfo::WhiteBalanceColorTemperature:
            return i18nc("Temperature in Kelvin", "%1 K", value.toInt());

        case MetadataInfo::AspectRatio:
        case MetadataInfo::AudioBitRate:
        case MetadataInfo::AudioChannelType:
        case MetadataInfo::AudioCodec:
        case MetadataInfo::Duration:
        case MetadataInfo::FrameRate:
        case MetadataInfo::VideoCodec:
            return value.toString();

        case MetadataInfo::Longitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W')) ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LongitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W')) ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Latitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N')) ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LatitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N')) ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Altitude:
        {
            QString meters = QString::fromLatin1("%L1").arg(value.toDouble(), 0, 'f', 2);
            // xgettext: no-c-format
            return i18nc("Height in meters", "%L1m", meters); // krazy:exclude=i18ncheckarg
        }

        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
            //TODO
            return value.toString();

        case MetadataInfo::PositionDescription:
            return value.toString();

            // Lang Alt
        case MetadataInfo::IptcCoreCopyrightNotice:
        case MetadataInfo::IptcCoreRightsUsageTerms:
        case MetadataInfo::Description:
        case MetadataInfo::Title:
        {
            QMap<QString, QVariant> map = value.toMap();

            // the most common cases
            if (map.isEmpty())
            {
                return QString();
            }
            else if (map.size() == 1)
            {
                return map.begin().value().toString();
            }

            // Try "en-us"
            QString spec = QLocale().name().toLower().replace(QLatin1Char('_'), QLatin1Char('-'));

            if (map.contains(spec))
            {
                return map[spec].toString();
            }

            // Try "en-"
            QStringList keys    = map.keys();
            QString spec2       = QLocale().name().toLower();
            QRegExp exp(spec2.left(spec2.indexOf(QLatin1Char('_'))) + QLatin1Char('-'));
            QStringList matches = keys.filter(exp);

            if (!matches.isEmpty())
            {
                return map[matches.first()].toString();
            }

            // return default
            if (map.contains(QLatin1String("x-default")))
            {
                return map[QLatin1String("x-default")].toString();
            }

            // return first entry
            return map.begin().value().toString();
        }

        // List
        case MetadataInfo::IptcCoreCreator:
        case MetadataInfo::IptcCoreScene:
        case MetadataInfo::IptcCoreSubjectCode:
            return value.toStringList().join(QLatin1String(" "));

            // Text
        case MetadataInfo::Comment:
        case MetadataInfo::CommentJfif:
        case MetadataInfo::CommentExif:
        case MetadataInfo::CommentIptc:
        case MetadataInfo::Headline:
        case MetadataInfo::DescriptionWriter:
        case MetadataInfo::IptcCoreProvider:
        case MetadataInfo::IptcCoreSource:
        case MetadataInfo::IptcCoreCreatorJobTitle:
        case MetadataInfo::IptcCoreInstructions:
        case MetadataInfo::IptcCoreCountryCode:
        case MetadataInfo::IptcCoreCountry:
        case MetadataInfo::IptcCoreCity:
        case MetadataInfo::IptcCoreLocation:
        case MetadataInfo::IptcCoreProvinceState:
        case MetadataInfo::IptcCoreIntellectualGenre:
        case MetadataInfo::IptcCoreJobID:
            return value.toString();

        default:
            break;
    }

    return QString();
}

QStringList DMetadata::valuesToString(const QVariantList& values, const MetadataFields& fields)
{
    int size = values.size();
    Q_ASSERT(size == values.size());

    QStringList list;
    for (int i = 0; i < size; ++i)
    {
        list << valueToString(values.at(i), fields.at(i));
    }

    return list;
}

QMap<int, QString> DMetadata::possibleValuesForEnumField(MetadataInfo::Field field)
{
    QMap<int, QString> map;
    int min, max;

    switch (field)
    {
        case MetadataInfo::Orientation:                      /// Int, enum from libMetaEngine
            min = ORIENTATION_UNSPECIFIED;
            max = ORIENTATION_ROT_270;
            break;
        case MetadataInfo::ExposureProgram:                  /// Int, enum from Exif
            min = 0;
            max = 8;
            break;
        case MetadataInfo::ExposureMode:                     /// Int, enum from Exif
            min = 0;
            max = 2;
            break;
        case MetadataInfo::WhiteBalance:                     /// Int, enum from Exif
            min = 0;
            max = 1;
            break;
        case MetadataInfo::MeteringMode:                     /// Int, enum from Exif
            min = 0;
            max = 6;
            map[255] = valueToString(255, field);
            break;
        case MetadataInfo::SubjectDistanceCategory:          /// int, enum from Exif
            min = 0;
            max = 3;
            break;
        case MetadataInfo::FlashMode:                        /// Int, bit mask from Exif
            // This one is a bit special.
            // We return a bit mask for binary AND searching.
            map[0x1] = i18n("Flash has been fired");
            map[0x40] = i18n("Flash with red-eye reduction mode");
            //more: TODO?
            return map;
        default:
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Unsupported field " << field << " in DMetadata::possibleValuesForEnumField";
            return map;
    }

    for (int i = min; i <= max; ++i)
    {
        map[i] = valueToString(i, field);
    }

    return map;
}

double DMetadata::apexApertureToFNumber(double aperture)
{
    // convert from APEX. See Exif spec, Annex C.
    if (aperture == 0.0)
    {
        return 1;
    }
    else if (aperture == 1.0)
    {
        return 1.4;
    }
    else if (aperture == 2.0)
    {
        return 2;
    }
    else if (aperture == 3.0)
    {
        return 2.8;
    }
    else if (aperture == 4.0)
    {
        return 4;
    }
    else if (aperture == 5.0)
    {
        return 5.6;
    }
    else if (aperture == 6.0)
    {
        return 8;
    }
    else if (aperture == 7.0)
    {
        return 11;
    }
    else if (aperture == 8.0)
    {
        return 16;
    }
    else if (aperture == 9.0)
    {
        return 22;
    }
    else if (aperture == 10.0)
    {
        return 32;
    }

    return exp(log(2) * aperture / 2.0);
}

double DMetadata::apexShutterSpeedToExposureTime(double shutterSpeed)
{
    // convert from APEX. See Exif spec, Annex C.
    if (shutterSpeed == -5.0)
    {
        return 30;
    }
    else if (shutterSpeed == -4.0)
    {
        return 15;
    }
    else if (shutterSpeed == -3.0)
    {
        return 8;
    }
    else if (shutterSpeed == -2.0)
    {
        return 4;
    }
    else if (shutterSpeed == -1.0)
    {
        return 2;
    }
    else if (shutterSpeed == 0.0)
    {
        return 1;
    }
    else if (shutterSpeed == 1.0)
    {
        return 0.5;
    }
    else if (shutterSpeed == 2.0)
    {
        return 0.25;
    }
    else if (shutterSpeed == 3.0)
    {
        return 0.125;
    }
    else if (shutterSpeed == 4.0)
    {
        return 1.0 / 15.0;
    }
    else if (shutterSpeed == 5.0)
    {
        return 1.0 / 30.0;
    }
    else if (shutterSpeed == 6.0)
    {
        return 1.0 / 60.0;
    }
    else if (shutterSpeed == 7.0)
    {
        return 0.008;    // 1/125
    }
    else if (shutterSpeed == 8.0)
    {
        return 0.004;    // 1/250
    }
    else if (shutterSpeed == 9.0)
    {
        return 0.002;    // 1/500
    }
    else if (shutterSpeed == 10.0)
    {
        return 0.001;    // 1/1000
    }
    else if (shutterSpeed == 11.0)
    {
        return 0.0005;    // 1/2000
    }
    // additions by me
    else if (shutterSpeed == 12.0)
    {
        return 0.00025;    // 1/4000
    }
    else if (shutterSpeed == 13.0)
    {
        return 0.000125;    // 1/8000
    }

    return exp( - log(2) * shutterSpeed);
}

MetaEngine::AltLangMap DMetadata::toAltLangMap(const QVariant& var)
{
    MetaEngine::AltLangMap map;

    if (var.isNull())
    {
        return map;
    }

    switch (var.type())
    {
        case QVariant::String:
            map.insert(QLatin1String("x-default"), var.toString());
            break;
        case QVariant::Map:
        {
            QMap<QString, QVariant> varMap = var.toMap();

            for (QMap<QString, QVariant>::const_iterator it = varMap.constBegin(); it != varMap.constEnd(); ++it)
            {
                map.insert(it.key(), it.value().toString());
            }

            break;
        }
        default:
            break;
    }

    return map;
}

bool DMetadata::addToXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToAdd) const
{
    //#ifdef _XMP_SUPPORT_

    QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries = entriesToAdd;

    // Create a list of keywords including old one which already exists.
    for (QStringList::const_iterator it = oldEntries.constBegin(); it != oldEntries.constEnd(); ++it )
    {
        if (!newEntries.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    //#endif // _XMP_SUPPORT_

    return false;
}

bool DMetadata::removeFromXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToRemove) const
{
    //#ifdef _XMP_SUPPORT_

    QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries;

    // Create a list of current keywords except those that shall be removed
    for (QStringList::const_iterator it = currentEntries.constBegin(); it != currentEntries.constEnd(); ++it )
    {
        if (!entriesToRemove.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    //#endif // _XMP_SUPPORT_

    return false;
}

QStringList DMetadata::getXmpKeywords() const
{
    return (getXmpTagStringBag("Xmp.dc.subject", false));
}

bool DMetadata::setXmpKeywords(const QStringList& newKeywords) const
{
    return setXmpTagStringBag("Xmp.dc.subject", newKeywords);
}

bool DMetadata::removeXmpKeywords(const QStringList& keywordsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}

QStringList DMetadata::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool DMetadata::setXmpSubCategories(const QStringList& newSubCategories) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}

bool DMetadata::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}

QStringList DMetadata::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool DMetadata::setXmpSubjects(const QStringList& newSubjects) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}

bool DMetadata::removeXmpSubjects(const QStringList& subjectsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}

bool DMetadata::removeExifColorSpace() const
{
    bool ret =  true;
    ret     &= removeExifTag("Exif.Photo.ColorSpace");
    ret     &= removeXmpTag("Xmp.exif.ColorSpace");

    return ret;
}

QString DMetadata::getExifTagStringFromTagsList(const QStringList& tagsList) const
{
    QString val;

    foreach(const QString& tag, tagsList)
    {
        val = getExifTagString(tag.toLatin1().constData());

        if (!val.isEmpty())
            return val;
    }

    return QString();
}

bool DMetadata::removeExifTags(const QStringList& tagFilters)
{
    MetaDataMap m = getExifTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeExifTag(it.key().toLatin1().constData());
    }

    return true;
}

bool DMetadata::removeIptcTags(const QStringList& tagFilters)
{
    MetaDataMap m = getIptcTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeIptcTag(it.key().toLatin1().constData());
    }

    return true;
}

bool DMetadata::removeXmpTags(const QStringList& tagFilters)
{
    MetaDataMap m = getXmpTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeXmpTag(it.key().toLatin1().constData());
    }

    return true;
}

}  // namespace Digikam
