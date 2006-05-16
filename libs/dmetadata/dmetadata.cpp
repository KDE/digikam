/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright 2006 by Gilles Caulier
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

 // C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qfile.h>
#include <qtextcodec.h>
#include <qwmatrix.h>

// KDE includes.

#include <kdebug.h>
#include <ktempfile.h>

// Exiv2 includes.

#include <exiv2/image.hpp>
#include <exiv2/datasets.hpp>
#include <exiv2/tags.hpp>

// Local includes.

#include "version.h"
#include "dcraw_parse.h"
#include "jpegmetaloader.h"
#include "pngmetaloader.h"
#include "tiffmetaloader.h"
#include "rawmetaloader.h"
#include "dmetadataprivate.h"
#include "dmetadata.h"

namespace Digikam
{

DMetadata::DMetadata()
{
    d = new DMetadataPriv;
}

DMetadata::DMetadata(const QString& filePath, DImg::FORMAT ff)
{
    d = new DMetadataPriv;
    load(filePath, ff);
}

DMetadata::~DMetadata()
{
    delete d;
}

bool DMetadata::applyChanges()
{
    return save(d->filePath, d->fileFormat);
}

QByteArray DMetadata::getComments() const
{
    QByteArray data(d->imageComments.size());
    memcpy(data.data(), d->imageComments.c_str(), d->imageComments.size());
    return data;
}

QByteArray DMetadata::getExif() const
{
    try
    {    
        if (!d->exifMetadata.empty())
        {                
            Exiv2::ExifData exif(d->exifMetadata);
            Exiv2::DataBuf const c2(exif.copy());
            QByteArray data(c2.size_);
            memcpy(data.data(), c2.pData_, c2.size_);
            return data;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Exif data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }       
    
    return QByteArray();
}

QByteArray DMetadata::getIptc() const
{
    try
    {    
        if (!d->iptcMetadata.empty())
        {                
            Exiv2::IptcData iptc(d->iptcMetadata);
            Exiv2::DataBuf const c2(iptc.copy());
            QByteArray data(c2.size_);
            memcpy(data.data(), c2.pData_, c2.size_);
            return data;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Iptc data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }       
    
    return QByteArray();
}

void DMetadata::setComments(const QByteArray& data)
{
    QString string(data);
    const std::string str(string.utf8());
    d->imageComments = str;
}
    
void DMetadata::setExif(const QByteArray& data)
{
    try
    {    
        d->exifMetadata.load((const Exiv2::byte*)data.data(), data.size());
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load Exif data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
}

void DMetadata::setExif(Exiv2::DataBuf const data)
{
    try
    {    
        d->exifMetadata.load(data.pData_, data.size_);
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load Exif data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
}

void DMetadata::setIptc(const QByteArray& data)
{
    try
    {    
        d->iptcMetadata.load((const Exiv2::byte*)data.data(), data.size());
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load Iptc data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
}

void DMetadata::setIptc(Exiv2::DataBuf const data)
{
    try
    {    
        d->iptcMetadata.load(data.pData_, data.size_);
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load Iptc data using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
}

bool DMetadata::load(const QString& filePath, DImg::FORMAT ff)
{
    DImg::FORMAT format = ff;
    
    if (format == DImg::NONE)
        format = fileFormat(filePath);

    d->fileFormat = format;
    d->filePath   = filePath;

    switch (d->fileFormat)
    {
        case(DImg::JPEG):
        {
            JPEGMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::PNG):
        {
            PNGMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::RAW):
        {
            RAWMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::TIFF):
        {
            TIFFMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        default:
        {
            return false;
            break;
        }
    }

    return false;
}

bool DMetadata::save(const QString& filePath, DImg::FORMAT ff)
{
    switch (ff)
    {
        case(DImg::JPEG):
        {
            JPEGMetaLoader loader(this);
            return (loader.save(filePath));
            break;
        }
        case(DImg::PNG):
        {
            PNGMetaLoader loader(this);
            return (loader.save(filePath));
            break;
        }
        case(DImg::RAW):
        {
            RAWMetaLoader loader(this);
            return (loader.save(filePath));
            break;
        }
        case(DImg::TIFF):
        {
            TIFFMetaLoader loader(this);
            return (loader.save(filePath));
            break;
        }
        default:
        {
            return false;
            break;
        }
    }

    return false;
}

DImg::FORMAT DMetadata::fileFormat(const QString& filePath)
{
    if ( filePath == QString::null )
        return DImg::NONE;

    FILE* f = fopen(QFile::encodeName(filePath), "rb");
    
    if (!f)
    {
        kdDebug() << k_funcinfo << "Failed to open file" << endl;
        return DImg::NONE;
    }
    
    const int headerLen = 8;
    unsigned char header[headerLen];
    
    if (fread(&header, 8, 1, f) != 1)
    {
        kdDebug() << k_funcinfo << "Failed to read header" << endl;
        fclose(f);
        return DImg::NONE;
    }
    
    fclose(f);
    
    DcrawParse rawFileParser;
    uchar jpegID[2]    = { 0xFF, 0xD8 };   
    uchar tiffBigID[2] = { 0x4D, 0x4D };
    uchar tiffLilID[2] = { 0x49, 0x49 };
    uchar pngID[8]     = {'\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n'};
    
    if (memcmp(&header, &jpegID, 2) == 0)            // JPEG file ?
    {
        return DImg::JPEG;
    }
    else if (memcmp(&header, &pngID, 8) == 0)        // PNG file ?
    {
        return DImg::PNG;
    }
    else if (rawFileParser.getCameraModel( QFile::encodeName(filePath), NULL, NULL) == 0)
    {
        // RAW File test using dcraw.  
        // Need to test it before TIFF because any RAW file 
        // formats using TIFF header.
        return DImg::RAW;
    }
    else if (memcmp(&header, &tiffBigID, 2) == 0 ||  // TIFF file ?
             memcmp(&header, &tiffLilID, 2) == 0)
    {
        return DImg::TIFF;
    }
    
    return DImg::NONE;
}

QImage DMetadata::getExifThumbnail(bool fixOrientation) const
{
    QImage thumbnail;
    
    if (d->exifMetadata.empty())
       return thumbnail;
    
    try
    {    
        Exiv2::DataBuf const c1(d->exifMetadata.copyThumbnail());
        thumbnail.loadFromData(c1.pData_, c1.size_);
        
        if (!thumbnail.isNull())
        {
            if (fixOrientation)
            {
                Exiv2::ExifKey key("Exif.Thumbnail.Orientation");
                Exiv2::ExifData exifData(d->exifMetadata);
                Exiv2::ExifData::iterator it = exifData.findKey(key);
                if (it != exifData.end())
                {
                    QWMatrix matrix;
                    long orientation = it->toLong();
                    kdDebug() << " Exif Thumbnail Orientation: " << orientation << endl;
                    
                    switch (orientation) 
                    {
                        case ORIENTATION_HFLIP:
                            matrix.scale(-1, 1);
                            break;
                    
                        case ORIENTATION_ROT_180:
                            matrix.rotate(180);
                            break;
                    
                        case ORIENTATION_VFLIP:
                            matrix.scale(1, -1);
                            break;
                    
                        case ORIENTATION_ROT_90_HFLIP:
                            matrix.scale(-1, 1);
                            matrix.rotate(90);
                            break;
                    
                        case ORIENTATION_ROT_90:
                            matrix.rotate(90);
                            break;
                    
                        case ORIENTATION_ROT_90_VFLIP:
                            matrix.scale(1, -1);
                            matrix.rotate(90);
                            break;
                    
                        case ORIENTATION_ROT_270:
                            matrix.rotate(270);
                            break;
                            
                        default:
                            break;
                    }
        
                    if ( orientation != ORIENTATION_NORMAL )
                        thumbnail = thumbnail.xForm( matrix );
                }
                    
                return thumbnail;
            }
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Exif Thumbnail using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return thumbnail;
}

bool DMetadata::setExifThumbnail(const QImage& thumb)
{
    try
    {   
        KTempFile thumbFile(QString::null, "DigikamDMetadataThumb");
        thumbFile.setAutoDelete(true);
        thumb.save(thumbFile.name(), "JPEG");

        const std::string &fileName( (const char*)(QFile::encodeName(thumbFile.name())) );
        d->exifMetadata.setJpegThumbnail( fileName );
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Exif Thumbnail using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

QSize DMetadata::getImageDimensions()
{
    if (d->exifMetadata.empty())
        return QSize();

    try
    {    
        long width=-1, height=-1;
        Exiv2::ExifKey key("Exif.Photo.PixelXDimension");
        Exiv2::ExifData exifData(d->exifMetadata);
        Exiv2::ExifData::iterator it = exifData.findKey(key);
       
        if (it != exifData.end())
            width = it->toLong();

        Exiv2::ExifKey key2("Exif.Photo.PixelYDimension");
        Exiv2::ExifData::iterator it2 = exifData.findKey(key2);
        
        if (it2 != exifData.end())
            height = it2->toLong();
        
        if (width != -1 && height != -1)
            return QSize(width, height);
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot parse image dimensions tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return QSize();
}

DMetadata::ImageOrientation DMetadata::getImageOrientation()
{
    if (d->exifMetadata.empty())
       return ORIENTATION_UNSPECIFIED;

    try
    {    
        Exiv2::ExifKey key("Exif.Image.Orientation");
        Exiv2::ExifData exifData(d->exifMetadata);
        Exiv2::ExifData::iterator it = exifData.findKey(key);
        
        if (it != exifData.end())
        {
            long orientation = it->toLong();
            kdDebug() << " Exif Orientation: " << orientation << endl;
            return (ImageOrientation)orientation;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot parse Exif Orientation tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return ORIENTATION_UNSPECIFIED;
}

bool DMetadata::setImageOrientation(ImageOrientation orientation)
{
    if (d->exifMetadata.empty())
       return false;

    try
    {    
        if (orientation < ORIENTATION_UNSPECIFIED || orientation > ORIENTATION_ROT_270)
        {
            kdDebug() << k_funcinfo << "Exif orientation tag value is not correct!" << endl;
            return false;
        }
        
        d->exifMetadata["Exif.Image.Orientation"] = (uint16_t)orientation;
        kdDebug() << "Exif orientation tag set to: " << orientation << endl;
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Exif Orientation tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

QDateTime DMetadata::getImageDateTime() const
{
    try
    {    
        // In first, trying to get Date & time from Exif tags.
        
        if (!d->exifMetadata.empty())
        {        
            // Try standard Exif date time entry.
    
            Exiv2::ExifKey key("Exif.Image.DateTime");
            Exiv2::ExifData exifData(d->exifMetadata);
            Exiv2::ExifData::iterator it = exifData.findKey(key);
            
            if (it != exifData.end())
            {
                QDateTime dateTime = QDateTime::fromString(it->toString().c_str(), Qt::ISODate);
    
                if (dateTime.isValid())
                {
                    // kdDebug() << "DateTime (Exif standard): " << dateTime << endl;
                    return dateTime;
                }
            }
    
            // Bogus standard Exif date time entry. Try Exif date time original.
    
            Exiv2::ExifKey key2("Exif.Photo.DateTimeOriginal");
            Exiv2::ExifData::iterator it2 = exifData.findKey(key2);
            
            if (it2 != exifData.end())
            {
                QDateTime dateTime = QDateTime::fromString(it2->toString().c_str(), Qt::ISODate);
    
                if (dateTime.isValid())
                {
                    // kdDebug() << "DateTime (Exif original): " << dateTime << endl;
                    return dateTime;
                }
            }
    
            // Bogus Exif date time original entry. Try Exif date time digitized.
    
            Exiv2::ExifKey key3("Exif.Photo.DateTimeDigitized");
            Exiv2::ExifData::iterator it3 = exifData.findKey(key3);
            
            if (it3 != exifData.end())
            {
                QDateTime dateTime = QDateTime::fromString(it3->toString().c_str(), Qt::ISODate);
    
                if (dateTime.isValid())
                {
                    // kdDebug() << "DateTime (Exif digitalized): " << dateTime << endl;
                    return dateTime;
                }
            }
        }
        
        // In second, trying to get Date & time from Iptc tags.
            
        if (!d->iptcMetadata.empty())
        {        
            // Try creation Iptc date time entries.

            Exiv2::IptcKey keyDateCreated("Iptc.Application2.DateCreated");
            Exiv2::IptcData iptcData(d->iptcMetadata);
            Exiv2::IptcData::iterator it = iptcData.findKey(keyDateCreated);
                        
            if (it != iptcData.end())
            {
                QString IptcDateCreated(it->toString().c_str());
    
                Exiv2::IptcKey keyTimeCreated("Iptc.Application2.TimeCreated");
                Exiv2::IptcData::iterator it2 = iptcData.findKey(keyTimeCreated);
                
                if (it2 != iptcData.end())
                {
                    QString IptcTimeCreated(it2->toString().c_str());
                    
                    QDate date = QDate::fromString(IptcDateCreated, Qt::ISODate);
                    QTime time = QTime::fromString(IptcTimeCreated, Qt::ISODate);
                    QDateTime dateTime = QDateTime(date, time);
                    
                    if (dateTime.isValid())
                    {
                        // kdDebug() << "Date (IPTC created): " << dateTime << endl;
                        return dateTime;
                    }                    
                }
            }                        
            
            // Try digitization Iptc date time entries.
    
            Exiv2::IptcKey keyDigitizationDate("Iptc.Application2.DigitizationDate");
            Exiv2::IptcData::iterator it3 = iptcData.findKey(keyDigitizationDate);
                        
            if (it3 != iptcData.end())
            {
                QString IptcDateDigitization(it3->toString().c_str());
    
                Exiv2::IptcKey keyDigitizationTime("Iptc.Application2.DigitizationTime");
                Exiv2::IptcData::iterator it4 = iptcData.findKey(keyDigitizationTime);
                
                if (it4 != iptcData.end())
                {
                    QString IptcTimeDigitization(it4->toString().c_str());
                    
                    QDate date = QDate::fromString(IptcDateDigitization, Qt::ISODate);
                    QTime time = QTime::fromString(IptcTimeDigitization, Qt::ISODate);
                    QDateTime dateTime = QDateTime(date, time);
                    
                    if (dateTime.isValid())
                    {
                        // kdDebug() << "Date (IPTC digitalized): " << dateTime << endl;
                        return dateTime;
                    }                    
                }
            }                       
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot parse Exif date & time tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return QDateTime();
}

bool DMetadata::setImageDateTime(const QDateTime& dateTime)
{
    try
    {    
        kdDebug() << d->filePath << " ==> Date&Time: " << dateTime.toString(Qt::ISODate) << endl;
        
        // In first we write date & time into Exif.
                
        const std::string &exifdatetime(dateTime.toString(Qt::ISODate).ascii());
        d->exifMetadata["Exif.Photo.DateTimeDigitized"] = exifdatetime;
        
        // In Second we write date & time into Iptc.

        const std::string &iptcdate(dateTime.date().toString(Qt::ISODate).ascii());
        d->iptcMetadata["Iptc.Application2.DigitizationDate"] = iptcdate;
        const std::string &iptctime(dateTime.time().toString(Qt::ISODate).ascii());
        d->iptcMetadata["Iptc.Application2.DigitizationTime"] = iptctime;
        
        setImageProgramId();
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Date & Time into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

bool DMetadata::setImageDimensions(const QSize& size)
{
    try
    {    
        d->exifMetadata["Exif.Image.ImageWidth"]      = size.width();
        d->exifMetadata["Exif.Image.ImageLength"]     = size.height();
        d->exifMetadata["Exif.Photo.PixelXDimension"] = size.width();
        d->exifMetadata["Exif.Photo.PixelYDimension"] = size.height();
        
        setImageProgramId();
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Date & Time into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

QString DMetadata::getImageComment() const
{
    try
    {
        if (d->filePath.isEmpty())
            return QString();

        // In first we trying to get image comments, outside of Exif and IPTC.

        QString comments = QString::fromUtf8(d->imageComments.c_str());

        if (!comments.isEmpty())
            return comments;

        // In second, we trying to get Exif comments

        if (!d->exifMetadata.empty())
        {
            Exiv2::ExifKey key("Exif.Photo.UserComment");
            Exiv2::ExifData exifData(d->exifMetadata);
            Exiv2::ExifData::iterator it = exifData.findKey(key);

            if (it != exifData.end())
            {
                QString exifComment = convertCommentValue(*it);

                if (!exifComment.isEmpty())
                  return exifComment;
            }
        }

        // In third, we trying to get IPTC comments

        if (!d->iptcMetadata.empty())
        {
            Exiv2::IptcKey key("Iptc.Application2.Caption");
            Exiv2::IptcData iptcData(d->iptcMetadata);
            Exiv2::IptcData::iterator it = iptcData.findKey(key);

            if (it != iptcData.end())
            {
                QString IptcComment = QString::fromLatin1(it->toString().c_str());

                if (!IptcComment.isEmpty())
                  return IptcComment;
            }
        }

    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Image comments using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }

    return QString();
}

bool DMetadata::setImageComment(const QString& comment)
{
    try
    {
        if (comment.isEmpty())
            return false;

        kdDebug() << d->filePath << " ==> Comment: " << comment << endl;

        // In first we trying to set image comments, outside of Exif and IPTC.

        const std::string str(comment.utf8());
        d->imageComments = str;

        // In Second we write comments into Exif.

        // Be aware that we are dealing with a UCS-2 string.
        // Null termination means \0\0, strlen does not work,
        // do not use any const-char*-only methods,
        // pass a std::string and not a const char * to ExifDatum::operator=().
        const unsigned short *ucs2 = comment.ucs2();
        std::string exifComment("charset=\"Unicode\" ");
        exifComment.append((const char*)ucs2, sizeof(unsigned short) * comment.length());
        d->exifMetadata["Exif.Photo.UserComment"] = exifComment;
        //d->exifMetadata["Exif.Photo.UserComment"] = comment.latin1();

        // In Third we write comments into Iptc. Note that Caption IPTC tag is limited to 2000 char.

        QString commentIptc = comment;
        commentIptc.truncate(2000);
        d->iptcMetadata["Iptc.Application2.Caption"] = commentIptc.latin1();

        setImageProgramId();
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Comment into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }

    return false;
}

QString DMetadata::convertCommentValue(const Exiv2::Exifdatum &exifDatum)
{
    try
    {
        std::string comment = exifDatum.toString();
        std::string charset;
    
        // libexiv2 will prepend "charset=\"SomeCharset\" " if charset is specified
        // Before conversion to QString, we must know the charset, so we stay with std::string for a while
        if (comment.length() > 8 && comment.substr(0, 8) == "charset=")
        {
            // the prepended charset specification is followed by a blank
            std::string::size_type pos = comment.find_first_of(' ');
            if (pos != std::string::npos)
            {
                // extract string between the = and the blank
                charset = comment.substr(8, pos-8);
                // get the rest of the string after the charset specification
                comment = comment.substr(pos+1);
            }
        }
    
        if (charset == "\"Unicode\"")
        {
            // QString expects a null-terminated UCS-2 string.
            // Is it already null terminated? In any case, add termination for safety.
            comment += "\0\0";
            return QString::fromUcs2((unsigned short *)comment.data());
        }
        else if (charset == "\"Jis\"")
        {
            QTextCodec *codec = QTextCodec::codecForName("JIS7");
            return codec->toUnicode(comment.c_str());
        }
        else if (charset == "\"Ascii\"")
        {
            return QString::fromLatin1(comment.c_str());
        }
        else
        {
            // or from local8bit ??
            return QString::fromLatin1(comment.c_str());
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot convert Comment using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }

    return QString();
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
    try
    {    
        if (d->filePath.isEmpty())
            return -1;
            
        if (!d->iptcMetadata.empty())
        {
            Exiv2::IptcKey key("Iptc.Application2.Urgency");
            Exiv2::IptcData iptcData(d->iptcMetadata);
            Exiv2::IptcData::iterator it = iptcData.findKey(key);
            
            if (it != iptcData.end())
            {
                QString IptcUrgency(it->toString().c_str());
    
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
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Image Rating tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return -1;
}

bool DMetadata::setImageRating(int rating)
{
    try
    {    
        if (rating < 0 || rating > 5)
        {
            kdDebug() << k_funcinfo << "Rating value to write out of range!" << endl;
            return false;
        }

        kdDebug() << d->filePath << " ==> Rating: " << rating << endl;
            
        setImageProgramId();
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
        
        d->iptcMetadata["Iptc.Application2.Urgency"] = urgencyTag.ascii();

        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Rating tag into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

// Warning: this method haven't be tested yet!
QStringList DMetadata::getImageKeywords() const
{
    try
    {    
        if (!d->iptcMetadata.empty())
        {
            QStringList keywords;          
            Exiv2::IptcData iptcData(d->iptcMetadata);

            for (Exiv2::IptcData::iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
                QString key = QString::fromLocal8Bit(it->key().c_str());
                
                if (key == QString("Iptc.Application2.Keywords"))
                {
                    QString val(it->toString().c_str());
                    keywords.append(val);
                }
            }
            
            return keywords;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Keywords from image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return QStringList();
}

bool DMetadata::setImageKeywords(const QStringList& oldKeywords, const QStringList& newKeywords)
{
    try
    {    
        QStringList oldkeys = oldKeywords;
        QStringList newkeys = newKeywords;
        
        setImageProgramId();
        kdDebug() << d->filePath << " ==> Keywords: " << newkeys << endl;
        
        // Remove all old keywords.
        Exiv2::IptcData iptcData(d->iptcMetadata);
        Exiv2::IptcData::iterator it = iptcData.begin();

        while(it != iptcData.end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());
            QString val(it->toString().c_str());
            
            if (key == QString("Iptc.Application2.Keywords") && oldKeywords.contains(val))
                it = iptcData.erase(it);
            else 
                ++it;
        };

        // Add new keywords. Note that Keywords IPTC tag is limited to 64 char but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.Keywords");

        for (QStringList::iterator it = newkeys.begin(); it != newkeys.end(); ++it)
        {
            QString key = *it;
            key.truncate(64);
            
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(key.latin1());
            iptcData.add(iptcTag, val.get());        
        }

        d->iptcMetadata = iptcData;

        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Keywords into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

bool DMetadata::setImagePhotographerId(const QString& author, const QString& authorTitle)
{
    try
    {    
        setImageProgramId();

        // Byline IPTC tag is limited to 32 char.
        QString Byline = author;
        Byline.truncate(32);
        kdDebug() << d->filePath << " ==> Author: " << Byline << endl;
        d->iptcMetadata["Iptc.Application2.Byline"] = Byline.latin1();
        
        // BylineTitle IPTC tag is limited to 32 char.
        QString BylineTitle = authorTitle;
        BylineTitle.truncate(32);
        kdDebug() << d->filePath << " ==> Author Title: " << BylineTitle << endl;
        d->iptcMetadata["Iptc.Application2.BylineTitle"] = BylineTitle.latin1();

        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Photographer identity into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

bool DMetadata::setImageCredits(const QString& credit, const QString& source, const QString& copyright)
{
    try
    {    
        setImageProgramId();

        // Credit IPTC tag is limited to 32 char.
        QString Credit = credit;
        Credit.truncate(32);
        kdDebug() << d->filePath << " ==> Credit: " << Credit << endl;
        d->iptcMetadata["Iptc.Application2.Credit"] = Credit.latin1();

        // Source IPTC tag is limited to 32 char.
        QString Source = source;
        Source.truncate(32);
        kdDebug() << d->filePath << " ==> Source: " << Source << endl;
        d->iptcMetadata["Iptc.Application2.Source"] = Source.latin1();

        // Copyright IPTC tag is limited to 128 char.
        QString Copyright = copyright;
        Copyright.truncate(128);
        kdDebug() << d->filePath << " ==> Copyright: " << Copyright << endl;
        d->iptcMetadata["Iptc.Application2.Copyright"] = Copyright.latin1();

        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Credits identity into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

bool DMetadata::setImageProgramId()
{
    try
    {    
        d->iptcMetadata["Iptc.Application2.Program"] = "digiKam";
        d->iptcMetadata["Iptc.Application2.ProgramVersion"] = digikam_version;
        return true;
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Program identity into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

PhotoInfoContainer DMetadata::getPhotographInformations() const
{
    try
    {    
        if (!d->exifMetadata.empty())
        {
            PhotoInfoContainer photoInfo;
            photoInfo.dateTime = getImageDateTime();
            photoInfo.make     = getExifTagValue("Exif.Image.Make");
            photoInfo.model    = getExifTagValue("Exif.Image.Model");

            photoInfo.aperture = getExifTagValue("Exif.Photo.FNumber");
            if (photoInfo.aperture.isEmpty())
                photoInfo.aperture = getExifTagValue("Exif.Photo.ApertureValue");

            photoInfo.exposureTime = getExifTagValue("Exif.Photo.ExposureTime");
            if (photoInfo.exposureTime.isEmpty())
                photoInfo.exposureTime = getExifTagValue("Exif.Photo.ShutterSpeedValue");

            photoInfo.exposureMode    = getExifTagValue("Exif.Photo.ExposureMode");
            photoInfo.exposureProgram = getExifTagValue("Exif.Photo.ExposureProgram");
                            
            photoInfo.focalLenght     = getExifTagValue("Exif.Photo.FocalLength");
            photoInfo.focalLenght35mm = getExifTagValue("Exif.Photo.FocalLengthIn35mmFilm");

            photoInfo.sensitivity = getExifTagValue("Exif.Photo.ISOSpeedRatings");
            if (photoInfo.sensitivity.isEmpty())
                photoInfo.sensitivity = getExifTagValue("Exif.Photo.ExposureIndex");

            photoInfo.flash = getExifTagValue("Exif.Photo.Flash");
            photoInfo.whiteBalance = getExifTagValue("Exif.Photo.WhiteBalance");

            return photoInfo;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot get Photograph informations into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        

    return PhotoInfoContainer();
}

QString DMetadata::getExifTagValue(const char* exifTagName) const
{
    try
    {    
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata);
        Exiv2::ExifData::iterator it = exifData.findKey(exifKey);
        if (it != exifData.end())
        {
            std::ostringstream os;
            os << *it;
            QString tagValue = QString::fromLocal8Bit(os.str().c_str());
            tagValue.replace("\n", " ");
            return tagValue;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot find Exif key '"
                  << exifTagName << "' into image using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        

    return QString();
}

}  // NameSpace Digikam
