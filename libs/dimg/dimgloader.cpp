/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-14
 * Description : DImg image loader interface 
 *
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

// Local includes.

#include "dimgprivate.h"
#include "dmetadata.h"
#include "dimgloader.h"

namespace Digikam
{

DImgLoader::DImgLoader(DImg* image)
          : m_image(image)
{
}

int DImgLoader::granularity(DImgLoaderObserver *observer, int total, float progressSlice)
{
    // Splits expect total value into the chunks where checks shall occur
    // and combines this with a possible correction factor from observer.
    // Progress slice is the part of 100% concerned with the current granularity
    // (E.g. in a loop only the values from 10% to 90% are used, then progressSlice is 0.8)
    // Current default is 1/20, that is progress info every 5%
    int granularity = (int)(( total / (20 * progressSlice)) / observer->granularity());
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

void DImgLoader::imageSetCameraModel(const QString& model)
{
    m_image->setCameraModel(model);
}

void DImgLoader::imageSetCameraConstructor(const QString& constructor)
{
    m_image->setCameraConstructor(constructor);
}

void DImgLoader::readMetadata(const QString& filePath, DImg::FORMAT ff)
{
    QMap<int, QByteArray>& imageMetadata = imageMetaData();
    imageMetadata.clear();

    DMetadata metaDataFromFile(filePath, ff);
    // Do not insert null data into metaData map:
    // Even if byte array is null, if there is a key in the map, it will
    // be interpreted as "There was data, so write it again to the file".
    if (!metaDataFromFile.getComments().isNull())
        imageMetadata.insert(DImg::COM, metaDataFromFile.getComments());
    if (!metaDataFromFile.getExif().isNull())
        imageMetadata.insert(DImg::EXIF, metaDataFromFile.getExif());
    if (!metaDataFromFile.getIptc().isNull())
        imageMetadata.insert(DImg::IPTC, metaDataFromFile.getIptc());
}

void DImgLoader::saveMetadata(const QString& filePath)
{
    DMetadata metaDataToFile(filePath);
    metaDataToFile.setComments(m_image->getComments());
    metaDataToFile.setExif(m_image->getExif());
    metaDataToFile.setIptc(m_image->getIptc());
    metaDataToFile.applyChanges();
}

}  // NameSpace Digikam
