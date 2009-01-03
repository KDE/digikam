/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// C++ includes.

#include <cmath>

// Qt includes.

#include <QDomDocument>
#include <QFile>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawinfocontainer.h>
#include <libkdcraw/kdcraw.h>

// Local includes.

#include "constants.h"
#include "version.h"

namespace Digikam
{

DMetadata::DMetadata()
         : KExiv2Iface::KExiv2()
{
}

DMetadata::DMetadata(const QString& filePath)
         : KExiv2Iface::KExiv2()
{
    load(filePath);
}

DMetadata::~DMetadata()
{
}

bool DMetadata::load(const QString& filePath) const
{
    // In first, we trying to get metadata using Exiv2,
    // else we will use dcraw to extract minimal information.

    if (!KExiv2::load(filePath))
    {
        if (!loadUsingDcraw(filePath))
            return false;
    }

    return true;
}

bool DMetadata::loadUsingDcraw(const QString& filePath) const
{
    KDcrawIface::DcrawInfoContainer identify;
    if (KDcrawIface::KDcraw::rawFileIdentify(identify, filePath))
    {
        long int num=1, den=1;

        if (!identify.model.isNull())
            setExifTagString("Exif.Image.Model", identify.model.toLatin1(), false);

        if (!identify.make.isNull())
            setExifTagString("Exif.Image.Make", identify.make.toLatin1(), false);

        if (!identify.owner.isNull())
            setExifTagString("Exif.Image.Artist", identify.owner.toLatin1(), false);

        if (identify.sensitivity != -1)
            setExifTagLong("Exif.Photo.ISOSpeedRatings", lroundf(identify.sensitivity), false);

        if (identify.dateTime.isValid())
            setImageDateTime(identify.dateTime, false, false);

        if (identify.exposureTime != -1.0)
        {
            convertToRational(1/identify.exposureTime, &num, &den, 8);
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
            setImageDimensions(identify.imageSize, false);

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

QString DMetadata::getImageComment() const
{
    if (getFilePath().isEmpty())
        return QString();

    // In first we trying to get image comments, outside of XMP, Exif, and IPTC.
    // For JPEG, string is extracted from JFIF Comments section.
    // For PNG, string is extracted from iTXt chunk.

    QString comment = getCommentsDecoded();
    if (!comment.isEmpty())
        return comment;

    // In second, we trying to get Exif comments

    if (hasExif())
    {
        QString exifComment = getExifComment();
        if (!exifComment.isEmpty())
            return exifComment;
    }

    // In third, we trying to get XMP comments. Language Alternative rule is not yet used.

    if (hasXmp())
    {
        QString xmpComment = getXmpTagStringLangAlt("Xmp.dc.description", QString(), false);
        if (!xmpComment.isEmpty())
            return xmpComment;

        xmpComment = getXmpTagStringLangAlt("Xmp.exif.UserComment", QString(), false);
        if (!xmpComment.isEmpty())
            return xmpComment;


        xmpComment = getXmpTagStringLangAlt("Xmp.tiff.ImageDescription", QString(), false);
        if (!xmpComment.isEmpty())
            return xmpComment;
}

    // In four, we trying to get IPTC comments

    if (hasIptc())
    {
        QString iptcComment = getIptcTagString("Iptc.Application2.Caption", false);
        if (!iptcComment.isEmpty() && !iptcComment.trimmed().isEmpty())
            return iptcComment;
    }

    return QString();
}

bool DMetadata::setImageComment(const QString& comment) const
{
    //See B.K.O #139313: An empty string is also a valid value
    /*if (comment.isEmpty())
          return false;*/

    kDebug(50003) << getFilePath() << " ==> Comment: " << comment << endl;

    // In first we set image comments, outside of Exif, XMP, and IPTC.

    if (!setComments(comment.toUtf8()))
        return false;

    // In Second we write comments into Exif.

    if (!setExifComment(comment))
        return false;

    // In Third we write comments into XMP. Language Alternative rule is not yet used.

    if (!setXmpTagStringLangAlt("Xmp.dc.description", comment, QString(), false))
        return false;

    if (!setXmpTagStringLangAlt("Xmp.exif.UserComment", comment, QString(), false))
        return false;

    if (!setXmpTagStringLangAlt("Xmp.tiff.ImageDescription", comment, QString(), false))
        return false;

    // In Four we write comments into IPTC.
    // Note that Caption IPTC tag is limited to 2000 char and ASCII charset.

    QString commentIptc = comment;
    commentIptc.truncate(2000);

    if (!setIptcTagString("Iptc.Application2.Caption", commentIptc))
        return false;

    return true;
}

int DMetadata::getImageRating() const
{
    if (getFilePath().isEmpty())
        return -1;

    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.xmp.Rating", false);
        if (!value.isEmpty())
        {
            bool ok     = false;
            long rating = value.toLong(&ok);
            if (ok && rating >= RatingMin && rating <= RatingMax)
                return rating;
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
                return rating;
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
                return 5;
            else if (IptcUrgency == QString("2"))
                return 4;
            else if (IptcUrgency == QString("3"))
                return 4;
            else if (IptcUrgency == QString("4"))
                return 3;
            else if (IptcUrgency == QString("5"))
                return 2;
            else if (IptcUrgency == QString("6"))
                return 1;
            else if (IptcUrgency == QString("7"))
                return 1;
            else if (IptcUrgency == QString("8"))
                return 0;
        }
    }

    return -1;
}

bool DMetadata::setImageRating(int rating) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Urgency to store Rating.
    // Now this way is obsolete, and we use standard XMP rating tag instead.

    if (rating < RatingMin || rating > RatingMax)
    {
        kDebug(50003) << "Rating value to write is out of range!" << endl;
        return false;
    }

    kDebug(50003) << getFilePath() << " ==> Rating: " << rating << endl;

    if (!setProgramId())
        return false;

    // Set standard XMP rating tag.

    if (!setXmpTagString("Xmp.xmp.Rating", QString::number(rating)))
        return false;

    // Set Exif rating tag used by Windows Vista.

    if (!setExifTagLong("Exif.Image.0x4746", rating))
        return false;

    // Wrapper around rating percents managed by Windows Vista.
    int ratePercents = 0;
    switch(rating)
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
        return false;

    return true;
}

PhotoInfoContainer DMetadata::getPhotographInformations() const
{
    PhotoInfoContainer photoInfo;

    if (hasExif() || hasXmp())
    {
        photoInfo.dateTime = getImageDateTime();

        photoInfo.make     = getExifTagString("Exif.Image.Make");
        if (photoInfo.make.isEmpty())
            photoInfo.make = getXmpTagString("Xmp.tiff.Make");

        photoInfo.model    = getExifTagString("Exif.Image.Model");
        if (photoInfo.model.isEmpty())
            photoInfo.model = getXmpTagString("Xmp.tiff.Model");

        photoInfo.lens     = getLensDescription();

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");
        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");
            if (photoInfo.aperture.isEmpty())
            {
                photoInfo.aperture = getXmpTagString("Xmp.exif.FNumber");
                if (photoInfo.aperture.isEmpty())
                    photoInfo.aperture = getXmpTagString("Xmp.exif.ApertureValue");
            }
        }

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");
        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");
            if (photoInfo.exposureTime.isEmpty())
            {
                photoInfo.exposureTime = getXmpTagString("Xmp.exif.ExposureTime");
                if (photoInfo.exposureTime.isEmpty())
                    photoInfo.exposureTime = getXmpTagString("Xmp.exif.ShutterSpeedValue");
            }
        }

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");
        if (photoInfo.exposureMode.isEmpty())
            photoInfo.exposureMode = getXmpTagString("Xmp.exif.ExposureMode");

        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");
        if (photoInfo.exposureProgram.isEmpty())
            photoInfo.exposureProgram = getXmpTagString("Xmp.exif.ExposureProgram");

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");
        if (photoInfo.focalLength.isEmpty())
            photoInfo.focalLength = getXmpTagString("Xmp.exif.FocalLength");

        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");
        if (photoInfo.focalLength35mm.isEmpty())
            photoInfo.focalLength35mm = getXmpTagString("Xmp.exif.FocalLengthIn35mmFilm");

        photoInfo.sensitivity = getExifTagString("Exif.Photo.ISOSpeedRatings");
        if (photoInfo.sensitivity.isEmpty())
        {
            photoInfo.sensitivity = getExifTagString("Exif.Photo.ExposureIndex");
            if (photoInfo.sensitivity.isEmpty())
            {
                photoInfo.sensitivity = getXmpTagString("Xmp.exif.ISOSpeedRatings");
                if (photoInfo.sensitivity.isEmpty())
                    photoInfo.sensitivity = getXmpTagString("Xmp.exif.ExposureIndex");
            }
        }

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");
        if (photoInfo.flash.isEmpty())
            photoInfo.flash = getXmpTagString("Xmp.exif.Flash");

        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");
        if (photoInfo.whiteBalance.isEmpty())
            photoInfo.whiteBalance = getXmpTagString("Xmp.exif.WhiteBalance");
    }

