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
#include <qwmatrix.h>

// KDE includes.

#include <kdebug.h>

// Exiv2 includes.

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/datasets.hpp>

// Local includes.

#include "version.h"
#include "dcraw_parse.h"
#include "jpegmetaloader.h"
#include "pngmetaloader.h"
#include "tiffmetaloader.h"
#include "rawmetaloader.h"
#include "dmetadata.h"

namespace Digikam
{

DMetadata::DMetadata(const QString& filePath, DImg::FORMAT ff)
{
    load(filePath, ff);
}

bool DMetadata::applyChanges()
{
    return save(m_filePath, m_fileFormat);
}

QByteArray DMetadata::getExif() const
{
    return m_exifMetadata;
}

QByteArray DMetadata::getIptc() const
{
    return m_iptcMetadata;
}
    
void DMetadata::setExif(const QByteArray& data)
{
    m_exifMetadata = data;
}

void DMetadata::setExif(Exiv2::DataBuf const data)
{
    m_exifMetadata = QByteArray(data.size_);
    memcpy(m_exifMetadata.data(), data.pData_, data.size_);
}

void DMetadata::setIptc(const QByteArray& data)
{
    m_iptcMetadata = data;
}

void DMetadata::setIptc(Exiv2::DataBuf const data)
{
    m_iptcMetadata = QByteArray(data.size_);
    memcpy(m_iptcMetadata.data(), data.pData_, data.size_);
}

bool DMetadata::load(const QString& filePath, DImg::FORMAT ff)
{
    DImg::FORMAT format = ff;
    
    if (format == DImg::NONE)
        format = fileFormat(filePath);

    m_fileFormat = format;
    m_filePath   = filePath;

    switch (m_fileFormat)
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
    
    if (m_exifMetadata.isEmpty())
       return thumbnail;
    
    try
    {    
        Exiv2::ExifData exifData;
        exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());
        Exiv2::DataBuf const c1(exifData.copyThumbnail());
        thumbnail.loadFromData(c1.pData_, c1.size_);
        
        if (!thumbnail.isNull())
        {
            if (fixOrientation)
            {
                Exiv2::ExifKey key("Exif.Image.Orientation");
                Exiv2::ExifData::iterator it = exifData.findKey(key);
                if (it != exifData.end())
                {
                    QWMatrix matrix;
                    long orientation = it->toLong();
                    kdDebug() << " Exif Orientation: " << orientation << endl;
                    
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
        kdDebug() << "Cannot parse Exif Thumbnail using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return thumbnail;
}

DMetadata::ImageOrientation DMetadata::getImageOrientation()
{
    if (m_exifMetadata.isEmpty())
       return ORIENTATION_UNSPECIFIED;

    try
    {    
        Exiv2::ExifData exifData;
        exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());
        Exiv2::ExifKey key("Exif.Image.Orientation");
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
    if (m_exifMetadata.isEmpty())
       return false;

    try
    {    
        if (orientation < ORIENTATION_UNSPECIFIED || orientation > ORIENTATION_ROT_270)
        {
            kdDebug() << k_funcinfo << "Exif orientation tag value is not correct!" << endl;
            return false;
        }
        
        Exiv2::ExifData exifData;
        exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());

        exifData["Exif.Image.Orientation"] = (uint16_t)orientation;
        kdDebug() << "Exif orientation tag set to: " << orientation << endl;
        setExif(exifData.copy());
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
        
        if (!m_exifMetadata.isEmpty())
        {        
            Exiv2::ExifData exifData;
            exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());
    
            // Try standard Exif date time entry.
    
            Exiv2::ExifKey key("Exif.Image.DateTime");
            Exiv2::ExifData::iterator it = exifData.findKey(key);
            
            if (it != exifData.end())
            {
                QDateTime dateTime = QDateTime::fromString(it->toString().c_str(), Qt::ISODate);
    
                if (dateTime.isValid())
                {
                    kdDebug() << "DateTime (Exif standard): " << dateTime << endl;
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
                    kdDebug() << "DateTime (Exif original): " << dateTime << endl;
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
                    kdDebug() << "DateTime (Exif digitalized): " << dateTime << endl;
                    return dateTime;
                }
            }
        }
        
        // In second, trying to get Date & time from Iptc tags.
            
        if (!m_iptcMetadata.isEmpty())
        {        
            Exiv2::IptcData iptcData;
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
            
            // Try creation Iptc date time entries.
    
            Exiv2::IptcKey keyDateCreated("Iptc.Application2.DateCreated");
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
                        kdDebug() << "Date (IPTC created): " << dateTime << endl;
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
                        kdDebug() << "Date (IPTC digitalized): " << dateTime << endl;
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
        kdDebug() << m_filePath << " ==> Date&Time: " << dateTime.toString(Qt::ISODate) << endl;
        
        Exiv2::ExifData exifData;
        if (!m_exifMetadata.isEmpty())
            exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());
    
        // In first we write date & time into Exif.
                
        const std::string &exifdatetime(dateTime.toString(Qt::ISODate).ascii());
        exifData["Exif.Photo.DateTimeDigitized"] = exifdatetime;
        setExif(exifData.copy());
        
        // In Second we write date & time into Iptc.

        Exiv2::IptcData iptcData;
        if (!m_iptcMetadata.isEmpty())
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
 
        setImageProgramId(iptcData);

        const std::string &iptcdate(dateTime.date().toString(Qt::ISODate).ascii());
        iptcData["Iptc.Application2.DigitizationDate"] = iptcdate;
        const std::string &iptctime(dateTime.time().toString(Qt::ISODate).ascii());
        iptcData["Iptc.Application2.DigitizationTime"] = iptctime;
        setIptc(iptcData.copy());
        
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
        if (m_filePath.isEmpty())
            return QString();
            
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(m_filePath)));
        
        // In first we trying to get image comments, outside of Exif and IPTC.
                                              
        image->readMetadata();
        QString comment(image->comment().c_str());
        
        if (!comment.isEmpty())
           return comment;
           
        // In second, we trying to get Exif comments   
                
        if (m_exifMetadata.isEmpty())
        {
            Exiv2::ExifData exifData;
            exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());
            Exiv2::ExifKey key("Exif.Photo.UserComment");
            Exiv2::ExifData::iterator it = exifData.findKey(key);
            
            if (it != exifData.end())
            {
                QString ExifComment(it->toString().c_str());
    
                if (!ExifComment.isEmpty())
                  return ExifComment;
            }
        }
        
