/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QDomDocument>
#include <QFile>
#include <QFileInfo>

// KDE includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/dcrawinfocontainer.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "filereadwritelock.h"
#include "metadatasettings.h"
#include "template.h"
#include "version.h"
#include "globals.h"

using namespace KDcrawIface;

namespace Digikam
{

DMetadata::DMetadata()
    : KExiv2()
{
    registerMetadataSettings();
}

DMetadata::DMetadata(const QString& filePath)
    : KExiv2()
{
    registerMetadataSettings();
    load(filePath);
}

#if KEXIV2_VERSION >= 0x010000
DMetadata::DMetadata(const KExiv2Data& data)
    : KExiv2(data)
{
    registerMetadataSettings();
}
#endif

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

#if KEXIV2_VERSION >= 0x000600
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
#endif
}

bool DMetadata::load(const QString& filePath) const
{
    // In first, we trying to get metadata using Exiv2,
    // else we will use dcraw to extract minimal information.

    FileReadLocker lock(filePath);

    if (!KExiv2::load(filePath))
    {
        if (!loadUsingDcraw(filePath))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::save(const QString& filePath) const
{
    FileWriteLocker lock(filePath);
    return KExiv2::save(filePath);
}

bool DMetadata::applyChanges() const
{
    FileWriteLocker lock(getFilePath());
    return KExiv2::applyChanges();
}

bool DMetadata::loadUsingDcraw(const QString& filePath) const
{
    DcrawInfoContainer identify;

    if (KDcraw::rawFileIdentify(identify, filePath))
    {
        long int num=1, den=1;

        if (!identify.model.isNull())
        {
            setExifTagString("Exif.Image.Model", identify.model.toLatin1(), false);
        }

        if (!identify.make.isNull())
        {
            setExifTagString("Exif.Image.Make", identify.make.toLatin1(), false);
        }

        if (!identify.owner.isNull())
        {
            setExifTagString("Exif.Image.Artist", identify.owner.toLatin1(), false);
        }

        if (identify.sensitivity != -1)
        {
            setExifTagLong("Exif.Photo.ISOSpeedRatings", lroundf(identify.sensitivity), false);
        }

        if (identify.dateTime.isValid())
        {
            setImageDateTime(identify.dateTime, false, false);
        }

        if (identify.exposureTime != -1.0)
        {
            convertToRationalSmallDenominator(identify.exposureTime, &num, &den);
            setExifTagRational("Exif.Photo.ExposureTime", num, den, false);
        }

        if (identify.aperture != -1.0)
        {
            convertToRational(identify.aperture, &num, &den, 8);
            setExifTagRational("Exif.Photo.ApertureValue", num, den, false);
        }

        if (identify.focalLength != -1.0)
        {
            convertToRational(identify.focalLength, &num, &den, 8);
            setExifTagRational("Exif.Photo.FocalLength", num, den, false);
        }

        if (identify.imageSize.isValid())
        {
            setImageDimensions(identify.imageSize, false);
        }

        // A RAW image is always uncalibrated. */
        setImageColorWorkSpace(WORKSPACE_UNCALIBRATED, false);

        return true;
    }

    return false;
}

bool DMetadata::setProgramId(bool on) const
{
    if (on)
    {
        QString version(digiKamVersion());
        QString software("digiKam");
        return setImageProgramId(software, version);
    }

    return true;
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
            int _ms = (int)(QString("0.%1").arg(sub).toFloat(&ok) * 1000.0);

            if (ok)
            {
                ms = _ms;
                kDebug() << "msec timestamp: " << ms;
            }
        }
    }

    return ok;
}

