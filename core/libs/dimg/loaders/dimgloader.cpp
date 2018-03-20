/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : DImg image loader interface
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <limits>
#include <new>
#include <cstddef>

// Qt includes

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "dimg_p.h"
#include "dmetadata.h"
#include "dimgloaderobserver.h"
#include "kmemoryinfo.h"

namespace Digikam
{

DImgLoader::DImgLoader(DImg* const image)
    : m_image(image)
{
    m_loadFlags = LoadAll;
}

DImgLoader::~DImgLoader()
{
}

void DImgLoader::setLoadFlags(LoadFlags flags)
{
    m_loadFlags = flags;
}

bool DImgLoader::hasLoadedData() const
{
    return (m_loadFlags & LoadImageData) && m_image->m_priv->data;
}

int DImgLoader::granularity(DImgLoaderObserver* const observer, int total, float progressSlice)
{
    // Splits expect total value into the chunks where checks shall occur
    // and combines this with a possible correction factor from observer.
    // Progress slice is the part of 100% concerned with the current granularity
    // (E.g. in a loop only the values from 10% to 90% are used, then progressSlice is 0.8)
    // Current default is 1/20, that is progress info every 5%
    int granularity = 0;

    if (observer)
    {
        granularity = (int)((total / (20 * progressSlice)) / observer->granularity());
    }

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

bool DImgLoader::imageHasAlpha() const
{
    return m_image->hasAlpha();
}

bool DImgLoader::imageSixteenBit() const
{
    return m_image->sixteenBit();
}

int DImgLoader::imageBitsDepth() const
{
    return m_image->bitsDepth();
}

int DImgLoader::imageBytesDepth() const
{
    return m_image->bytesDepth();
}

void DImgLoader::imageSetIccProfile(const IccProfile& profile)
{
    m_image->setIccProfile(profile);
}

QVariant DImgLoader::imageGetAttribute(const QString& key) const
{
    return m_image->attribute(key);
}

QString DImgLoader::imageGetEmbbededText(const QString& key) const
{
    return m_image->embeddedText(key);
}

void DImgLoader::imageSetAttribute(const QString& key, const QVariant& value)
{
    m_image->setAttribute(key, value);
}

QMap<QString, QString>& DImgLoader::imageEmbeddedText() const
{
    return m_image->m_priv->embeddedText;
}

void DImgLoader::imageSetEmbbededText(const QString& key, const QString& text)
{
    m_image->setEmbeddedText(key, text);
}

void DImgLoader::loadingFailed()
{
    if (m_image->m_priv->data)
    {
        delete [] m_image->m_priv->data;
    }

    m_image->m_priv->data   = 0;
    m_image->m_priv->width  = 0;
    m_image->m_priv->height = 0;
}

qint64 DImgLoader::checkAllocation(qint64 fullSize)
{
    if ((quint64)fullSize >= std::numeric_limits<size_t>::max())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Cannot allocate buffer of size" << fullSize;
        return 0;
    }

    // Do extra check if allocating serious amounts of memory.
    // At the time of writing (2011), I consider 100 MB as "serious".
    if (fullSize > (qint64)(100 * 1024 * 1024))
    {
        KMemoryInfo memory = KMemoryInfo::currentInfo();

        int res = memory.isValid();

        if (res == -1)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Not a recognized platform to get memory information";
            return -1;
        }
        else if (res == 0)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Error to get physical memory information form a recognized platform";
            return 0;
        }

        qint64 available = memory.bytes(KMemoryInfo::AvailableMemory);

        if (fullSize > available)
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Not enough memory to allocate buffer of size " << fullSize;
            qCWarning(DIGIKAM_DIMG_LOG) << "Available memory size is " << available;
            return 0;
        }
    }

    return fullSize;
}

bool DImgLoader::readMetadata(const QString& filePath, DImg::FORMAT /*ff*/)
{
    if (!((m_loadFlags & LoadMetadata) || (m_loadFlags & LoadUniqueHash) || (m_loadFlags & LoadImageHistory)))
    {
        return false;
    }

    DMetadata metaDataFromFile;

    if (!metaDataFromFile.load(filePath))
    {
        m_image->setMetadata(MetaEngineData());
        return false;
    }

    m_image->setMetadata(metaDataFromFile.data());

    if (m_loadFlags & LoadImageHistory)
    {
        DImageHistory history = DImageHistory::fromXml(metaDataFromFile.getImageHistory());
        HistoryImageId id     = createHistoryImageId(filePath, *m_image, metaDataFromFile);
        id.m_type             = HistoryImageId::Current;
        history << id;

        m_image->setImageHistory(history);
        imageSetAttribute(QLatin1String("originalImageHistory"), QVariant::fromValue(history));
    }

    return true;
}