    return photoInfo;
}

bool DMetadata::getImageTagsPath(QStringList& tagsPath) const
{
    // Try to get Tags Path list from XMP in first.
    tagsPath = getXmpTagStringSeq("Xmp.digiKam.TagsList", false);
    if (!tagsPath.isEmpty())
        return true;

    // Try to get Tags Path list from XMP keywords.
    tagsPath = getXmpKeywords();
    if (!tagsPath.isEmpty())
        return true;

    // Try to get Tags Path list from IPTC keywords.
    // digiKam 0.9.x has used IPTC keywords to store Tags Path list.
    // This way is obsolete now since digiKam support XMP because IPTC
    // do not support UTF-8 and have strings size limitation. But we will
    // let the capability to import it for interworking issues.
    tagsPath = getIptcKeywords();
    if (!tagsPath.isEmpty())
        return true;

    return false;
}

bool DMetadata::setImageTagsPath(const QStringList& tagsPath) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Keywords for that.
    // Now this way is obsolete, and we use XMP instead.

    // Remove the old Tags path list from metadata if already exist.
    if (!removeXmpTag("Xmp.digiKam.TagsList", false))
        return false;

    // An now, add the new Tags path list as well.
    if (!setXmpTagStringSeq("Xmp.digiKam.TagsList", tagsPath))
        return false;

    return true;
}