        // In third, we trying to get IPTC comments   
                
        if (m_iptcMetadata.isEmpty())
        {
            Exiv2::IptcData iptcData;
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
            Exiv2::IptcKey key("Iptc.Application2.Caption");
            Exiv2::IptcData::iterator it = iptcData.findKey(key);
            
            if (it != iptcData.end())
            {
                QString IptcComment(it->toString().c_str());
    
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

        kdDebug() << m_filePath << " ==> Comment: " << comment << endl;
            
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(m_filePath)));
        
        // In first we write comments outside of Exif and IPTC if possible.
                                              
        image->readMetadata();
        const std::string &str(comment.latin1());
        image->setComment(str);
        image->writeMetadata();

        // In Second we write comments into Exif.
                
        Exiv2::ExifData exifData;
        if (!m_exifMetadata.isEmpty())
            exifData.load((const Exiv2::byte*)m_exifMetadata.data(), m_exifMetadata.size());

        exifData["Exif.Photo.UserComment"] = comment.latin1();
        setExif(exifData.copy());
        
        // In Third we write comments into Iptc. Note that Caption IPTC tag is limited to 2000 char.

        Exiv2::IptcData iptcData;
        if (!m_iptcMetadata.isEmpty())
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());

        setImageProgramId(iptcData);

        QString commentIptc = comment;
        commentIptc.truncate(2000);
        iptcData["Iptc.Application2.Caption"] = commentIptc.latin1();
        setIptc(iptcData.copy());
    
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
        if (m_filePath.isEmpty())
            return -1;
            
        if (m_iptcMetadata.isEmpty())
        {
            Exiv2::IptcData iptcData;
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
            Exiv2::IptcKey key("Iptc.Application2.Urgency");
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

        kdDebug() << m_filePath << " ==> Rating: " << rating << endl;
            
        Exiv2::IptcData iptcData;
        if (!m_iptcMetadata.isEmpty())
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());

        setImageProgramId(iptcData);
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
        
        iptcData["Iptc.Application2.Urgency"] = urgencyTag.ascii();
        setIptc(iptcData.copy());

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
        if (m_iptcMetadata.isEmpty())
        {
            Exiv2::IptcData iptcData;
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
            QStringList keywords;
            
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
    
    return QString();
}