CaptionsMap DMetadata::getImageComments() const
{
    if (getFilePath().isEmpty())
    {
        return CaptionsMap();
    }

    CaptionsMap        captionsMap;
    KExiv2::AltLangMap authorsMap;
    KExiv2::AltLangMap datesMap;
    KExiv2::AltLangMap commentsMap;
    QString            commonAuthor;

    // In first try to get captions properties from digiKam XMP namespace

    if (supportXmp())
    {
        authorsMap = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames",    false);
        datesMap   = getXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", false);
    }

    // Get author name from IPTC DescriptionWriter. Private namespace above gets precedence.
    QVariant descriptionWriter = getMetadataField(MetadataInfo::DescriptionWriter);

    if (!descriptionWriter.isNull())
    {
        commonAuthor = descriptionWriter.toString();
    }

    // In first, we check XMP alternative language tags to create map of values.

    if (hasXmp())
    {
        if (authorsMap.isEmpty() && commonAuthor.isEmpty())
        {
            QString xmpAuthors = getXmpTagString("Xmp.acdsee.author", false);

            if (!xmpAuthors.isEmpty())
            {
                authorsMap.insert(QString("x-default"), xmpAuthors);
            }
        }

        commentsMap = getXmpTagStringListLangAlt("Xmp.dc.description", false);

        if (!commentsMap.isEmpty())
        {
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        QString xmpComment = getXmpTagStringLangAlt("Xmp.exif.UserComment", QString(), false);

        if (!xmpComment.isEmpty())
        {
            commentsMap.insert(QString("x-default"), xmpComment);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        xmpComment = getXmpTagStringLangAlt("Xmp.tiff.ImageDescription", QString(), false);

        if (!xmpComment.isEmpty())
        {
            commentsMap.insert(QString("x-default"), xmpComment);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }

        xmpComment = getXmpTagString("Xmp.acdsee.notes", false);

        if (!xmpComment.isEmpty())
        {
            commentsMap.insert(QString("x-default"), xmpComment);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    // Now, we trying to get image comments, outside of XMP.
    // For JPEG, string is extracted from JFIF Comments section.
    // For PNG, string is extracted from iTXt chunk.

    QString comment = getCommentsDecoded();

    if (!comment.isEmpty())
    {
        commentsMap.insert(QString("x-default"), comment);
        captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
        return captionsMap;
    }

    // We trying to get Exif comments

    if (hasExif())
    {
        QString exifComment = getExifComment();

        if (!exifComment.isEmpty())
        {
            commentsMap.insert(QString("x-default"), exifComment);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    // We trying to get IPTC comments

    if (hasIptc())
    {
        QString iptcComment = getIptcTagString("Iptc.Application2.Caption", false);

        if (!iptcComment.isEmpty() && !iptcComment.trimmed().isEmpty())
        {
            commentsMap.insert(QString("x-default"), iptcComment);
            captionsMap.setData(commentsMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageComments(const CaptionsMap& comments) const
{
    //See bug #139313: An empty string is also a valid value
    /*
    if (comments.isEmpty())
          return false;
    */

    kDebug() << getFilePath() << " ==> Comment: " << comments;

    // In first, set captions properties to digiKam XMP namespace

    if (supportXmp())
    {
        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsAuthorNames", comments.authorsList(), false))
        {
            return false;
        }

        QString defaultAuthor  = comments.value("x-default").author;

        removeXmpTag("Xmp.acdsee.author");
        if (!defaultAuthor.isNull())
        {
            if (!setXmpTagString("Xmp.acdsee.author", defaultAuthor, false))
            {
                return false;
            }
        }

        if (!setXmpTagStringListLangAlt("Xmp.digiKam.CaptionsDateTimeStamps", comments.datesList(), false))
        {
            return false;
        }
    }

    QString defaultComment = comments.value("x-default").caption;

    // In first we set image comments, outside of Exif, XMP, and IPTC.

    if (!setComments(defaultComment.toUtf8()))
    {
        return false;
    }

    // In Second we write comments into Exif.

    if (!setExifComment(defaultComment))
    {
        return false;
    }

    // In Third we write comments into XMP. Language Alternative rule is not yet used.

    if (supportXmp())
    {
        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values
        if (!setXmpTagStringListLangAlt("Xmp.dc.description", comments.toAltLangMap(), false))
        {
            return false;
        }

        // setXmpTagStringLangAlt does not remove xmp tag before adding a new value, so we do it.
        removeXmpTag("Xmp.exif.UserComment");
        if (!defaultComment.isNull())
        {
            if (!setXmpTagStringLangAlt("Xmp.exif.UserComment", defaultComment, QString(), false))
            {
                return false;
            }
        }

        removeXmpTag("Xmp.tiff.ImageDescription");
        if (!defaultComment.isNull())
        {
            if (!setXmpTagStringLangAlt("Xmp.tiff.ImageDescription", defaultComment, QString(), false))
            {
                return false;
            }
        }

        removeXmpTag("Xmp.acdsee.notes");
        if (!defaultComment.isEmpty())
        {
            if (!setXmpTagString("Xmp.acdsee.notes", defaultComment, false))
            {
                return false;
            }
        }
    }

    // In Four we write comments into IPTC.
    // Note that Caption IPTC tag is limited to 2000 char and ASCII charset.

    removeIptcTag("Iptc.Application2.Caption");

    if (!defaultComment.isNull())
    {
        defaultComment.truncate(2000);

        if (!setIptcTagString("Iptc.Application2.Caption", defaultComment))
        {
            return false;
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
    }

    return -1;
}

CaptionsMap DMetadata::getImageTitles() const
{
    if (getFilePath().isEmpty())
        return CaptionsMap();

    CaptionsMap        captionsMap;
    KExiv2::AltLangMap authorsMap;
    KExiv2::AltLangMap datesMap;
    KExiv2::AltLangMap titlesMap;
    QString            commonAuthor;

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
            titlesMap.insert(QString("x-default"), xmpTitle);
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
            titlesMap.insert(QString("x-default"), iptcTitle);
            captionsMap.setData(titlesMap, authorsMap, commonAuthor, datesMap);
            return captionsMap;
        }
    }

    return captionsMap;
}

bool DMetadata::setImageTitles(const CaptionsMap& titles) const
{
    kDebug() << getFilePath() << " ==> Title: " << titles;

    QString defaultTitle = titles[QString("x-default")].caption;

    // In First we write comments into XMP. Language Alternative rule is not yet used.

    if (supportXmp())
    {
        // NOTE : setXmpTagStringListLangAlt remove xmp tag before to add new values
        if (!setXmpTagStringListLangAlt("Xmp.dc.title", titles.toAltLangMap(), false))
        {
            return false;
        }

        removeXmpTag("Xmp.acdsee.caption");
        if (!defaultTitle.isEmpty())
        {
            if (!setXmpTagString("Xmp.acdsee.caption", defaultTitle, false))
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

int DMetadata::getImageRating() const
{
    if (getFilePath().isEmpty())
    {
        return -1;
    }

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.xmp.Rating", false);

        if (!value.isEmpty())
        {
            bool ok     = false;
            long rating = value.toLong(&ok);

            if (ok && rating >= RatingMin && rating <= RatingMax)
            {
                return rating;
            }
        }

        value = getXmpTagString("Xmp.acdsee.rating", false);

        if (!value.isEmpty())
        {
            bool ok     = false;
            long rating = value.toLong(&ok);

            if (ok && rating >= RatingMin && rating <= RatingMax)
            {
                return rating;
            }
        }

        value = getXmpTagString("Xmp.MicrosoftPhoto.Rating", false);

        if (!value.isEmpty())
        {
            bool ok            = false;
            long ratingPercent = value.toLong(&ok);

            if (ok)
            {
                // Wrapper around rating percents managed by Windows Vista.
                long rating = -1;

                switch (ratingPercent)
                {
                    case 0:
                        rating = 0;
                        break;
                    case 1:
                        rating = 1;
                        break;
                    case 25:
                        rating = 2;
                        break;
                    case 50:
                        rating = 3;
                        break;
                    case 75:
                        rating = 4;
                        break;
                    case 99:
                        rating = 5;
                        break;
                }

                if (rating != -1 && rating >= RatingMin && rating <= RatingMax)
                {
                    return rating;
                }
            }
        }
    }

    // Check Exif rating tag set by Windows Vista
    // Note : no need to check rating in percent tags (Exif.image.0x4747) here because
    // its appear always with rating tag value (Exif.image.0x4749).

    if (hasExif())
    {
        long rating = -1;

        if (getExifTagLong("Exif.Image.0x4746", rating))
        {
            if (rating >= RatingMin && rating <= RatingMax)
            {
                return rating;
            }
        }
    }

    // digiKam 0.9.x has used IPTC Urgency to store Rating.
    // This way is obsolete now since digiKam support XMP.
    // But we will let the capability to import it.
    // Iptc.Application2.Urgency <==> digiKam Rating links:
    //
    // digiKam     IPTC
    // Rating      Urgency
    //
    // 0 star  <=>  8          // Least important
    // 1 star  <=>  7
    // 1 star  <==  6
    // 2 star  <=>  5
    // 3 star  <=>  4
    // 4 star  <==  3
    // 4 star  <=>  2
    // 5 star  <=>  1          // Most important

    if (hasIptc())
    {
        QString IptcUrgency(getIptcTagData("Iptc.Application2.Urgency"));

        if (!IptcUrgency.isEmpty())
        {
            if (IptcUrgency == QString("1"))
            {
                return 5;
            }
            else if (IptcUrgency == QString("2"))
            {
                return 4;
            }
            else if (IptcUrgency == QString("3"))
            {
                return 4;
            }
            else if (IptcUrgency == QString("4"))
            {
                return 3;
            }
            else if (IptcUrgency == QString("5"))
            {
                return 2;
            }
            else if (IptcUrgency == QString("6"))
            {
                return 1;
            }
            else if (IptcUrgency == QString("7"))
            {
                return 1;
            }
            else if (IptcUrgency == QString("8"))
            {
                return 0;
            }
        }
    }

    return -1;
}

bool DMetadata::setImagePickLabel(int pickId) const
{
    if (pickId < NoPickLabel || pickId > AcceptedLabel)
    {
        kDebug() << "Pick Label value to write is out of range!";
        return false;
    }

    kDebug() << getFilePath() << " ==> Pick Label: " << pickId;

    if (!setProgramId())
    {
        return false;
    }

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
        kDebug() << "Color Label value to write is out of range!";
        return false;
    }

    kDebug() << getFilePath() << " ==> Color Label: " << colorId;

    if (!setProgramId())
    {
        return false;
    }

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
    }

    return true;
}

bool DMetadata::setImageRating(int rating) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Urgency to store Rating.
    // Now this way is obsolete, and we use standard XMP rating tag instead.

    if (rating < RatingMin || rating > RatingMax)
    {
        kDebug() << "Rating value to write is out of range!";
        return false;
    }

    kDebug() << getFilePath() << " ==> Rating: " << rating;

    if (!setProgramId())
    {
        return false;
    }

    // Set standard XMP rating tag.

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.xmp.Rating", QString::number(rating)))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.acdsee.rating", QString::number(rating), false))
        {
            return false;
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

    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.MicrosoftPhoto.Rating", QString::number(ratePercents)))
        {
            return false;
        }
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
        if (!setXmpTagString("Xmp.digiKam.ImageHistory", imageHistoryXml, false))
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
        kDebug() << "Loading image history " << value;
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
    if (hasXmp())
    {
        QString uuid = getXmpTagString("Xmp.digiKam.ImageUniqueID");

        if (!uuid.isEmpty())
        {
            return uuid;
        }

        QString exifUid = getXmpTagString("Xmp.exif.ImageUniqueId");

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
            return exifUid;
        }

        // Exif.Image.ImageID can also be a pathname, so it's not sufficiently unique

        QString dngUid = getExifTagString("Exif.Image.RawDataUniqueID");

        if (!dngUid.isEmpty())
        {
            return dngUid;
        }
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

        ISOSpeedTags << "Exif.Photo.ISOSpeedRatings";
        ISOSpeedTags << "Exif.Photo.ExposureIndex";
        ISOSpeedTags << "Exif.Image.ISOSpeedRatings";
        ISOSpeedTags << "Xmp.exif.ISOSpeedRatings";
        ISOSpeedTags << "Xmp.exif.ExposureIndex";
        ISOSpeedTags << "Exif.CanonSi.ISOSpeed";
        ISOSpeedTags << "Exif.CanonCs.ISOSpeed";
        ISOSpeedTags << "Exif.Nikon1.ISOSpeed";
        ISOSpeedTags << "Exif.Nikon2.ISOSpeed";
        ISOSpeedTags << "Exif.Nikon3.ISOSpeed";
        ISOSpeedTags << "Exif.NikonIi.ISO";
        ISOSpeedTags << "Exif.NikonIi.ISO2";
        ISOSpeedTags << "Exif.MinoltaCsNew.ISOSetting";
        ISOSpeedTags << "Exif.MinoltaCsOld.ISOSetting";
        ISOSpeedTags << "Exif.MinoltaCs5D.ISOSpeed";
        ISOSpeedTags << "Exif.MinoltaCs7D.ISOSpeed";
        ISOSpeedTags << "Exif.Sony1Cs.ISOSetting";
        ISOSpeedTags << "Exif.Sony2Cs.ISOSetting";
        ISOSpeedTags << "Exif.Sony1Cs2.ISOSetting";
        ISOSpeedTags << "Exif.Sony2Cs2.ISOSetting";
        ISOSpeedTags << "Exif.Sony1MltCsA100.ISOSetting";
        ISOSpeedTags << "Exif.Pentax.ISO";
        ISOSpeedTags << "Exif.Olympus.ISOSpeed";
        ISOSpeedTags << "Exif.Samsung2.ISO";

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
            videoInfo.aspectRatio = getXmpTagString("Xmp.video.AspectRatio");
        }

        if (videoInfo.audioBitRate.isEmpty())
        {
            videoInfo.audioBitRate = getXmpTagString("Xmp.audio.SampleRate");
        }

        if (videoInfo.audioChannelType.isEmpty())
        {
            videoInfo.audioChannelType = getXmpTagString("Xmp.audio.ChannelType");
        }

        if (videoInfo.audioCompressor.isEmpty())
        {
            videoInfo.audioCompressor = getXmpTagString("Xmp.audio.Compressor");
        }

        if (videoInfo.duration.isEmpty())
        {
            videoInfo.duration = getXmpTagString("Xmp.video.Duration");
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

bool DMetadata::getImageTagsPath(QStringList& tagsPath) const
{
    // Try to get Tags Path list from XMP in first.
    tagsPath = getXmpTagStringSeq("Xmp.digiKam.TagsList", false);
    if (!tagsPath.isEmpty())
    {
        return true;
    }

    // See bug #269418 : try to get Tags Path list from M$ Windows Live Photo Gallery.
    tagsPath = getXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", false);
    if (!tagsPath.isEmpty())
    {
        return true;
    }

    // Try to get Tags Path list from XMP in first.
    tagsPath = getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    // See bug #221460: there is another LR tag for hierarchical subjects.
    if (tagsPath.isEmpty())
    {
        tagsPath = getXmpTagStringSeq("Xmp.lr.HierarchicalSubject", false);
    }

    if (!tagsPath.isEmpty())
    {
        // See bug #197285: LightRoom use '|' as separator.
        tagsPath = tagsPath.replaceInStrings("|", "/");
        kDebug() << "Tags Path imported from LightRoom: " << tagsPath;
        return true;
    }

    // Try to get Tags Path list from Media Pro XMP first.
    tagsPath = getXmpTagStringBag("Xmp.mediapro.CatalogSets", false);

    // There is another Media Pro tag for hierarchical subjects.
    if (tagsPath.isEmpty())
    {
        tagsPath = getXmpTagStringBag("Xmp.expressionmedia.CatalogSets", false);
    }

    if (!tagsPath.isEmpty())
    {
        // Media Pro Catalog Sets use '|' as separator.
        tagsPath = tagsPath.replaceInStrings("|", "/");
        kDebug() << "Tags Path imported from Media Pro: " << tagsPath;
        return true;
    }

    // Try to get Tags Path list from ACDSee 8 Pro categories.
    QString xmlACDSee = getXmpTagString("Xmp.acdsee.categories", false);
    if (!xmlACDSee.isEmpty())
    {
        xmlACDSee.remove("</Categories>");
        xmlACDSee.remove("<Categories>");
        xmlACDSee.replace("/", "\\");

        QStringList xmlTags = xmlACDSee.split("<Category Assigned");
        int category        = 0;

        foreach(const QString& tags, xmlTags)
        {
            if (!tags.isEmpty())
            {
                int count  = tags.count("<\\Category>");
                int length = tags.length() - (11 * count) - 5;

                if (category == 0)
                {
                    tagsPath << tags.mid(5, length);
                }
                else
                {
                    tagsPath.last().append(QString("/") + tags.mid(5, length));
                }

                category = category - count + 1;

                if (tags.left(5) == QString("=\"1\">") && category > 0)
                {
                    tagsPath << tagsPath.last().section("/", 0, category - 1);
                }
            }
        }

        if (!tagsPath.isEmpty())
        {
            kDebug() << "Tags Path imported from ACDSee: " << tagsPath;
            return true;
        }
    }

    // Try to get Tags Path list from XMP keywords.
    tagsPath = getXmpKeywords();
    if (!tagsPath.isEmpty())
    {
        return true;
    }

    // Try to get Tags Path list from IPTC keywords.
    // digiKam 0.9.x has used IPTC keywords to store Tags Path list.
    // This way is obsolete now since digiKam support XMP because IPTC
    // do not support UTF-8 and have strings size limitation. But we will
    // let the capability to import it for interworking issues.
    tagsPath = getIptcKeywords();
    if (!tagsPath.isEmpty())
    {
        // Work around to Imach tags path list hosted in IPTC with '.' as separator.
        QStringList ntp = tagsPath.replaceInStrings(".", "/");

        if (ntp != tagsPath)
        {
            tagsPath = ntp;
            kDebug() << "Tags Path imported from Imach: " << tagsPath;
        }

        return true;
    }

    // Try to get Tags Path list from Exif Windows keywords.
    QString keyWords = getExifTagString("Exif.Image.XPKeywords", false);
    if (!keyWords.isEmpty())
    {
        tagsPath = keyWords.split(";");

        if (!tagsPath.isEmpty())
        {
            return true;
        }
    }

    return false;
}

bool DMetadata::setImageTagsPath(const QStringList& tagsPath) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Keywords for that.
    // Now this way is obsolete, and we use XMP instead.

    // Set the new Tags path list. This is set, not add-to like setXmpKeywords.
    // Unlike the other keyword fields, we do not need to merge existing entries.
    if (supportXmp())
    {
        if (!setXmpTagStringSeq("Xmp.digiKam.TagsList", tagsPath))
        {
            return false;
        }

        // See bug #269418 : register Tags path list for Windows Live Photo Gallery.
        if (!setXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", tagsPath))
        {
            return false;
        }

        QStringList LRtagsPath = tagsPath;
        LRtagsPath             = LRtagsPath.replaceInStrings("/", "|");

        if (!setXmpTagStringBag("Xmp.lr.hierarchicalSubject", LRtagsPath))
        {
            return false;
        }

        QStringList MPtagsPath = tagsPath;
        MPtagsPath             = MPtagsPath.replaceInStrings("/", "|");

        if (!setXmpTagStringBag("Xmp.mediapro.CatalogSets", MPtagsPath))
        {
            return false;
        }

        // Converting Tags path list to ACDSee 8 Pro categories.
        const QString category("<Category Assigned=\"%1\">");
        QStringList splitTags;
        QStringList xmlTags;

        foreach(const QString& tags, tagsPath)
        {
            splitTags   = tags.split("/");
            int current = 0;

            for(int index = 0; index < splitTags.size(); index++)
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
                        xmlTags << QString("</Category>");
                        current = xmlTags.size() - 1;
                    }
                    else
                    {
                        xmlTags.insert(current, splitTags[index]);
                        xmlTags.insert(current + 1, QString("</Category>"));
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

        QString xmlACDSee = QString("<Categories>") + xmlTags.join("") + QString("</Categories>");

        removeXmpTag("Xmp.acdsee.categories");
        if (!xmlTags.isEmpty())
        {
            if (!setXmpTagString("Xmp.acdsee.categories", xmlACDSee, false))
            {
                return false;
            }
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
    const QString personPathTemplate = "Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:PersonDisplayName";
    const QString rectPathTemplate   = "Xmp.MP.RegionInfo/MPRI:Regions[%1]/MPReg:Rectangle";

    for (int i=1; ; i++)
    {
        QString person = getXmpTagString(personPathTemplate.arg(i).toLatin1(), false);

        if (person.isEmpty())
            break;

        // The WLPG tags have the format X.XX, Y.YY, W.WW, H.HH
        // That is, four decimal numbers ranging from 0-1.
        // The top left position is indicated by X.XX, Y.YY (as a
        // percentage of the width/height of the entire image).
        // Similarly the width and height of the face's box are
        // indicated by W.WW and H.HH.
        QString rectString = getXmpTagString(rectPathTemplate.arg(i).toLatin1(), false);
        QStringList list   = rectString.split(',');

        if (list.size() < 4)
        {
            kDebug() << "Cannot parse WLPG rectangle string" << rectString;
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
#if KEXIV2_VERSION >= 0x020301
    // Read face tags as saved by Picasa
    // http://www.exiv2.org/tags-xmp-mwg-rs.html
    const QString mwg_personPathTemplate  = "Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Name";
    const QString mwg_rect_x_PathTemplate = "Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:x";
    const QString mwg_rect_y_PathTemplate = "Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:y";
    const QString mwg_rect_w_PathTemplate = "Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:w";
    const QString mwg_rect_h_PathTemplate = "Xmp.mwg-rs.Regions/mwg-rs:RegionList[%1]/mwg-rs:Area/stArea:h";

    for (int i=1; ; i++)
    {
        QString person = getXmpTagString(mwg_personPathTemplate.arg(i).toLatin1(), false);

        if (person.isEmpty())
            break;

        // x and y is the center point
        float x = getXmpTagString(mwg_rect_x_PathTemplate.arg(i).toLatin1(), false).toFloat();
        float y = getXmpTagString(mwg_rect_y_PathTemplate.arg(i).toLatin1(), false).toFloat();
        float w = getXmpTagString(mwg_rect_w_PathTemplate.arg(i).toLatin1(), false).toFloat();
        float h = getXmpTagString(mwg_rect_h_PathTemplate.arg(i).toLatin1(), false).toFloat();
        QRectF rect(x - w/2,
                    y - h/2,
                    w,
                    h);

        faces.insertMulti(person, rect);
        kDebug() << "Found new rect " << person << " "<< rect;
    }
#endif

    return !faces.isEmpty();
}

bool DMetadata::setImageFacesMap(QMultiMap< QString, QVariant >& facesPath, bool write) const
{
#if KEXIV2_VERSION >= 0x020301
    QString qxmpTagName("Xmp.mwg-rs.Regions/mwg-rs:RegionList");
    QString nameTagKey = qxmpTagName + QString("[%1]/mwg-rs:Name");
    QString typeTagKey = qxmpTagName + QString("[%1]/mwg-rs:Type");
    QString areaTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area");
    QString areaxTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QString("[%1]/mwg-rs:Area/stArea:unit");

    QString winQxmpTagName("Xmp.MP.RegionInfo/MPRI:Regions");
    QString winRectTagKey = winQxmpTagName + QString("[%1]/MPReg:Rectangle");
    QString winNameTagKey = winQxmpTagName + QString("[%1]/MPReg:PersonDisplayName");

    if(!write)
    {
        QString check = getXmpTagString(nameTagKey.arg(1).toLatin1());

        if(check.isEmpty())
            return true;
    }

    setXmpTagString(qxmpTagName.toLatin1(),
                    QString(),KExiv2::XmpTagType(1),false);

    setXmpTagString(winQxmpTagName.toLatin1(),
                    QString(),KExiv2::XmpTagType(1),false);

    QMap<QString,QVariant>::const_iterator it = facesPath.constBegin();
    int i = 1;
    bool ok = true;
    while(it != facesPath.constEnd())
    {
        qreal x,y,w,h;
        it.value().toRectF().getRect(&x,&y,&w,&h);

        /** Write face tags in Windows Live Photo format **/

        QString rectString;

        rectString.append(QString::number(x) + QString(", "));
        rectString.append(QString::number(y) + QString(", "));
        rectString.append(QString::number(w) + QString(", "));
        rectString.append(QString::number(h));

        /** Set tag rect **/
        setXmpTagString(winRectTagKey.arg(i).toLatin1(), rectString,
                             KExiv2::XmpTagType(0),false);
        /** Set tag name **/

        setXmpTagString(winNameTagKey.arg(i).toLatin1(),it.key(),
                             KExiv2::XmpTagType(0),false);

        /** Writing rectangle in Metadata Group format **/
        x += w/2;
        y += h/2;

        /** Set tag name **/
        ok &= setXmpTagString(nameTagKey.arg(i).toLatin1(),
                              it.key(),KExiv2::XmpTagType(0),false);
        /** Set tag type as Face **/
        ok &= setXmpTagString(typeTagKey.arg(i).toLatin1(),
                              QString("Face"),KExiv2::XmpTagType(0),false);

        /** Set tag Area, with xmp type struct **/
        ok &= setXmpTagString(areaTagKey.arg(i).toLatin1(),
                              QString(),KExiv2::XmpTagType(2),false);

        /** Set stArea:x inside Area structure **/
        ok &= setXmpTagString(areaxTagKey.arg(i).toLatin1(),
                              QString::number(x),KExiv2::XmpTagType(0),false);

        /** Set stArea:y inside Area structure **/
        ok &= setXmpTagString(areayTagKey.arg(i).toLatin1(),
                              QString::number(y),KExiv2::XmpTagType(0),false);

        /** Set stArea:w inside Area structure **/
        ok &= setXmpTagString(areawTagKey.arg(i).toLatin1(),
                              QString::number(w),KExiv2::XmpTagType(0),false);

        /** Set stArea:h inside Area structure **/
        ok &= setXmpTagString(areahTagKey.arg(i).toLatin1(),
                              QString::number(h),KExiv2::XmpTagType(0),false);

        /** Set stArea:unit inside Area structure  as normalized **/
        ok &= setXmpTagString(areanormTagKey.arg(i).toLatin1(),
                              QString("normalized"),KExiv2::XmpTagType(0),false);

        ++it;
        ++i;
    }

    return ok;
#else
    Q_UNUSED(facesPath);
    Q_UNUSED(write);
    return false;
#endif
}

bool DMetadata::setMetadataTemplate(const Template& t) const
{
    if (t.isNull())
    {
        return false;
    }

    if (!setProgramId())
    {
        return false;
    }

    QStringList authors           = t.authors();
    QString authorsPosition       = t.authorsPosition();
    QString credit                = t.credit();
    QString source                = t.source();
    KExiv2::AltLangMap copyright  = t.copyright();
    KExiv2::AltLangMap rightUsage = t.rightUsageTerms();
    QString instructions          = t.instructions();

    kDebug() << "Applying Metadata Template: " << t.templateTitle() << " :: " << authors;

    // Set XMP tags. XMP<->IPTC Schema from Photoshop 7.0

    if (supportXmp())
    {
        if (!setXmpTagStringSeq("Xmp.dc.creator", authors, false))
        {
            return false;
        }

        if (!setXmpTagStringSeq("Xmp.tiff.Artist", authors, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.AuthorsPosition", authorsPosition, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Credit", credit, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Source", source, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.dc.source", source, false))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.dc.rights", copyright, false))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.tiff.Copyright", copyright, false))
        {
            return false;
        }

        if (!setXmpTagStringListLangAlt("Xmp.xmpRights.UsageTerms", rightUsage, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.Instructions", instructions, false))
        {
            return false;
        }
    }

    // Set IPTC tags.

    if (!setIptcTagsStringList("Iptc.Application2.Byline", 32,
                               getIptcTagsStringList("Iptc.Application2.Byline"),
                               authors, false))
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

    if (!setIptcTag(copyright["x-default"], 128, "Copyright",     "Iptc.Application2.Copyright"))
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
            str.replace(0, 3, "IPTC");
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

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", info.city, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", info.country, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr", info.address, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode", info.postalCode, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion", info.provinceState, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork", info.email, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork", info.phone, false))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork", info.webUrl, false))
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
        if (!setXmpTagString("Xmp.photoshop.Country", location.country, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.CountryCode", location.countryCode, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.City", location.city, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.Location", location.location, false))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.State", location.provinceState, false))
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

    lensExifTags.append("Exif.CanonCs.LensType");      // Canon Cameras Makernote.
    lensExifTags.append("Exif.CanonCs.Lens");          // Canon Cameras Makernote.
    lensExifTags.append("Exif.Canon.0x0095");          // Alternative Canon Cameras Makernote.
    lensExifTags.append("Exif.NikonLd1.LensIDNumber"); // Nikon Cameras Makernote.
    lensExifTags.append("Exif.NikonLd2.LensIDNumber"); // Nikon Cameras Makernote.
    lensExifTags.append("Exif.NikonLd3.LensIDNumber"); // Nikon Cameras Makernote.
    lensExifTags.append("Exif.Minolta.LensID");        // Minolta Cameras Makernote.
    lensExifTags.append("Exif.Photo.LensModel");       // Sony Cameras Makernote and others?
    lensExifTags.append("Exif.Sony1.LensID");          // Sony Cameras Makernote.
    lensExifTags.append("Exif.Sony2.LensID");          // Sony Cameras Makernote.
    lensExifTags.append("Exif.SonyMinolta.LensID");    // Sony Cameras Makernote.
    lensExifTags.append("Exif.Pentax.LensType");       // Pentax Cameras Makernote.
    lensExifTags.append("Exif.Panasonic.0x0051");      // Panasonic Cameras Makernote.
    lensExifTags.append("Exif.Panasonic.0x0310");      // Panasonic Cameras Makernote.
    lensExifTags.append("Exif.Sigma.LensRange");       // Sigma Cameras Makernote.
    lensExifTags.append("Exif.Samsung2.LensType");     // Samsung Cameras Makernote.
    lensExifTags.append("Exif.Photo.0xFDEA");          // Non-standard Exif tag set by Camera Raw.
    lensExifTags.append("Exif.OlympusEq.LensModel");   // Olympus Cameras Makernote.
    //lensExifTags.append("Exif.OlympusEq.LensType");    // Olympus Cameras Makernote. FIXME is this necessary? exiv2 returns complete name, which doesn't match with lensfun information, see #311295

    // TODO : add Fuji camera Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::const_iterator it = lensExifTags.constBegin(); it != lensExifTags.constEnd(); ++it)
    {
        lens = getExifTagString((*it).toAscii());

        if ( !lens.isEmpty() &&
             !(lens.startsWith('(') && lens.endsWith(')')) )   // To prevent undecoded tag values from Exiv2 as "(65535)".
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
            lens.append(" ");
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
        kDebug() << "Found an ICC profile in Exif metadata";
        return data;
    }

    // Else check the Exif color-space tag and use default profiles that we ship
    switch (getImageColorWorkSpace())
    {
        case DMetadata::WORKSPACE_SRGB:
        {
            kDebug() << "Exif color-space tag is sRGB. Using default sRGB ICC profile.";
            return IccProfile::sRGB();
        }

        case DMetadata::WORKSPACE_ADOBERGB:
        {
            kDebug() << "Exif color-space tag is AdobeRGB. Using default AdobeRGB ICC profile.";
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
    kDebug() << getFilePath() << " ==> " << debugLabel << ": " << truncatedText;
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
    map["x-default"] = str;
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
            return getImageComments()[QString("x-default")].caption;
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
            QString str = getImageTitles()[QString("x-default")].caption;

            if (str.isEmpty())
            {
                return QVariant(QVariant::Map);
            }

            QMap<QString, QVariant> map;
            map["x-default"] = str;
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
            return fromXmpLangAlt("Xmp.video.AspectRatio");
        case MetadataInfo::AudioBitRate:
            return fromXmpLangAlt("Xmp.audio.SampleRate");
        case MetadataInfo::AudioChannelType:
            return fromXmpLangAlt("Xmp.audio.ChannelType");
        case MetadataInfo::AudioCompressor:
            return fromXmpLangAlt("Xmp.audio.Compressor");
        case MetadataInfo::Duration:
            return fromXmpLangAlt("Xmp.video.Duration"); // duration is in ms
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

QString DMetadata::valueToString (const QVariant& value, MetadataInfo::Field field)
{
    KExiv2 exiv2Iface;

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
        case MetadataInfo::AudioCompressor:
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

            QString direction = (directionRef == 'W') ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LongitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (directionRef == 'W') ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
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

            QString direction = (directionRef == 'N') ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LatitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (directionRef == 'N') ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "North");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                   .arg(minutes).arg(QChar(0x2032))
                   .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Altitude:
        {
            QString meters = QString("%L1").arg(value.toDouble(), 0, 'f', 2);
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
            KLocale* locale = KGlobal::locale();
            QString spec = locale->language().toLower() + '-' + locale->country().toLower();

            if (map.contains(spec))
            {
                return map[spec].toString();
            }

            // Try "en-"
            QStringList keys = map.keys();
            QRegExp exp(locale->language().toLower() + '-');
            QStringList matches = keys.filter(exp);

            if (!matches.isEmpty())
            {
                return map[matches.first()].toString();
            }

            // return default
            if (map.contains("x-default"))
            {
                return map["x-default"].toString();
            }

            // return first entry
            return map.begin().value().toString();
        }

        // List
        case MetadataInfo::IptcCoreCreator:
        case MetadataInfo::IptcCoreScene:
        case MetadataInfo::IptcCoreSubjectCode:
            return value.toStringList().join(" ");

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
    for (int i=0; i<size; ++i)
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
        case MetadataInfo::Orientation:                      /// Int, enum from libkexiv2
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
            kWarning() << "Unsupported field " << field << " in DMetadata::possibleValuesForEnumField";
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

KExiv2::AltLangMap DMetadata::toAltLangMap(const QVariant& var)
{
    KExiv2::AltLangMap map;

    if (var.isNull())
    {
        return map;
    }

    switch (var.type())
    {
        case QVariant::String:
            map.insert("x-default", var.toString());
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

#if KEXIV2_VERSION < 0x020300

KUrl DMetadata::sidecarUrl(const KUrl& url)
{
    QString sidecarPath = sidecarFilePathForFile(url.path());
    KUrl sidecarUrl(url);
    sidecarUrl.setPath(sidecarPath);
    return sidecarUrl;
}

KUrl DMetadata::sidecarUrl(const QString& path)
{
    return KUrl::fromPath(sidecarFilePathForFile(path));
}

QString DMetadata::sidecarPath(const QString& path)
{
    return sidecarFilePathForFile(path);
}

bool DMetadata::hasSidecar(const QString& path)
{
    return QFileInfo(sidecarFilePathForFile(path)).exists();
}
#endif // KEXIV2_VERSION < 0x020300

// ---------- Pushed to libkexiv2 for KDE 4.4 --------------

bool DMetadata::addToXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToAdd,
                                     bool setProgramName) const
{
    //#ifdef _XMP_SUPPORT_

    if (!setProgramId(setProgramName))
    {
        return false;
    }

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

    if (setXmpTagStringBag(xmpTagName, newEntries, false))
    {
        return true;
    }

    //#endif // _XMP_SUPPORT_

    return false;
}

bool DMetadata::removeFromXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToRemove,
        bool setProgramName) const
{
    //#ifdef _XMP_SUPPORT_

    if (!setProgramId(setProgramName))
    {
        return false;
    }

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

    if (setXmpTagStringBag(xmpTagName, newEntries, false))
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

bool DMetadata::setXmpKeywords(const QStringList& newKeywords, bool setProgramName) const
{
    return setXmpTagStringBag("Xmp.dc.subject", newKeywords, setProgramName);
}

bool DMetadata::removeXmpKeywords(const QStringList& keywordsToRemove, bool setProgramName)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove, setProgramName);
}

QStringList DMetadata::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool DMetadata::setXmpSubCategories(const QStringList& newSubCategories, bool setProgramName) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories, setProgramName);
}

bool DMetadata::removeXmpSubCategories(const QStringList& subCategoriesToRemove, bool setProgramName)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove, setProgramName);
}

QStringList DMetadata::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool DMetadata::setXmpSubjects(const QStringList& newSubjects, bool setProgramName) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects, setProgramName);
}

bool DMetadata::removeXmpSubjects(const QStringList& subjectsToRemove, bool setProgramName)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove, setProgramName);
}
// End: Pushed to libkexiv2 for KDE4.4

//------------------------------------------------------------------------------------------------
// Compatibility for < KDE 4.4.
#if KEXIV2_VERSION < 0x010000
DMetadata::DMetadata(const KExiv2Data& data)
{
    setData(data);
}

KExiv2Data DMetadata::data() const
{
    KExiv2Data data;
    data.exifData = getExif();
    data.iptcData = getIptc();
    data.xmpData  = getXmp();
    data.imageComments = getComments();
    return data;
}

void DMetadata::setData(const KExiv2Data& data)
{
    setExif(data.exifData);
    setIptc(data.iptcData);
    setXmp(data.xmpData);
    setComments(data.imageComments);
}

#endif
// End: Compatibility for < KDE 4.4
//------------------------------------------------------------------------------------------------

// NOTE: this method can be moved to libkexiv2 later...
bool DMetadata::removeExifColorSpace() const
{
    bool ret =  true;
    ret      &= removeExifTag("Exif.Photo.ColorSpace", true);
    ret      &= removeXmpTag("Xmp.exif.ColorSpace", true);

    return ret;
}

QString DMetadata::getExifTagStringFromTagsList(const QStringList& tagsList) const
{
    QString val;

    foreach(const QString& tag, tagsList)
    {
        val = getExifTagString(tag.toAscii());
        if (!val.isEmpty())
            return val;
    }

    return QString();
}

}  // namespace Digikam