bool DMetadata::setImagePhotographerId(const QString& author, const QString& authorTitle) const
{
    if (!setProgramId())
        return false;

    // Set XMP tags. XMP<->IPTC Schema from Photoshop 7.0

    // Create a list of authors including old one witch already exists.
    QStringList oldAuthors = getXmpTagStringSeq("Xmp.dc.creator", false);
    QStringList newAuthors(author);

    for (QStringList::Iterator it = oldAuthors.begin(); it != oldAuthors.end(); ++it)
    {
        if (!newAuthors.contains(*it))
            newAuthors.append(*it);
    }

    if (!setXmpTagStringSeq("Xmp.dc.creator", newAuthors, false))
        return false;

    if (!setXmpTagStringSeq("Xmp.tiff.Artist", newAuthors, false))
        return false;

    if (!setXmpTagString("Xmp.photoshop.AuthorsPosition", authorTitle, false))
        return false;

    // Set IPTC tags.

    if (!setIptcTag(author,      32, "Author",       "Iptc.Application2.Byline"))      return false;
    if (!setIptcTag(authorTitle, 32, "Author Title", "Iptc.Application2.BylineTitle")) return false;

    return true;
}

bool DMetadata::setImageCredits(const QString& credit, const QString& source, const QString& copyright) const
{
    if (!setProgramId())
        return false;

    // Set XMP tags. XMP<->IPTC Schema from Photoshop 7.0

    if (!setXmpTagString("Xmp.photoshop.Credit", credit, false))
        return false;

    if (!setXmpTagString("Xmp.photoshop.Source", source, false))
        return false;

    if (!setXmpTagString("Xmp.dc.source", source, false))
        return false;

    // NOTE : language Alternative rule is not yet used here.
    if (!setXmpTagStringLangAlt("Xmp.dc.rights", copyright, QString(), false))
        return false;

    if (!setXmpTagStringLangAlt("Xmp.tiff.Copyright", copyright, QString(), false))
        return false;

    // Set IPTC tags.

    if (!setIptcTag(credit,     32, "Credit",    "Iptc.Application2.Credit"))    return false;
    if (!setIptcTag(source,     32, "Source",    "Iptc.Application2.Source"))    return false;
    if (!setIptcTag(copyright, 128, "Copyright", "Iptc.Application2.Copyright")) return false;

    return true;
}