// copied from imagescanner.cpp
static QDateTime creationDateFromFilesystem(const QFileInfo& info)
{
    // creation date is not what it seems on Unix
    QDateTime ctime = info.created();
    QDateTime mtime = info.lastModified();

    if (ctime.isNull())
    {
        return mtime;
    }

    if (mtime.isNull())
    {
        return ctime;
    }

    return qMin(ctime, mtime);
}

HistoryImageId DImgLoader::createHistoryImageId(const QString& filePath, const DImg& image, const DMetadata& metadata)
{
    QFileInfo file(filePath);

    if (!file.exists())
    {
        return HistoryImageId();
    }

    HistoryImageId id(metadata.getImageUniqueId());

    QDateTime dt = metadata.getImageDateTime();

    if (dt.isNull())
    {
        dt = creationDateFromFilesystem(file);
    }

    id.setCreationDate(dt);
    id.setFileName(file.fileName());
    id.setPath(file.path());
    id.setUniqueHash(QString::fromUtf8(uniqueHashV2(filePath, &image)), file.size());

    return id;
}

bool DImgLoader::saveMetadata(const QString& filePath)
{
    DMetadata metaDataToFile(filePath);
    metaDataToFile.setData(m_image->getMetadata());
    return metaDataToFile.applyChanges();
}

bool DImgLoader::checkExifWorkingColorSpace() const
{
    DMetadata metaData(m_image->getMetadata());
    IccProfile profile = metaData.getIccProfile();

    if (!profile.isNull())
    {
        m_image->setIccProfile(profile);
        return true;
    }

    return false;
}

void DImgLoader::storeColorProfileInMetadata()
{
    IccProfile profile = m_image->getIccProfile();

    if (profile.isNull())
    {
        return;
    }

    DMetadata metaData(m_image->getMetadata());
    metaData.setIccProfile(profile);
    m_image->setMetadata(metaData.data());
}

void DImgLoader::purgeExifWorkingColorSpace()
{
    DMetadata meta(m_image->getMetadata());
    meta.removeExifColorSpace();
    m_image->setMetadata(meta.data());
}

QByteArray DImgLoader::uniqueHashV2(const QString& filePath, const DImg* const img)
{
    QFile file(filePath);

    if (!file.open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        return QByteArray();
    }

    QCryptographicHash md5(QCryptographicHash::Md5);

    // Specified size: 100 kB; but limit to file size
    const qint64 specifiedSize = 100 * 1024; // 100 kB
    qint64 size                = qMin(file.size(), specifiedSize);

    if (size)
    {
        QScopedArrayPointer<char> databuf(new char[size]);
        int read;

        // Read first 100 kB
        if ((read = file.read(databuf.data(), size)) > 0)
        {
            md5.addData(databuf.data(), read);
        }

        // Read last 100 kB
        file.seek(file.size() - size);

        if ((read = file.read(databuf.data(), size)) > 0)
        {
            md5.addData(databuf.data(), read);
        }
    }

    QByteArray hash = md5.result().toHex();

    if (img && !hash.isNull())
    {
        const_cast<DImg*>(img)->setAttribute(QString::fromUtf8("uniqueHashV2"), hash);
    }

    return hash;
}

QByteArray DImgLoader::uniqueHash(const QString& filePath, const DImg& img, bool loadMetadata)
{
    QByteArray bv;

    if (loadMetadata)
    {
        DMetadata metaDataFromFile(filePath);
        bv = metaDataFromFile.getExifEncoded();
    }
    else
    {
        DMetadata metaDataFromImage(img.getMetadata());
        bv = metaDataFromImage.getExifEncoded();
    }

    // Create the unique ID

    QCryptographicHash md5(QCryptographicHash::Md5);

    // First, read the Exif data into the hash
    md5.addData(bv);

    // Second, read in the first 8KB of the file
    QFile qfile(filePath);

    char databuf[8192];
    QByteArray hash;

    if (qfile.open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        int readlen = 0;

        if ((readlen = qfile.read(databuf, 8192)) > 0)
        {
            QByteArray size = 0;
            md5.addData(databuf, readlen);
            md5.addData(size.setNum(qfile.size()));
            hash = md5.result().toHex();
        }
    }

    if (!hash.isNull())
    {
        const_cast<DImg&>(img).setAttribute(QLatin1String("uniqueHash"), hash);
    }

    return hash;
}

unsigned char* DImgLoader::new_failureTolerant(size_t unsecureSize)
{
    return new_failureTolerant<unsigned char>(unsecureSize);
}

unsigned char* DImgLoader::new_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    return new_failureTolerant<unsigned char>(w, h, typesPerPixel);
}

unsigned short* DImgLoader::new_short_failureTolerant(size_t unsecureSize)
{
    return new_failureTolerant<unsigned short>(unsecureSize);
}

unsigned short* DImgLoader::new_short_failureTolerant(quint64 w, quint64 h, uint typesPerPixel)
{
    return new_failureTolerant<unsigned short>(w, h, typesPerPixel);
}

}  // namespace Digikam