bool DMetadata::setImageKeywords(const QStringList& keywords)
{
    try
    {    
        if (keywords.isEmpty())
            return false;

        QStringList keys = keywords;
        
        Exiv2::IptcData iptcData;
        if (!m_iptcMetadata.isEmpty())
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());
        
        setImageProgramId(iptcData);
        kdDebug() << m_filePath << " ==> Keywords: " << keywords << endl;
        
        // Keywords IPTC tag is limited to 64 char but can be redondancy.
        
        for (QStringList::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            QString key = *it;
            key.truncate(64);
            Exiv2::IptcKey iptcTag("Iptc.Application2.Keywords");
            
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(key.latin1());
            iptcData.add(iptcTag, val.get());        
        }
        
        setIptc(iptcData.copy());
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

bool DMetadata::setImagePhotographerId(const QString& author, const QString& authorTitle,
                const QString& city, const QString& province, const QString& country)
{
    try
    {    
        Exiv2::IptcData iptcData;
        if (!m_iptcMetadata.isEmpty())
            iptcData.load((const Exiv2::byte*)m_iptcMetadata.data(), m_iptcMetadata.size());

        setImageProgramId(iptcData);

        // Byline IPTC tag is limited to 32 char.
        QString Byline = author;
        Byline.truncate(32);
        kdDebug() << m_filePath << " ==> Author: " << Byline << endl;
        iptcData["Iptc.Application2.Byline"] = Byline.latin1();
        
        // BylineTitle IPTC tag is limited to 32 char.
        QString BylineTitle = authorTitle;
        BylineTitle.truncate(32);
        kdDebug() << m_filePath << " ==> Author Title: " << BylineTitle << endl;
        iptcData["Iptc.Application2.BylineTitle"] = BylineTitle.latin1();

        // City IPTC tag is limited to 32 char.
        QString City = city;
        City.truncate(32);
        kdDebug() << m_filePath << " ==> City: " << City << endl;
        iptcData["Iptc.Application2.City"] = City.latin1();

        // ProvinceState IPTC tag is limited to 32 char.
        QString ProvinceState = province;
        ProvinceState.truncate(32);
        kdDebug() << m_filePath << " ==> Province: " << ProvinceState << endl;
        iptcData["Iptc.Application2.ProvinceState"] = ProvinceState.latin1();

        // CountryName IPTC tag is limited to 32 char.
        QString CountryName = country;
        CountryName.truncate(64);
        kdDebug() << m_filePath << " ==> Country: " << CountryName << endl;
        iptcData["Iptc.Application2.CountryName"] = CountryName.latin1();

        setIptc(iptcData.copy());
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

bool DMetadata::setImageProgramId(Exiv2::IptcData& iptcData)
{
    try
    {    
        iptcData["Iptc.Application2.Program"] = "digiKam";
        iptcData["Iptc.Application2.ProgramVersion"] = digikam_version;
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

}  // NameSpace Digikam
