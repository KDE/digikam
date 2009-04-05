/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dimgloader.h"

// Qt includes

#include <QFile>

// KDE includes

#include <kstandarddirs.h>
#include <kcodecs.h>       // for KMD5
#include <kdebug.h>

// Local includes

#include "dimgprivate.h"
#include "dmetadata.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

DImgLoader::DImgLoader(DImg* image)
          : m_image(image)
{
    m_loadFlags = LoadAll;
}

void DImgLoader::setLoadFlags(LoadFlags flags)
{
    m_loadFlags = flags;
}

bool DImgLoader::hasLoadedData() const
{
    return m_loadFlags & LoadImageData;
}

int DImgLoader::granularity(DImgLoaderObserver *observer, int total, float progressSlice)
{
    // Splits expect total value into the chunks where checks shall occur
    // and combines this with a possible correction factor from observer.
    // Progress slice is the part of 100% concerned with the current granularity
    // (E.g. in a loop only the values from 10% to 90% are used, then progressSlice is 0.8)
    // Current default is 1/20, that is progress info every 5%
    int granularity=0;

    if (observer)
        granularity = (int)(( total / (20 * progressSlice)) / observer->granularity());

    return granularity ? granularity : 1;
}

unsigned char*& DImgLoader::imageData()
{
    return m_image->m_priv->data;
}

unsigned int& DImgLoader::imageWidth()
{
    return m_image->m_priv->width;
}

unsigned int& DImgLoader::imageHeight()
{
    return m_image->m_priv->height;
}

bool DImgLoader::imageHasAlpha()
{
    return m_image->hasAlpha();
}

bool DImgLoader::imageSixteenBit()
{
    return m_image->sixteenBit();
}

int DImgLoader::imageBitsDepth()
{
    return m_image->bitsDepth();
}

int DImgLoader::imageBytesDepth()
{
    return m_image->bytesDepth();
}

QMap<int, QByteArray>& DImgLoader::imageMetaData()
{
    return m_image->m_priv->metaData;
}

QVariant DImgLoader::imageGetAttribute(const QString& key)
{
    return m_image->attribute(key);
}

QString DImgLoader::imageGetEmbbededText(const QString& key)
{
    return m_image->embeddedText(key);
}

void DImgLoader::imageSetAttribute(const QString& key, const QVariant& value)
{
    m_image->setAttribute(key, value);
}

QMap<QString, QString>& DImgLoader::imageEmbeddedText()
{
    return m_image->m_priv->embeddedText;
}

void DImgLoader::imageSetEmbbededText(const QString& key, const QString& text)
{
    m_image->setEmbeddedText(key, text);
}

bool DImgLoader::readMetadata(const QString& filePath, DImg::FORMAT /*ff*/)
{
    if (! (m_loadFlags & LoadMetadata || m_loadFlags & LoadUniqueHash) )
        return false;

    QMap<int, QByteArray>& imageMetadata = imageMetaData();
    imageMetadata.clear();

    DMetadata metaDataFromFile;
    if (!metaDataFromFile.load(filePath))
        return false;

    // Do not insert null data into metaData map:
    // Even if byte array is null, if there is a key in the map, it will
    // be interpreted as "There was data, so write it again to the file".
    if (!metaDataFromFile.getComments().isNull())
        imageMetadata.insert(DImg::COM, metaDataFromFile.getComments());
    if (!metaDataFromFile.getExif().isNull())
        imageMetadata.insert(DImg::EXIF, metaDataFromFile.getExif());
    if (!metaDataFromFile.getIptc().isNull())
        imageMetadata.insert(DImg::IPTC, metaDataFromFile.getIptc());
    if (!metaDataFromFile.getXmp().isNull())
        imageMetadata.insert(DImg::XMP, metaDataFromFile.getXmp());

    return true;
}

bool DImgLoader::saveMetadata(const QString& filePath)
{
    DMetadata metaDataToFile(filePath);
    metaDataToFile.setComments(m_image->getComments());
    metaDataToFile.setExif(m_image->getExif());
    metaDataToFile.setIptc(m_image->getIptc());
    metaDataToFile.setXmp(m_image->getXmp());
    return metaDataToFile.applyChanges();
}

bool DImgLoader::checkExifWorkingColorSpace()
{
    DMetadata metaData;
    metaData.setExif(m_image->getExif());

    // Check if Exif data contains an ICC color profile.
    QByteArray profile = metaData.getExifTagData("Exif.Image.InterColorProfile");
    if (!profile.isNull())
    {
        kDebug(50003) << "Found an ICC profile in Exif metadata" << endl;
        m_image->setICCProfil(profile);
        return true;
    }

    // Else check the Exif color-space tag and use a default profiles available with libkdcraw.
    QString directory = KStandardDirs::installPath("data") + QString("libkdcraw/profiles/");

    switch(metaData.getImageColorWorkSpace())
    {
        case DMetadata::WORKSPACE_SRGB:
        {
            kDebug(50003) << "Exif color-space tag is sRGB. Using default sRGB ICC profile." << endl;
            m_image->getICCProfilFromFile(directory + QString("srgb.icm"));
            return true;
            break;
        }

        case DMetadata::WORKSPACE_ADOBERGB:
        {
            kDebug(50003) << "Exif color-space tag is AdobeRGB. Using default AdobeRGB ICC profile." << endl;
            m_image->getICCProfilFromFile(directory + QString("adobergb.icm"));
            return true;
            break;
        }

        default:
            break;
    }

    return false;
}

QByteArray DImgLoader::uniqueHash(const QString &filePath, const DImg &img, bool loadMetadata)
{
    QByteArray bv;

    if (loadMetadata)
    {
        DMetadata metaDataFromFile(filePath);
        bv = metaDataFromFile.getExif();
    }
    else
    {
        bv = img.getExif();
    }

    // Create the unique ID

    KMD5 md5;

    // First, read the Exif data into the hash
    md5.update( bv );

    // Second, read in the first 8KB of the file
    QFile qfile( filePath );

    char databuf[8192];
    int readlen = 0;
    QByteArray size = 0;

    if( qfile.open( QIODevice::Unbuffered | QIODevice::ReadOnly ) )
    {
        if( ( readlen = qfile.read( databuf, 8192 ) ) > 0 )
        {
            md5.update( databuf, readlen );
            md5.update( size.setNum( qfile.size() ) );
            return md5.hexDigest();
        }
        else
            return QByteArray();
    }

    return QByteArray();
}

}  // namespace Digikam