QString DMetadata::getLensDescription() const
{
    QString lens;
    QStringList lensExifTags;
    lensExifTags.append("Exif.CanonCs.Lens");        // Canon Cameras Makernote.
    lensExifTags.append("Exif.Canon.0x0095");        // Alternative Canon Cameras Makernote.
    lensExifTags.append("Exif.Nikon3.LensData");     // Nikon Cameras Makernote.
    lensExifTags.append("Exif.Minolta.LensID");      // Minolta Cameras Makernote.
    lensExifTags.append("Exif.Pentax.LensType");     // Pentax Cameras Makernote.
    lensExifTags.append("Exif.Panasonic.0x0310");    // Panasonic Cameras Makernote.
    lensExifTags.append("Exif.Sigma.LensRange");     // Sigma Cameras Makernote.
    lensExifTags.append("Exif.Photo.0xFDEA");        // Nonstandard Exif tag set by Camera Raw.
    // TODO : add Fuji, Olympus, Sony Cameras Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::Iterator it = lensExifTags.begin(); it != lensExifTags.end(); ++it)
    {
        lens = getExifTagString((*it).toAscii());
        if (!lens.isEmpty())
            return lens;
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
            lens.append(" ");

        lens.append(getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
    }

    return lens;
}

bool DMetadata::setIptcTag(const QString& text, int maxLength,
                           const char* debugLabel, const char* tagKey)  const
{
    QString truncatedText = text;
    truncatedText.truncate(maxLength);
    kDebug(50003) << getFilePath() << " ==> " << debugLabel << ": " << truncatedText << endl;
    return setIptcTagString(tagKey, truncatedText);    // returns false if failed
}

inline QVariant DMetadata::fromExifOrXmp(const char *exifTagName, const char *xmpTagName) const
{
    QVariant var;

    if (exifTagName)
    {
        var = getExifTagVariant(exifTagName, false);
        if (!var.isNull())
            return var;
    }

    if (xmpTagName)
    {
        var = getXmpTagVariant(xmpTagName);
        if (!var.isNull())
            return var;
    }

    return var;
}

inline QVariant DMetadata::fromIptcOrXmp(const char *iptcTagName, const char *xmpTagName) const
{
    if (iptcTagName)
    {
        QString iptcValue = getIptcTagString(iptcTagName);
        if (!iptcValue.isNull())
            return iptcValue;
    }

    if (xmpTagName)
    {
        QVariant var = getXmpTagVariant(xmpTagName);
        if (!var.isNull())
            return var;
    }

    return QVariant(QVariant::String);
}

inline QVariant DMetadata::fromIptcEmulateList(const char *iptcTagName) const
{
    QStringList iptcValues = getIptcTagsStringList(iptcTagName);

    if (iptcValues.isEmpty())
        return QVariant(QVariant::StringList);
    return iptcValues;
}

inline QVariant DMetadata::fromXmpList(const char *xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
        return QVariant(QVariant::StringList);
    return var;
}

inline QVariant DMetadata::fromIptcEmulateLangAlt(const char *iptcTagName) const
{
    QString str = getIptcTagString(iptcTagName);
    if (str.isNull())
        return QVariant(QVariant::Map);

    QMap<QString, QVariant> map;
    map["x-default"] = str;
    return map;
}

inline QVariant DMetadata::fromXmpLangAlt(const char *xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
        return QVariant(QVariant::Map);
    return var;
}

QVariant DMetadata::getMetadataField(MetadataInfo::Field field)
{
    switch (field)
    {
        case MetadataInfo::Comment:
            return getImageComment();
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
                return var;

            var = fromXmpLangAlt("Xmp.tiff.ImageDescription");
            if (!var.isNull())
                return var;

            return fromIptcEmulateLangAlt("Iptc.Application2.Caption");
        }
        case MetadataInfo::Headline:
            return fromIptcOrXmp("Iptc.Application2.Headline", "Xmp.photoshop.Headline");
        case MetadataInfo::Title:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.title");
            if (!var.isNull())
                return var;

            return fromIptcEmulateLangAlt("Iptc.Application2.ObjectName");
        }
        case MetadataInfo::DescriptionWriter:
            return fromIptcOrXmp("Iptc.Application2.Writer", "Xmp.photoshop.CaptionWriter");

        case MetadataInfo::Keywords:
        {
            QStringList list;
            if (getImageTagsPath(list))
                return list;
            return QVariant(QVariant::StringList);
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
            return fromExifOrXmp("Exif.Image.Make", "Xmp.tiff.Make");
        case MetadataInfo::Model:
            return fromExifOrXmp("Exif.Image.Model", "Xmp.tiff.Model");
        case MetadataInfo::Lens:
            return getLensDescription();
        case MetadataInfo::Aperture:
        {
            QVariant var = fromExifOrXmp("Exif.Photo.FNumber", "Xmp.exif.FNumber");
            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ApertureValue", "Xmp.exif.ApertureValue");
                if (!var.isNull())
                    var = apexApertureToFNumber(var.toDouble());
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
                    var = apexShutterSpeedToExposureTime(var.toDouble());
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
                return longitude;
            else
                return QVariant(QVariant::Double);
        }
        case MetadataInfo::Latitude:
            return getGPSLatitudeString();
        case MetadataInfo::LatitudeNumber:
        {
            double latitude;
            if (getGPSLatitudeNumber(&latitude))
                return latitude;
            else
                return QVariant(QVariant::Double);
        }
        case MetadataInfo::Altitude:
        {
            double altitude;
            if (getGPSAltitude(&altitude))
                return altitude;
            else
                return QVariant(QVariant::Double);
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
                return var;

            var = fromXmpLangAlt("Xmp.tiff.Copyright");
            if (!var.isNull())
                return var;

            return fromIptcEmulateLangAlt("Iptc.Application2.Copyright");
        }
        case MetadataInfo::IptcCoreCreator:
        {
            QVariant var = fromXmpList("Xmp.dc.creator");
            if (!var.isNull())
                return var;

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
            // TODO: find out correct IPTC tag
            return fromIptcOrXmp(/*"Iptc.Application2.ObjectAttribute"?*/ 0, "Xmp.iptc.IntellectualGenre");
        case MetadataInfo::IptcCoreJobID:
            return fromIptcOrXmp("Iptc.Application2.TransmissionReference", "Xmp.photoshop.TransmissionReference");
        case MetadataInfo::IptcCoreScene:
            return fromXmpList("Xmp.iptc.Scene");
        case MetadataInfo::IptcCoreSubjectCode:
        {
            QVariant var = fromXmpList("Xmp.iptc.SubjectCode");
            if (!var.isNull())
                return var;

            return fromIptcEmulateList("Iptc.Application2.Subject");
        }

        default:
            return QVariant();
    }
}

QVariantList DMetadata::getMetadataFields(const MetadataFields &fields)
{
    QVariantList list;
    foreach (MetadataInfo::Field field, fields)
    {
        list << getMetadataField(field);
    }
    return list;
}

QString DMetadata::valueToString (const QVariant &value, MetadataInfo::Field field)
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

        case MetadataInfo::Longitude:
        {
            int degrees, minutes;
            double seconds;
            char directionRef;
            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
                return QString();
            QString direction = (directionRef == 'W') ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                                              .arg(minutes).arg(QChar(0x2032))
                                              .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LongitudeNumber:
        {
            int degrees, minutes;
            double seconds;
            char directionRef;
            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (directionRef == 'W') ?
                                i18nc("For use in longitude coordinate", "West") : i18nc("For use in longitude coordinate", "East");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                                              .arg(minutes).arg(QChar(0x2032))
                                              .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::Latitude:
        {
            int degrees, minutes;
            double seconds;
            char directionRef;
            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
                return QString();
            QString direction = (directionRef == 'N') ?
                                i18nc("For use in latitude coordinate", "North") : i18nc("For use in latitude coordinate", "South");
            return QString("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                                              .arg(minutes).arg(QChar(0x2032))
                                              .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }
        case MetadataInfo::LatitudeNumber:
        {
            int degrees, minutes;
            double seconds;
            char directionRef;
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
            return i18nc("Height in meters", "%L1m", meters);
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
                return QString();
            else if (map.size() == 1)
                return map.begin().value().toString();
            // Try "en-us"
            KLocale *locale = KGlobal::locale();
            QString spec = locale->language().toLower() + '-' + locale->country().toLower();
            if (map.contains(spec))
                return map[spec].toString();

            // Try "en-"
            QStringList keys = map.keys();
            QRegExp exp(locale->language().toLower() + '-');
            QStringList matches = keys.filter(exp);
            if (!matches.isEmpty())
                return map[matches.first()].toString();

            // return default
            if (map.contains("x-default"))
                return map["x-default"].toString();

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
            return QString();
    }
}

QStringList DMetadata::valuesToString(const QVariantList &values, const MetadataFields &fields)
{
    int size = values.size();
    Q_ASSERT(size == values.size());

    QStringList list;
    for (int i=0; i<size; i++)
    {
        list << valueToString(values[i], fields[i]);
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
            kWarning(50003) << "Unsupported field " << field << " in DMetadata::possibleValuesForEnumField" << endl;
            return map;
    }

    for (int i = min; i <= max; i++)
    {
        map[i] = valueToString(i, field);
    }
    return map;
}

double DMetadata::apexApertureToFNumber(double aperture)
{
    // convert from APEX. See Exif spec, Annex C.
    if (aperture == 0.0)
        return 1;
    else if (aperture == 1.0)
        return 1.4;
    else if (aperture == 2.0)
        return 2;
    else if (aperture == 3.0)
        return 2.8;
    else if (aperture == 4.0)
        return 4;
    else if (aperture == 5.0)
        return 5.6;
    else if (aperture == 6.0)
        return 8;
    else if (aperture == 7.0)
        return 11;
    else if (aperture == 8.0)
        return 16;
    else if (aperture == 9.0)
        return 22;
    else if (aperture == 10.0)
        return 32;
    return exp(log(2) * aperture / 2.0);
}

double DMetadata::apexShutterSpeedToExposureTime(double shutterSpeed)
{
    // convert from APEX. See Exif spec, Annex C.
    if (shutterSpeed == -5.0)
        return 30;
    else if (shutterSpeed == -4.0)
        return 15;
    else if (shutterSpeed == -3.0)
        return 8;
    else if (shutterSpeed == -2.0)
        return 4;
    else if (shutterSpeed == -1.0)
        return 2;
    else if (shutterSpeed == 0.0)
        return 1;
    else if (shutterSpeed == 1.0)
        return 0.5;
    else if (shutterSpeed == 2.0)
        return 0.25;
    else if (shutterSpeed == 3.0)
        return 0.125;
    else if (shutterSpeed == 4.0)
        return 1.0 / 15.0;
    else if (shutterSpeed == 5.0)
        return 1.0 / 30.0;
    else if (shutterSpeed == 6.0)
        return 1.0 / 60.0;
    else if (shutterSpeed == 7.0)
        return 0.008; // 1/125
    else if (shutterSpeed == 8.0)
        return 0.004; // 1/250
    else if (shutterSpeed == 9.0)
        return 0.002; // 1/500
    else if (shutterSpeed == 10.0)
        return 0.001; // 1/1000
    else if (shutterSpeed == 11.0)
        return 0.0005; // 1/2000
    // additions by me
    else if (shutterSpeed == 12.0)
        return 0.00025; // 1/4000
    else if (shutterSpeed == 13.0)
        return 0.000125; // 1/8000

    return exp( - log(2) * shutterSpeed);
}

/**
The following methods set and get an XML dataset into a private IPTC.Application2 tags
to backup digiKam image properties. The XML text data are compressed using zlib and stored
like a byte array. The XML text data format are like below:

<?xml version="1.0" encoding="UTF-8"?>
<digikamproperties>
 <comments value="A cool photo from Adrien..." />
 <date value="2006-11-23T13:36:26" />
 <rating value="4" />
 <tagslist>
  <tag path="Gilles/Adrien/testphoto" />
  <tag path="monuments/Trocadero/Tour Eiffel" />
  <tag path="City/Paris" />
 </tagslist>
</digikamproperties>

*/

bool DMetadata::getXMLImageProperties(QString& comments, QDateTime& date,
                                      int& rating, QStringList& tagsPath) const
{
    rating = 0;

    QByteArray data = getIptcTagData("Iptc.Application2.0x00ff");
    if (data.isEmpty())
        return false;
    QByteArray decompressedData = qUncompress(data);
    QString doc;
    QDataStream ds(&decompressedData, QIODevice::ReadOnly);
    ds >> doc;

    QDomDocument xmlDoc;
    QString error;
    int row, col;
    if (!xmlDoc.setContent(doc, true, &error, &row, &col))
    {
        kDebug(50003) << doc << endl;
        kDebug(50003) << error << " :: row=" << row << " , col=" << col << endl;
        return false;
    }

    QDomElement rootElem = xmlDoc.documentElement();
    if (rootElem.tagName() != QString::fromLatin1("digikamproperties"))
        return false;

    for (QDomNode node = rootElem.firstChild();
         !node.isNull(); node = node.nextSibling())
    {
        QDomElement e = node.toElement();
        QString name  = e.tagName();
        QString val   = e.attribute(QString::fromLatin1("value"));

        if (name == QString::fromLatin1("comments"))
        {
            comments = val;
        }
        else if (name == QString::fromLatin1("date"))
        {
            if (val.isEmpty()) continue;
            date = QDateTime::fromString(val, Qt::ISODate);
        }
        else if (name == QString::fromLatin1("rating"))
        {
            if (val.isEmpty()) continue;
            bool ok=false;
            rating = val.toInt(&ok);
            if (!ok) rating = 0;
        }
        else if (name == QString::fromLatin1("tagslist"))
        {
            for (QDomNode node2 = e.firstChild();
                !node2.isNull(); node2 = node2.nextSibling())
            {
                QDomElement e2 = node2.toElement();
                QString name2  = e2.tagName();
                QString val2   = e2.attribute(QString::fromLatin1("path"));

                if (name2 == QString::fromLatin1("tag"))
                {
                    if (val2.isEmpty()) continue;
                    tagsPath.append(val2);
                }
            }
        }
    }

    return true;
}

bool DMetadata::setXMLImageProperties(const QString& comments, const QDateTime& date,
                                      int rating, const QStringList& tagsPath) const
{
    QDomDocument xmlDoc;

    xmlDoc.appendChild(xmlDoc.createProcessingInstruction( QString::fromLatin1("xml"),
                       QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );

    QDomElement propertiesElem = xmlDoc.createElement(QString::fromLatin1("digikamproperties"));
    xmlDoc.appendChild( propertiesElem );

    QDomElement c = xmlDoc.createElement(QString::fromLatin1("comments"));
    c.setAttribute(QString::fromLatin1("value"), comments);
    propertiesElem.appendChild(c);

    QDomElement d = xmlDoc.createElement(QString::fromLatin1("date"));
    d.setAttribute(QString::fromLatin1("value"), date.toString(Qt::ISODate));
    propertiesElem.appendChild(d);

    QDomElement r = xmlDoc.createElement(QString::fromLatin1("rating"));
    r.setAttribute(QString::fromLatin1("value"), rating);
    propertiesElem.appendChild(r);

    QDomElement tagsElem = xmlDoc.createElement(QString::fromLatin1("tagslist"));
    propertiesElem.appendChild(tagsElem);

    QStringList path = tagsPath;
    for ( QStringList::Iterator it = path.begin(); it != path.end(); ++it )
    {
        QDomElement e = xmlDoc.createElement(QString::fromLatin1("tag"));
        e.setAttribute(QString::fromLatin1("path"), *it);
        tagsElem.appendChild(e);
    }

    QByteArray  data, compressedData;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds << xmlDoc.toString();
    compressedData = qCompress(data);
    return (setIptcTagData("Iptc.Application2.0x00ff", compressedData));
}

}  // namespace Digikam
