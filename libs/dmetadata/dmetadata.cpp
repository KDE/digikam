/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qdom.h>
#include <qfile.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawinfocontainer.h>
#include <libkdcraw/kdcraw.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
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
    // else we will use dcraw to extract minimal information.

    if (!KExiv2::load(filePath))
    {
        if (!loadUsingDcraw(filePath))
            return false;
    }

    return true;
}

bool DMetadata::loadUsingDcraw(const QString& filePath)
{
    KDcrawIface::DcrawInfoContainer identify;
    if (KDcrawIface::KDcraw::rawFileIdentify(identify, filePath))
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

    // Check Exif rating tag set by Windows Vista
    // Note : no need to check rating in percent tags (Exif.image.0x4747) here because 
    // its appear always with rating tag value (Exif.image.0x4749). 

    if (!getExif().isEmpty())
    {
        long rating = -1;
        if (getExifTagLong("Exif.Image.0x4746", rating))
        {
            if (rating >= 0 && rating <= 5)
                return rating;            
        }
    }

    // Check Iptc Urgency tag content

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

    // Set Iptc Urgency tag value.

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
                                      int& rating, QStringList& tagsPath) 
{
    rating = 0;

    QByteArray data = getIptcTagData("Iptc.Application2.0x00ff");
    if (data.isEmpty())
        return false;
    QByteArray decompressedData = qUncompress(data);
    QString doc;
    QDataStream ds(decompressedData, IO_ReadOnly);
    ds >> doc;

    QDomDocument xmlDoc;
    QString error;
    int row, col;
    if (!xmlDoc.setContent(doc, true, &error, &row, &col))
    {
        DDebug() << doc << endl;
        DDebug() << error << " :: row=" << row << " , col=" << col << endl; 
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
                                      int rating, const QStringList& tagsPath)
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
    QDataStream ds(data, IO_WriteOnly);
    ds << xmlDoc.toString();
    compressedData = qCompress(data);
    return (setIptcTagData("Iptc.Application2.0x00ff", compressedData));
}

}  // NameSpace Digikam
