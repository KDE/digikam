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

// Local includes.

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

void DMetadata::setIptc(const QByteArray& data)
{
    m_iptcMetadata = data;
}

bool DMetadata::load(const QString& filePath, DImg::FORMAT ff)
{
    DImg::FORMAT format = ff;
    
    if (format == DImg::NONE)
        format = fileFormat(filePath);

    switch (format)
    {
        case(DImg::JPEG):
        {
            kdDebug() << filePath << " : JPEG file identified" << endl;
            JPEGMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::PNG):
        {
            kdDebug() << filePath << " : PNG file identified" << endl;
            PNGMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::RAW):
        {
            kdDebug() << filePath << " : RAW file identified" << endl;
            RAWMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        case(DImg::TIFF):
        {
            kdDebug() << filePath << " : TIFF file identified" << endl;
            TIFFMetaLoader loader(this);
            return (loader.load(filePath));
            break;
        }
        default:
        {
            kdDebug() << filePath << " : Unsupported image format !!!" << endl;
            return false;
            break;
        }
    }

    return false;
}

bool DMetadata::save(const QString& filePath, const QString& format)
{
    if (format.isEmpty())
        return false;

    QString frm = format.upper();

    if (frm == "JPEG" || frm == "JPG")
    {
        JPEGMetaLoader loader(this);
        return loader.save(filePath);
    }
    else if (frm == "PNG")
    {
        PNGMetaLoader loader(this);
        return loader.save(filePath);
    }
    else if (frm == "TIFF" || frm == "TIF")
    {
        TIFFMetaLoader loader(this);
        return loader.save(filePath);
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

DMetadata::ImageOrientation DMetadata::getExifImageOrientation()
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

QDateTime DMetadata::getExifDateTime() const
{
    if (m_exifMetadata.isEmpty())
       return QDateTime();

    try
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
                kdDebug() << " Exif Date (standard): " << dateTime << endl;
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
                kdDebug() << " Exif Date (original): " << dateTime << endl;
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
                kdDebug() << " Exif Date (digitalized): " << dateTime << endl;
                return dateTime;
            }
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot parse Exif date tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return QDateTime();
}

bool DMetadata::writeExifImageOrientation(const QString& filePath, ImageOrientation orientation)
{
    try
    {    
        if (filePath.isEmpty())
            return false;
            
        if (orientation < ORIENTATION_UNSPECIFIED || orientation > ORIENTATION_ROT_270)
        {
            kdDebug() << k_funcinfo << "Exif orientation tag value is not correct!" << endl;
            return false;
        }
            
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath)));
        
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        
        if (exifData.empty())
        {
            kdDebug() << "Cannot set Exif Orientation tag from " << filePath 
                      << " because there is no Exif informations available!" << endl;
        }
        else
        {
            exifData["Exif.Image.Orientation"] = (uint16_t)orientation;
            image->setExifData(exifData);
            image->writeMetadata();
            kdDebug() << "Exif orientation tag set to:" << orientation << endl;
            return true;
        }    
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot set Exif Orientation tag using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        
    
    return false;
}

}  // NameSpace Digikam
