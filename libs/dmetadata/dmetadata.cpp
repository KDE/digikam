/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dcrawiface.h"
#include "dmetadata.h"

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

bool DMetadata::load(const QString& filePath)
{
    // In first, we trying to get metadata using Exiv2,
    // else we will use dcraw to extract minimal informations.

    if (!KExiv2::load(filePath))
    {
        if (!loadUsingDcraw(filePath))
            return false;
    }

    return true;
}

bool DMetadata::loadUsingDcraw(const QString& filePath)
{
    DcrawInfoContainer identify;
    if (DcrawIface::rawFileIdentify(identify, filePath))
    {
        long int num=1, den=1;

        if (!identify.model.isNull())
            setExifTagString("Exif.Image.Model", identify.model.latin1(), false);

        if (!identify.make.isNull())
            setExifTagString("Exif.Image.Make", identify.make.latin1(), false);

        if (identify.sensitivity != -1)
            setExifTagLong("Exif.Photo.ISOSpeedRatings", identify.sensitivity, false);

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

        // A RAW picture is always uncalibrated. */
        setImageColorWorkSpace(WORKSPACE_UNCALIBRATED, false);

        return true;
    }

    return false;
}

QString DMetadata::getImageComment() const
{
    if (getFilePath().isEmpty())
        return QString();

    // In first we trying to get image comments, outside of Exif and IPTC.

    QString comment = getCommentsDecoded();
    if (!comment.isEmpty())
        return comment;

    // In second, we trying to get Exif comments

    if (!getExif().isEmpty())
    {
        QString exifComment = getExifComment();     
        if (!exifComment.isEmpty())
            return exifComment;
    }

    // In third, we trying to get IPTC comments

    if (!getIptc().isEmpty())
    {
        QString iptcComment = getIptcTagString("Iptc.Application2.Caption", false);
        if (!iptcComment.isEmpty() && !iptcComment.stripWhiteSpace().isEmpty())
            return iptcComment;
    }

    return QString();
}

bool DMetadata::setImageComment(const QString& comment)
{
    //See bug #139313: An empty string is also a valid value
    //if (comment.isEmpty())
    //    return false;
    
    DDebug() << getFilePath() << " ==> Comment: " << comment << endl;

    if (!setProgramId())
        return false;

    // In first we trying to set image comments, outside of Exif and IPTC.

    if (!setComments(comment.utf8()))
        return false;

    // In Second we write comments into Exif.

    if (!setExifComment(comment))
        return false;

    // In Third we write comments into Iptc.
    // Note that Caption IPTC tag is limited to 2000 char and ASCII charset.

    QString commentIptc = comment;
    commentIptc.truncate(2000);

    if (!setIptcTagString("Iptc.Application2.Caption", commentIptc))
        return false;

    return true;
}

/*
Iptc.Application2.Urgency <==> digiKam Rating links:

digiKam     IPTC
Rating      Urgency

0 star  <=>  8          // Least important
1 star  <=>  7
1 star  <==  6
2 star  <=>  5
3 star  <=>  4
4 star  <==  3
4 star  <=>  2
5 star  <=>  1          // Most important
*/

int DMetadata::getImageRating() const
{
    if (getFilePath().isEmpty())
        return -1;

    if (!getIptc().isEmpty())
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

bool DMetadata::setImageRating(int rating)
{
    if (rating < 0 || rating > 5)
    {
        DDebug() << k_funcinfo << "Rating value to write out of range!" << endl;
        return false;
    }

    DDebug() << getFilePath() << " ==> Rating: " << rating << endl;

    if (!setProgramId())
        return false;

    QString urgencyTag;
    
    switch(rating)
    {
        case 0:
            urgencyTag = QString("8");
            break;
        case 1:
            urgencyTag = QString("7");
            break;
        case 2:
            urgencyTag = QString("5");
            break;
        case 3:
            urgencyTag = QString("4");
            break;
        case 4:
            urgencyTag = QString("3");
            break;
        case 5:
            urgencyTag = QString("1");
            break;
    }
        
    if (!setIptcTagString("Iptc.Application2.Urgency", urgencyTag))
        return false;

    return true;
}

bool DMetadata::setImagePhotographerId(const QString& author, const QString& authorTitle)
{
    if (!setProgramId())
        return false;

    // Byline IPTC tag is limited to 32 char.
    QString Byline = author;
    Byline.truncate(32);
    DDebug() << getFilePath() << " ==> Author: " << Byline << endl;
    if (!setIptcTagString("Iptc.Application2.Byline", Byline))
        return false;
    
    // BylineTitle IPTC tag is limited to 32 char.
    QString BylineTitle = authorTitle;
    BylineTitle.truncate(32);
    DDebug() << getFilePath() << " ==> Author Title: " << BylineTitle << endl;
    if (!setIptcTagString("Iptc.Application2.BylineTitle", BylineTitle))
        return false;

    return true;
}

bool DMetadata::setImageCredits(const QString& credit, const QString& source, const QString& copyright)
{
    if (!setProgramId())
        return false;

    // Credit IPTC tag is limited to 32 char.
    QString Credit = credit;
    Credit.truncate(32);
    DDebug() << getFilePath() << " ==> Credit: " << Credit << endl;
    if (!setIptcTagString("Iptc.Application2.Credit", Credit))
        return false;

    // Source IPTC tag is limited to 32 char.
    QString Source = source;
    Source.truncate(32);
    DDebug() << getFilePath() << " ==> Source: " << Source << endl;
    if (!setIptcTagString("Iptc.Application2.Source", Source))
        return false;

    // Copyright IPTC tag is limited to 128 char.
    QString Copyright = copyright;
    Copyright.truncate(128);
    DDebug() << getFilePath() << " ==> Copyright: " << Copyright << endl;
    if (!setIptcTagString("Iptc.Application2.Copyright", Copyright))
        return false;

    return true;
}

bool DMetadata::setProgramId(bool on)
{
    if (on)
    {
        QString version(digikam_version);
        QString software("digiKam");
        return setImageProgramId(software, version);
    }
    
    return true;
}

PhotoInfoContainer DMetadata::getPhotographInformations() const
{
    PhotoInfoContainer photoInfo;

    if (!getExif().isEmpty())
    {
        photoInfo.dateTime = getImageDateTime();
        photoInfo.make     = getExifTagString("Exif.Image.Make");
        photoInfo.model    = getExifTagString("Exif.Image.Model");

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");
        if (photoInfo.aperture.isEmpty())
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");
        if (photoInfo.exposureTime.isEmpty())
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");
        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");
        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");

        photoInfo.sensitivity = getExifTagString("Exif.Photo.ISOSpeedRatings");
        if (photoInfo.sensitivity.isEmpty())
            photoInfo.sensitivity = getExifTagString("Exif.Photo.ExposureIndex");

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");
        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");
    }

    return photoInfo;
}

}  // NameSpace Digikam
