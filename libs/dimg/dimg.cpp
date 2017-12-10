/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimg.h"
#include "dimg_p.h"

// C ANSI includes

#ifndef Q_OS_WIN
extern "C"
{
#endif

#include <stdint.h>

#ifndef Q_OS_WIN
#include <math.h>
}
#endif

// C++ includes

#include <cstdio>

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMap>
#include <QPaintEngine>
#include <QPainter>
#include <QPixmap>
#include <QSysInfo>
#include <QUuid>

// Local includes

#include "metaengine_rotation.h"
#include "drawdecoder.h"
#include "digikam_config.h"
#include "dimagehistory.h"
#include "pngloader.h"
#include "tiffloader.h"
#include "ppmloader.h"
#include "rawloader.h"
#include "pgfloader.h"
#include "qimageloader.h"
#include "jpegloader.h"
#include "filereadwritelock.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "dmetadata.h"
#include "dimgloaderobserver.h"
#include "randomnumbergenerator.h"
#include "digikam_debug.h"

#ifdef HAVE_JASPER
#include "jp2kloader.h"
#endif // HAVE_JASPER

typedef uint64_t ullong;
typedef int64_t  llong;

namespace Digikam
{

DImg::DImg()
    : m_priv(new Private)
{
}

DImg::DImg(const QByteArray& filePath, DImgLoaderObserver* const observer,
           const DRawDecoding& rawDecodingSettings)
    : m_priv(new Private)
{
    load(QString::fromUtf8(filePath), observer, rawDecodingSettings);
}

DImg::DImg(const QString& filePath, DImgLoaderObserver* const observer,
           const DRawDecoding& rawDecodingSettings)
    : m_priv(new Private)
{
    load(filePath, observer, rawDecodingSettings);
}

DImg::DImg(const DImg& image)
    : m_priv(image.m_priv)
{
}

DImg::DImg(uint width, uint height, bool sixteenBit, bool alpha, uchar* const data, bool copyData)
    : m_priv(new Private)
{
    putImageData(width, height, sixteenBit, alpha, data, copyData);
}

DImg::DImg(const DImg& image, int w, int h)
    : m_priv(new Private)
{
    // This private constructor creates a copy of everything except the data.
    // The image size is set to the given values and a buffer corresponding to these values is allocated.
    // This is used by copy and scale.
    copyImageData(image.m_priv);
    copyMetaData(image.m_priv);
    setImageDimension(w, h);
    allocateData();
}

DImg::DImg(const QImage& image)
    : m_priv(new Private)
{
    if (!image.isNull())
    {
        QImage target;

        if (image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)
        {
            target = image;
        }
        else
        {
            target = image.convertToFormat(QImage::Format_ARGB32);
        }

        setImageData(true, image.width(), image.height(), false, image.hasAlphaChannel());

        if (allocateData())
        {
            uint*  sptr       = reinterpret_cast<uint*>(target.bits());
            uchar* dptr       = m_priv->data;
            const uint pixels = numPixels();

            for (uint i = 0 ; i < pixels ; ++i)
            {
                dptr[0] = qBlue(*sptr);
                dptr[1] = qGreen(*sptr);
                dptr[2] = qRed(*sptr);
                dptr[3] = qAlpha(*sptr);

                dptr += 4;
                ++sptr;
            }
        }

    }
}

DImg::~DImg()
{
}

//---------------------------------------------------------------------------------------------------
// data management

DImg& DImg::operator=(const DImg& image)
{
    //qCDebug(DIGIKAM_DIMG_LOG) << "Original image: " << m_priv->imageHistory.entries().count() << " | " << &m_priv;
    //qCDebug(DIGIKAM_DIMG_LOG) << "New image: " << image.m_priv->imageHistory.entries().count() << " | " << &(image.m_priv);
    m_priv = image.m_priv;
    //qCDebug(DIGIKAM_DIMG_LOG) << "Original new image: " << m_priv->imageHistory.entries().count() << " | " << &m_priv;
    return *this;
}

bool DImg::operator==(const DImg& image) const
{
    return m_priv == image.m_priv;
}

void DImg::reset()
{
    m_priv = new Private;
}

void DImg::detach()
{
    // are we being shared?
    if (!m_priv->hasMoreReferences())
    {
        return;
    }

    DSharedDataPointer<Private> old = m_priv;

    m_priv = new Private;
    copyImageData(old);
    copyMetaData(old);

    if (old->data)
    {
        size_t size = allocateData();
        memcpy(m_priv->data, old->data, size);
    }
}

void DImg::putImageData(uint width, uint height, bool sixteenBit, bool alpha, uchar* const data, bool copyData)
{
    // set image data, metadata is untouched

    bool null = (width == 0) || (height == 0);
    // allocateData, or code below will set null to false
    setImageData(true, width, height, sixteenBit, alpha);

    // replace data
    delete [] m_priv->data;

    if (null)
    {
        // image is null - no data
        m_priv->data = 0;
    }
    else if (copyData)
    {
        size_t size = allocateData();

        if (data)
        {
            memcpy(m_priv->data, data, size);
        }
    }
    else
    {
        if (data)
        {
            m_priv->data = data;
            m_priv->null = false;
        }
        else
        {
            allocateData();
        }
    }
}

void DImg::putImageData(uchar* const data, bool copyData)
{
    if (!data)
    {
        delete [] m_priv->data;
        m_priv->data = 0;
        m_priv->null = true;
    }
    else if (copyData)
    {
        memcpy(m_priv->data, data, numBytes());
    }
    else
    {
        m_priv->data = data;
    }
}

void DImg::resetMetaData()
{
    m_priv->attributes.clear();
    m_priv->embeddedText.clear();
    m_priv->metaData = MetaEngineData();
}

uchar* DImg::stripImageData()
{
    uchar* const data  = m_priv->data;
    m_priv->data       = 0;
    m_priv->null       = true;
    return data;
}

void DImg::copyMetaData(const Private* const src)
{
    m_priv->metaData     = src->metaData;
    m_priv->attributes   = src->attributes;
    m_priv->embeddedText = src->embeddedText;
    m_priv->iccProfile   = src->iccProfile;
    m_priv->imageHistory = src->imageHistory;
    //FIXME: what about sharing and deleting lanczos_func?
}

void DImg::copyImageData(const Private* const src)
{
    setImageData(src->null, src->width, src->height, src->sixteenBit, src->alpha);
}

size_t DImg::allocateData()
{
    size_t size  = m_priv->width * m_priv->height * (m_priv->sixteenBit ? 8 : 4);
    m_priv->data = DImgLoader::new_failureTolerant(size);

    if (!m_priv->data)
    {
        m_priv->null = true;
        return 0;
    }

    m_priv->null = false;
    return size;
}

void DImg::setImageDimension(uint width, uint height)
{
    m_priv->width  = width;
    m_priv->height = height;
}

void DImg::setImageData(bool null, uint width, uint height, bool sixteenBit, bool alpha)
{
    m_priv->null       = null;
    m_priv->width      = width;
    m_priv->height     = height;
    m_priv->alpha      = alpha;
    m_priv->sixteenBit = sixteenBit;
}


//---------------------------------------------------------------------------------------------------
// load and save

bool DImg::loadImageInfo(const QString& filePath, bool loadMetadata, bool loadICCData, bool loadUniqueHash, bool loadImageHistory)
{
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadImageInfo;

    if (loadMetadata)
    {
        loadFlags |= DImgLoader::LoadMetadata;
    }

    if (loadICCData)
    {
        loadFlags |= DImgLoader::LoadICCData;
    }

    if (loadUniqueHash)
    {
        loadFlags |= DImgLoader::LoadUniqueHash;
    }

    if (loadImageHistory)
    {
        loadFlags |= DImgLoader::LoadImageHistory;
    }

    return load(filePath, loadFlags, 0, DRawDecoding());
}

bool DImg::load(const QString& filePath, DImgLoaderObserver* const observer, const DRawDecoding& rawDecodingSettings)
{
    return load(filePath, DImgLoader::LoadAll, observer, rawDecodingSettings);
}

bool DImg::load(const QString& filePath, bool loadMetadata, bool loadICCData, bool loadUniqueHash, bool loadImageHistory,
                DImgLoaderObserver* const observer, const DRawDecoding& rawDecodingSettings)
{
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadImageInfo | DImgLoader::LoadImageData;

    if (loadMetadata)
    {
        loadFlags |= DImgLoader::LoadMetadata;
    }

    if (loadICCData)
    {
        loadFlags |= DImgLoader::LoadICCData;
    }

    if (loadUniqueHash)
    {
        loadFlags |= DImgLoader::LoadUniqueHash;
    }

    if (loadImageHistory)
    {
        loadFlags |= DImgLoader::LoadImageHistory;
    }

    return load(filePath, loadFlags, observer, rawDecodingSettings);
}

bool DImg::load(const QString& filePath, int loadFlagsInt, DImgLoaderObserver* const observer,
                const DRawDecoding& rawDecodingSettings)
{
    FORMAT format = fileFormat(filePath);
    DImgLoader::LoadFlags loadFlags = (DImgLoader::LoadFlags)loadFlagsInt;

    setAttribute(QLatin1String("detectedFileFormat"), format);
    setAttribute(QLatin1String("originalFilePath"),   filePath);

    FileReadLocker lock(filePath);

    switch (format)
    {
        case (NONE):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : Unknown image format !!!";
            return false;
        }

        case (JPEG):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : JPEG file identified";
            JPEGLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }

        case (TIFF):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : TIFF file identified";
            TIFFLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }

        case (PNG):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : PNG file identified";
            PNGLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }

        case (PPM):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : PPM file identified";
            PPMLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }

        case (RAW):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : RAW file identified";
            RAWLoader loader(this, rawDecodingSettings);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                loader.postProcess(observer);
                return true;
            }

            break;
        }

#ifdef HAVE_JASPER
        case (JP2K):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : JPEG2000 file identified";
            JP2KLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }
#endif // HAVE_JASPER

        case (PGF):
        {
            qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : PGF file identified";
            PGFLoader loader(this);
            loader.setLoadFlags(loadFlags);

            if (loader.load(filePath, observer))
            {
                m_priv->null       = !loader.hasLoadedData();
                m_priv->alpha      = loader.hasAlpha();
                m_priv->sixteenBit = loader.sixteenBit();
                setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
                return true;
            }

            break;
        }

        default:
            break;
    }

    if (observer && !observer->continueQuery(0))
    {
        return false;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : QIMAGE file identified";
    QImageLoader loader(this);
    loader.setLoadFlags(loadFlags);

    if (loader.load(filePath, observer))
    {
        m_priv->null       = !loader.hasLoadedData();
        m_priv->alpha      = loader.hasAlpha();
        m_priv->sixteenBit = loader.sixteenBit();
        setAttribute(QLatin1String("isreadonly"), loader.isReadOnly());
        return true;
    }

    return false;
}

QString DImg::formatToMimeType(FORMAT frm)
{
    QString format;

    switch (frm)
    {
        case (NONE):
        {
            return format;
        }

        case (JPEG):
        {
            format = QLatin1String("JPG");
            break;
        }

        case (TIFF):
        {
            format = QLatin1String("TIF");
            break;
        }

        case (PNG):
        {
            format = QLatin1String("PNG");
            break;
        }

        case (PPM):
        {
            format = QLatin1String("PPM");
            break;
        }

        case (JP2K):
        {
            format = QLatin1String("JP2");
            break;
        }

        case (PGF):
        {
            format = QLatin1String("PGF");
            break;
        }

        default:
        {
            // For QImage based.
            break;
        }
    }

    return format;
}

bool DImg::save(const QString& filePath, FORMAT frm, DImgLoaderObserver* const observer)
{
    if (isNull())
    {
        return false;
    }

    return(save(filePath, formatToMimeType(frm), observer));
}

bool DImg::save(const QString& filePath, const QString& format, DImgLoaderObserver* const observer)
{
    qCDebug(DIGIKAM_DIMG_LOG) << "Saving to " << filePath << " with format: " << format;

    if (isNull())
    {
        return false;
    }

    if (format.isEmpty())
    {
        return false;
    }

    QString frm = format.toUpper();
    setAttribute(QLatin1String("savedFilePath"), filePath);

    FileWriteLocker lock(filePath);

    if (frm == QLatin1String("JPEG") || frm == QLatin1String("JPG") || frm == QLatin1String("JPE"))
    {
        // JPEG does not support transparency, so we shall provide an image without alpha channel.
        // This is only necessary if the image has an alpha channel, and there are actually transparent pixels
        if (hasTransparentPixels())
        {
            DImg alphaRemoved = copy();
            alphaRemoved.removeAlphaChannel();
            JPEGLoader loader(&alphaRemoved);
            setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
            return loader.save(filePath, observer);
        }
        else
        {
            JPEGLoader loader(this);
            setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
            return loader.save(filePath, observer);
        }
    }
    else if (frm == QLatin1String("PNG"))
    {
        PNGLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
    else if (frm == QLatin1String("TIFF") || frm == QLatin1String("TIF"))
    {
        TIFFLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
    else if (frm == QLatin1String("PPM"))
    {
        PPMLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }

#ifdef HAVE_JASPER
    else if (frm == QLatin1String("JP2") || frm == QLatin1String("J2K") || frm == QLatin1String("JPX") || frm == QLatin1String("JPC") || frm == QLatin1String("PGX"))
    {
        JP2KLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
#endif // HAVE_JASPER

    else if (frm == QLatin1String("PGF"))
    {
        PGFLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
    else
    {
        setAttribute(QLatin1String("format"), format);
        QImageLoader loader(this);
        setAttribute(QLatin1String("savedformat-isreadonly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }

    return false;
}

DImg::FORMAT DImg::fileFormat(const QString& filePath)
{
    if (filePath.isNull())
    {
        return NONE;
    }

    // In first we trying to check the file extension. This is mandatory because
    // some tiff files are detected like RAW files by identify method.

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "File " << filePath << " does not exist";
        return NONE;
    }

    QString rawFilesExt = QLatin1String(DRawDecoder::rawFiles());
    QString ext         = fileInfo.suffix().toUpper();

    if (!ext.isEmpty())
    {
        if (ext == QLatin1String("JPEG") || ext == QLatin1String("JPG") || ext == QLatin1String("JPE"))
        {
            return JPEG;
        }
        else if (ext == QLatin1String("PNG"))
        {
            return PNG;
        }
        else if (ext == QLatin1String("TIFF") || ext == QLatin1String("TIF"))
        {
            return TIFF;
        }
        else if (rawFilesExt.toUpper().contains(ext))
        {
            return RAW;
        }
        else if (ext == QLatin1String("JP2") || ext == QLatin1String("JPX") || // JPEG2000 file format
                 ext == QLatin1String("JPC") || ext == QLatin1String("J2K") || // JPEG2000 code stream
                 ext == QLatin1String("PGX"))                                  // JPEG2000 WM format
        {
            return JP2K;
        }
        else if (ext == QLatin1String("PGF"))
        {
            return PGF;
        }
    }

    // In second, we trying to parse file header.

    FILE* const f = fopen(QFile::encodeName(filePath).constData(), "rb");

    if (!f)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to open file " << filePath;
        return NONE;
    }

    const int headerLen = 9;

    unsigned char header[headerLen];

    if (fread(&header, headerLen, 1, f) != 1)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to read header of file " << filePath;
        fclose(f);
        return NONE;
    }

    fclose(f);

    RawInfo dcrawIdentify;
    uchar jpegID[2]    = { 0xFF, 0xD8 };
    uchar tiffBigID[2] = { 0x4D, 0x4D };
    uchar tiffLilID[2] = { 0x49, 0x49 };
    uchar pngID[8]     = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    uchar jp2ID[5]     = { 0x6A, 0x50, 0x20, 0x20, 0x0D, };
    uchar jpcID[2]     = { 0xFF, 0x4F };
    uchar pgfID[3]     = { 0x50, 0x47, 0x46 };

    if (memcmp(&header, &jpegID, 2) == 0)            // JPEG file ?
    {
        return JPEG;
    }
    else if (memcmp(&header, &pngID, 8) == 0)        // PNG file ?
    {
        return PNG;
    }
    else if (memcmp(&header[0], "P", 1)  == 0 &&
             memcmp(&header[2], "\n", 1) == 0)       // PPM 16 bits file ?
    {
        int         width, height, rgbmax;
        char        nl;
        FILE* const file = fopen(QFile::encodeName(filePath).constData(), "rb");

        // FIXME: scanf without field width limits can crash with huge input data
        if (file && fscanf(file, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) == 4)
        {
            if (rgbmax > 255)
            {
                fclose(file);
                return PPM;
            }
        }

        if (file)
            fclose(file);
    }
    else if (DRawDecoder::rawFileIdentify(dcrawIdentify, filePath)
             && dcrawIdentify.isDecodable)
    {
        // RAW File test using identify method.
        // Need to test it before TIFF because any RAW file
        // formats using TIFF header.
        return RAW;
    }
    else if (memcmp(&header, &tiffBigID, 2) == 0 ||  // TIFF file ?
             memcmp(&header, &tiffLilID, 2) == 0)
    {
        return TIFF;
    }
    else if (memcmp(&header[4], &jp2ID, 5) == 0 ||   // JPEG2000 file ?
             memcmp(&header,    &jpcID, 2) == 0)
    {
        return JP2K;
    }
    else if (memcmp(&header, &pgfID, 3) == 0)        // PGF file ?
    {
        return PNG;
    }

    // In others cases, QImage will be used to try to open file.
    return QIMAGE;
}


//---------------------------------------------------------------------------------------------------
// accessing properties


bool DImg::isNull() const
{
    return m_priv->null;
}

uint DImg::width() const
{
    return m_priv->width;
}

uint DImg::height() const
{
    return m_priv->height;
}

QSize DImg::size() const
{
    return QSize(m_priv->width, m_priv->height);
}

uchar* DImg::bits() const
{
    return m_priv->data;
}

uchar* DImg::copyBits() const
{
    uchar* const data = new uchar[numBytes()];
    memcpy(data, bits(), numBytes());
    return data;
}

uchar* DImg::scanLine(uint i) const
{
    if (i >= height())
    {
        return 0;
    }

    uchar* const data = bits() + (width() * bytesDepth() * i);
    return data;
}

bool DImg::hasAlpha() const
{
    return m_priv->alpha;
}

bool DImg::sixteenBit() const
{
    return m_priv->sixteenBit;
}

bool DImg::isReadOnly() const
{
    return attribute(QLatin1String("isreadonly")).toBool();
}

DImg::COLORMODEL DImg::originalColorModel() const
{
    if (m_priv->attributes.contains(QLatin1String("originalColorModel")))
    {
        return (COLORMODEL)m_priv->attributes.value(QLatin1String("originalColorModel")).toInt();
    }
    else
    {
        return COLORMODELUNKNOWN;
    }
}

int DImg::originalBitDepth() const
{
    return m_priv->attributes.value(QLatin1String("originalBitDepth")).toInt();
}

QSize DImg::originalSize() const
{
    if (m_priv->attributes.contains(QLatin1String("originalSize")))
    {
        QSize size = m_priv->attributes.value(QLatin1String("originalSize")).toSize();

        if (size.isValid() && !size.isNull())
        {
            return size;
        }
    }

    return size();
}

DImg::FORMAT DImg::detectedFormat() const
{
    if (m_priv->attributes.contains(QLatin1String("detectedFileFormat")))
    {
        return (FORMAT)m_priv->attributes.value(QLatin1String("detectedFileFormat")).toInt();
    }
    else
    {
        return NONE;
    }
}

QString DImg::format() const
{
    return m_priv->attributes.value(QLatin1String("format")).toString();
}

QString DImg::savedFormat() const
{
    return m_priv->attributes.value(QLatin1String("savedformat")).toString();
}

DRawDecoding DImg::rawDecodingSettings() const
{
    if (m_priv->attributes.contains(QLatin1String("rawDecodingSettings")))
    {
        return m_priv->attributes.value(QLatin1String("rawDecodingSettings")).value<DRawDecoding>();
    }
    else
    {
        return DRawDecoding();
    }
}

IccProfile DImg::getIccProfile() const
{
    return m_priv->iccProfile;
}

void DImg::setIccProfile(const IccProfile& profile)
{
    m_priv->iccProfile = profile;
}

MetaEngineData DImg::getMetadata() const
{
    return m_priv->metaData;
}

void DImg::setMetadata(const MetaEngineData& data)
{
    m_priv->metaData = data;
}

uint DImg::numBytes() const
{
    return (width() * height() * bytesDepth());
}

uint DImg::numPixels() const
{
    return (width() * height());
}

int DImg::bytesDepth() const
{
    if (m_priv->sixteenBit)
    {
        return 8;
    }

    return 4;
}

int DImg::bitsDepth() const
{
    if (m_priv->sixteenBit)
    {
        return 16;
    }

    return 8;
}

void DImg::setAttribute(const QString& key, const QVariant& value)
{
    m_priv->attributes.insert(key, value);
}

QVariant DImg::attribute(const QString& key) const
{
    if (m_priv->attributes.contains(key))
    {
        return m_priv->attributes[key];
    }

    return QVariant();
}

bool DImg::hasAttribute(const QString& key) const
{
    return m_priv->attributes.contains(key);
}

void DImg::removeAttribute(const QString& key)
{
    m_priv->attributes.remove(key);
}

void DImg::setEmbeddedText(const QString& key, const QString& text)
{
    m_priv->embeddedText.insert(key, text);
}

QString DImg::embeddedText(const QString& key) const
{
    if (m_priv->embeddedText.contains(key))
    {
        return m_priv->embeddedText[key];
    }

    return QString();
}

void DImg::imageSavedAs(const QString& savePath)
{
    setAttribute(QLatin1String("savedFilePath"), savePath);
    addAsReferredImage(savePath);
}

QString DImg::originalFilePath() const
{
    return attribute(QLatin1String("originalFilePath")).toString();
}

QString DImg::lastSavedFilePath() const
{
    return attribute(QLatin1String("savedFilePath")).toString();
}

QVariant DImg::fileOriginData() const
{
    QVariantMap map;

    foreach(const QString& key, m_priv->fileOriginAttributes())
    {
        QVariant attr = attribute(key);

        if (!attr.isNull())
        {
            map.insert(key, attr);
        }
    }

    return map;
}

QVariant DImg::lastSavedFileOriginData() const
{
    QVariantMap map;
    QVariant savedformat = attribute(QLatin1String("savedformat"));

    if (!savedformat.isNull())
    {
        map.insert(QLatin1String("format"), savedformat);
    }

    QVariant readonly = attribute(QLatin1String("savedformat-isreadonly"));

    if (!readonly.isNull())
    {
        map.insert(QLatin1String("isreadonly"), readonly);
    }

    QVariant filePath = attribute(QLatin1String("savedFilePath"));

    if (!filePath.isNull())
    {
        map.insert(QLatin1String("originalFilePath"), filePath);
    }

    DImageHistory history = m_priv->imageHistory;

    if (!history.isEmpty())
    {
        history.adjustReferredImages();

        if (!history.entries().last().referredImages.isEmpty())
        {
            history.entries().last().referredImages.last().setType(HistoryImageId::Current);
        }

        map.insert(QLatin1String("originalImageHistory"), QVariant::fromValue(history));
    }

    return map;
}

void DImg::setFileOriginData(const QVariant& data)
{
    QVariantMap map = data.toMap();

    foreach(const QString& key, m_priv->fileOriginAttributes())
    {
        removeAttribute(key);
        QVariant attr = map.value(key);

        if (!attr.isNull())
        {
            setAttribute(key, attr);
        }
    }
}

void DImg::switchOriginToLastSaved()
{
    setFileOriginData(lastSavedFileOriginData());
}

DColor DImg::getPixelColor(uint x, uint y) const
{
    if (m_priv->null || x >= m_priv->width || y >= m_priv->height)
    {
        return DColor();
    }

    int depth         = bytesDepth();
    uchar* const data = m_priv->data + x * depth + (m_priv->width * y * depth);

    return (DColor(data, m_priv->sixteenBit));
}

void DImg::prepareSubPixelAccess()
{
    if (m_priv->lanczos_func)
    {
        return;
    }

    /* Precompute the Lanczos kernel */
    LANCZOS_DATA_TYPE* lanczos_func = new LANCZOS_DATA_TYPE[LANCZOS_SUPPORT * LANCZOS_SUPPORT * LANCZOS_TABLE_RES];

    for (int i = 0; i < LANCZOS_SUPPORT * LANCZOS_SUPPORT * LANCZOS_TABLE_RES; ++i)
    {
        if (i == 0)
        {
            lanczos_func [i] = LANCZOS_DATA_ONE;
        }
        else
        {
            float d          = sqrt(((float)i) / LANCZOS_TABLE_RES);
            lanczos_func [i] = (LANCZOS_DATA_TYPE)((LANCZOS_DATA_ONE * LANCZOS_SUPPORT *
                                                    sin(M_PI * d) * sin((M_PI / LANCZOS_SUPPORT) * d)) /
                                                   (M_PI * M_PI * d * d));
        }
    }

    m_priv->lanczos_func = lanczos_func;
}

#ifdef LANCZOS_DATA_FLOAT

static inline int normalizeAndClamp(float norm, int sum, int max)
{
    int r = 0;

    if (norm != 0.0)
    {
        r = sum / norm;
    }

    if (r < 0)
    {
        r = 0;
    }
    else if (r > max)
    {
        r = max;
    }

    return r;
}

#else /* LANCZOS_DATA_FLOAT */

static inline int normalizeAndClamp(int norm, int sum, int max)
{
    int r = 0;

    if (norm != 0)
    {
        r = sum / norm;
    }

    if (r < 0)
    {
        r = 0;
    }
    else if (r > max)
    {
        r = max;
    }

    return r;
}

#endif /* LANCZOS_DATA_FLOAT */

DColor DImg::getSubPixelColor(float x, float y) const
{
    if (isNull())
    {
        return DColor();
    }

    const LANCZOS_DATA_TYPE* lanczos_func = m_priv->lanczos_func;

    if (lanczos_func == 0)
    {
        return DColor();
    }

    x = qBound(0.0f, x, (float)width()  - 1);
    y = qBound(0.0f, y, (float)height() - 1);

    Digikam::DColor col(0, 0, 0, 0xFFFF, sixteenBit());

#ifdef LANCZOS_DATA_FLOAT

    float xs = ::ceilf(x)  - LANCZOS_SUPPORT;
    float xe = ::floorf(x) + LANCZOS_SUPPORT;
    float ys = ::ceilf(y)  - LANCZOS_SUPPORT;
    float ye = ::floorf(y) + LANCZOS_SUPPORT;

    if (xs >= 0 && ys >= 0 && xe < width() && ye < height())
    {
        float norm = 0.0;
        float sumR = 0.0;
        float sumG = 0.0;
        float sumB = 0.0;
        float _dx  = x - xs;
        float dy   = y - ys;

        for (; ys <= ye; ys += 1.0, dy -= 1.0)
        {
            float xc, dx = _dx;

            for (xc = xs; xc <= xe; xc += 1.0, dx -= 1.0)
            {
                uchar* const data = bits() + (int)(xs * bytesDepth()) + (int)(width() * ys * bytesDepth());
                DColor src        = DColor(data, sixteenBit());
                float d           = dx * dx + dy * dy;

                if (d >= LANCZOS_SUPPORT * LANCZOS_SUPPORT)
                {
                    continue;
                }

                d     = lanczos_func [(int)(d * LANCZOS_TABLE_RES)];
                norm += d;
                sumR += d * src.red();
                sumG += d * src.green();
                sumB += d * src.blue();
            }
        }

        int max = sixteenBit() ? 65535 : 255;
        col.setRed(normalizeAndClamp(norm, sumR, max));
        col.setGreen(normalizeAndClamp(norm, sumG, max));
        col.setBlue(normalizeAndClamp(norm, sumB, max));
    }

#else /* LANCZOS_DATA_FLOAT */

    /* Do it in integer arithmetic, it's faster */
    int xx   = (int)x;
    int yy   = (int)y;
    int xs   = xx + 1 - LANCZOS_SUPPORT;
    int xe   = xx     + LANCZOS_SUPPORT;
    int ys   = yy + 1 - LANCZOS_SUPPORT;
    int ye   = yy     + LANCZOS_SUPPORT;
    int norm = 0;
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    int _dx  = (int)(x * 4096.0) - (xs << 12);
    int dy   = (int)(y * 4096.0) - (ys << 12);

    for (; ys <= ye; ++ys, dy -= 4096)
    {
        int xc, dx = _dx;

        for (xc = xs; xc <= xe; ++xc, dx -= 4096)
        {
            DColor src(0, 0, 0, 0xFFFF, sixteenBit());

            if (xc >= 0 && ys >= 0 && xc < (int)width() && ys < (int)height())
            {
                uchar* const data = bits() + xc * bytesDepth() + width() * ys * bytesDepth();
                src.setColor(data, sixteenBit());
            }

            int d = (dx * dx + dy * dy) >> 12;

            if (d >= 4096 * LANCZOS_SUPPORT * LANCZOS_SUPPORT)
            {
                continue;
            }

            d     = lanczos_func [(d * LANCZOS_TABLE_RES) >> 12];
            norm += d;
            sumR += d * src.red();
            sumG += d * src.green();
            sumB += d * src.blue();
        }
    }

    int max = sixteenBit() ? 65535 : 255;
    col.setRed(normalizeAndClamp(norm, sumR, max));
    col.setGreen(normalizeAndClamp(norm, sumG, max));
    col.setBlue(normalizeAndClamp(norm, sumB, max));

#endif /* LANCZOS_DATA_FLOAT */

    return col;
}

DColor DImg::getSubPixelColorFast(float x, float y) const
{
    if (isNull())
    {
        return DColor();
    }

    x = qBound(0.0f, x, (float)width()  - 1);
    y = qBound(0.0f, y, (float)height() - 1);

    int xx      = (int)x;
    int yy      = (int)y;
    float d_x   = x - (int)x;
    float d_y   = y - (int)y;
    uchar* data = 0;

    DColor d00, d01, d10, d11;
    DColor col;

    data = bits() + xx * bytesDepth() + yy * width() * bytesDepth();
    d00.setColor(data, sixteenBit());

    if ((xx + 1) < (int)width())
    {
        data = bits() + (xx + 1) * bytesDepth() + yy * width() * bytesDepth();
        d10.setColor(data, sixteenBit());
    }

    if ((yy + 1) < (int)height())
    {
        data = bits() + xx * bytesDepth() + (yy + 1) * width() * bytesDepth();
        d01.setColor(data, sixteenBit());
    }

    if ((xx + 1) < (int)width() && (yy + 1) < (int)height())
    {
        data = bits() + (xx + 1) * bytesDepth() + (yy + 1) * width() * bytesDepth();
        d11.setColor(data, sixteenBit());
    }

    d00.multiply(1.0 - d_x);
    d00.multiply(1.0 - d_y);

    d10.multiply(d_x);
    d10.multiply(1.0 - d_y);

    d01.multiply(1.0 - d_x);
    d01.multiply(d_y);

    d11.multiply(d_x);
    d11.multiply(d_y);

    col.blendAdd(d00);
    col.blendAdd(d10);
    col.blendAdd(d01);
    col.blendAdd(d11);

    if (sixteenBit())
    {
        col.blendClamp16();
    }
    else
    {
        col.blendClamp8();
    }

    return col;
}

void DImg::setPixelColor(uint x, uint y, const DColor& color)
{
    if (m_priv->null || x >= m_priv->width || y >= m_priv->height)
    {
        return;
    }

    if (color.sixteenBit() != m_priv->sixteenBit)
    {
        return;
    }

    int depth         = bytesDepth();
    uchar* const data = m_priv->data + x * depth + (m_priv->width * y * depth);
    color.setPixel(data);
}

bool DImg::hasTransparentPixels() const
{
    if (m_priv->null || !m_priv->alpha)
    {
        return false;
    }

    const uint w = m_priv->width;
    const uint h = m_priv->height;

    if (!m_priv->sixteenBit)     // 8 bits image.
    {
        uchar* srcPtr = m_priv->data;

        for (uint j = 0; j < h; ++j)
        {
            for (uint i = 0; i < w; ++i)
            {
                if (srcPtr[3] != 0xFF)
                {
                    return true;
                }

                srcPtr += 4;
            }
        }
    }
    else
    {
        unsigned short* srcPtr = reinterpret_cast<unsigned short*>(m_priv->data);

        for (uint j = 0; j < h; ++j)
        {
            for (uint i = 0; i < w; ++i)
            {
                if (srcPtr[3] != 0xFFFF)
                {
                    return true;
                }

                srcPtr += 4;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------
// copying operations

DImg DImg::copy() const
{
    DImg img(*this);
    img.detach();
    return img;
}

DImg DImg::copyImageData() const
{
    DImg img(width(), height(), sixteenBit(), hasAlpha(), bits(), true);
    return img;
}

DImg DImg::copyMetaData() const
{
    DImg img;
    // copy width, height, alpha, sixteenBit, null
    img.copyImageData(m_priv);
    // deeply copy metadata
    img.copyMetaData(m_priv);
    // set image to null
    img.m_priv->null = true;
    return img;
}

DImg DImg::copy(const QRect& rect) const
{
    return copy(rect.x(), rect.y(), rect.width(), rect.height());
}

DImg DImg::copy(const QRectF& rel) const
{
    if (isNull() || !rel.isValid())
    {
        return DImg();
    }

    return copy(QRectF(rel.x()      * m_priv->width,
                       rel.y()      * m_priv->height,
                       rel.width()  * m_priv->width,
                       rel.height() * m_priv->height)
                .toRect());
}

DImg DImg::copy(int x, int y, int w, int h) const
{
    if (isNull() || w <= 0 || h <= 0)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << " : return null image! ("
                                  << isNull() << ", " << w
                                  << ", " << h << ")";
        return DImg();
    }

    if (!Private::clipped(x, y, w, h, m_priv->width, m_priv->height))
    {
        return DImg();
    }

    DImg image(*this, w, h);
    image.bitBltImage(this, x, y, w, h, 0, 0);

    return image;
}

//---------------------------------------------------------------------------------------------------
// bitwise operations

void DImg::bitBltImage(const DImg* const src, int dx, int dy)
{
    bitBltImage(src, 0, 0, src->width(), src->height(), dx, dy);
}

void DImg::bitBltImage(const DImg* const src, int sx, int sy, int dx, int dy)
{
    bitBltImage(src, sx, sy, src->width() - sx, src->height() - sy, dx, dy);
}

void DImg::bitBltImage(const DImg* const src, int sx, int sy, int w, int h, int dx, int dy)
{
    if (isNull())
    {
        return;
    }

    if (src->sixteenBit() != sixteenBit())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blitting from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    if (w == -1 && h == -1)
    {
        w = src->width();
        h = src->height();
    }

    bitBlt(src->bits(), bits(), sx, sy, w, h, dx, dy,
           src->width(), src->height(), width(), height(), sixteenBit(), src->bytesDepth(), bytesDepth());
}

void DImg::bitBltImage(const uchar* const src, int sx, int sy, int w, int h, int dx, int dy,
                       uint swidth, uint sheight, int sdepth)
{
    if (isNull())
    {
        return;
    }

    if (bytesDepth() != sdepth)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blitting from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    if (w == -1 && h == -1)
    {
        w = swidth;
        h = sheight;
    }

    bitBlt(src, bits(), sx, sy, w, h, dx, dy, swidth, sheight, width(), height(), sixteenBit(), sdepth, bytesDepth());
}

bool DImg::normalizeRegionArguments(int& sx, int& sy, int& w, int& h, int& dx, int& dy,
                                    uint swidth, uint sheight, uint dwidth, uint dheight)
{
    if (sx < 0)
    {
        // sx is negative, so + is - and - is +
        dx -= sx;
        w  += sx;
        sx = 0;
    }

    if (sy < 0)
    {
        dy -= sy;
        h  += sy;
        sy = 0;
    }

    if (dx < 0)
    {
        sx -= dx;
        w  += dx;
        dx = 0;
    }

    if (dy < 0)
    {
        sy -= dy;
        h  += dy;
        dy = 0;
    }

    if (sx + w > (int)swidth)
    {
        w = swidth - sx;
    }

    if (sy + h > (int)sheight)
    {
        h = sheight - sy;
    }

    if (dx + w > (int)dwidth)
    {
        w = dwidth - dx;
    }

    if (dy + h > (int)dheight)
    {
        h = dheight - dy;
    }

    // Nothing left to copy
    if (w <= 0 || h <= 0)
    {
        return false;
    }

    return true;
}

void DImg::bitBlt(const uchar* const src, uchar* const dest,
                  int sx, int sy, int w, int h, int dx, int dy,
                  uint swidth, uint sheight, uint dwidth, uint dheight,
                  bool /*sixteenBit*/, int sdepth, int ddepth)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
    {
        return;
    }

    // Same pixels
    if (src == dest && dx == sx && dy == sy)
    {
        return;
    }

    const uchar* sptr  = 0;
    uchar* dptr        = 0;
    uint   slinelength = swidth * sdepth;
    uint   dlinelength = dwidth * ddepth;
    int scurY          = sy;
    int dcurY          = dy;
    int sdepthlength   = w * sdepth;

    for (int j = 0 ; j < h ; ++j, ++scurY, ++dcurY)
    {
        sptr  = &src [ scurY * slinelength ] + sx * sdepth;
        dptr  = &dest[ dcurY * dlinelength ] + dx * ddepth;

        // plain and simple bitBlt
        for (int i = 0; i < sdepthlength ; ++i, ++sptr, ++dptr)
        {
            *dptr = *sptr;
        }
    }
}


void DImg::bitBlendImage(DColorComposer* const composer, const DImg* const src,
                         int sx, int sy, int w, int h, int dx, int dy,
                         DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (isNull())
    {
        return;
    }

    if (src->sixteenBit() != sixteenBit())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blending from 8-bit to 16-bit or vice versa is not supported";
        return;
    }

    bitBlend(composer, src->bits(), bits(), sx, sy, w, h, dx, dy,
             src->width(), src->height(), width(), height(), sixteenBit(),
             src->bytesDepth(), bytesDepth(), multiplicationFlags);
}

void DImg::bitBlend(DColorComposer* const composer, uchar* const src, uchar* const dest,
                    int sx, int sy, int w, int h, int dx, int dy,
                    uint swidth, uint sheight, uint dwidth, uint dheight,
                    bool sixteenBit, int sdepth, int ddepth,
                    DColorComposer::MultiplicationFlags multiplicationFlags)
{
    // Normalize
    if (!normalizeRegionArguments(sx, sy, w, h, dx, dy, swidth, sheight, dwidth, dheight))
    {
        return;
    }

    uchar* sptr      = 0;
    uchar* dptr      = 0;
    uint slinelength = swidth * sdepth;
    uint dlinelength = dwidth * ddepth;
    int scurY        = sy;
    int dcurY        = dy;

    for (int j = 0 ; j < h ; ++j, ++scurY, ++dcurY)
    {
        sptr = &src [ scurY * slinelength ] + sx * sdepth;
        dptr = &dest[ dcurY * dlinelength ] + dx * ddepth;

        // blend src and destination
        for (int i = 0 ; i < w ; ++i, sptr += sdepth, dptr += ddepth)
        {
            DColor src(sptr, sixteenBit);
            DColor dst(dptr, sixteenBit);

            // blend colors
            composer->compose(dst, src, multiplicationFlags);

            dst.setPixel(dptr);
        }
    }
}

void DImg::bitBlendImageOnColor(const DColor& color)
{
    bitBlendImageOnColor(color, 0, 0, width(), height());
}

void DImg::bitBlendImageOnColor(const DColor& color, int x, int y, int w, int h)
{
    // get composer for compositing rule
    DColorComposer* const composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
    // flags would be MultiplicationFlagsDImg for anything but PorterDuffNone
    bitBlendImageOnColor(composer, color, x, y, w, h, DColorComposer::NoMultiplication);

    delete composer;
}

void DImg::bitBlendImageOnColor(DColorComposer* const composer, const DColor& color,
                                int x, int y, int w, int h,
                                DColorComposer::MultiplicationFlags multiplicationFlags)
{
    if (isNull())
    {
        return;
    }

    DColor c = color;

    if (sixteenBit())
    {
        c.convertToSixteenBit();
    }
    else
    {
        c.convertToEightBit();
    }

    bitBlendOnColor(composer, c, bits(), x, y, w, h,
                    width(), height(), sixteenBit(), bytesDepth(), multiplicationFlags);
}

void DImg::bitBlendOnColor(DColorComposer* const composer, const DColor& color,
                           uchar* const data, int x, int y, int w, int h,
                           uint width, uint height, bool sixteenBit, int depth,
                           DColorComposer::MultiplicationFlags multiplicationFlags)
{
    // Normalize
    if (!normalizeRegionArguments(x, y, w, h, x, y, width, height, width, height))
    {
        return;
    }

    uchar* ptr      = 0;
    uint linelength = width * depth;
    int curY        = y;

    for (int j = 0 ; j < h ; ++j, ++curY)
    {
        ptr = &data[ curY * linelength ] + x * depth;

        // blend src and destination
        for (int i = 0 ; i < w ; ++i, ptr += depth)
        {
            DColor src(ptr, sixteenBit);
            DColor dst(color);

            // blend colors
            composer->compose(dst, src, multiplicationFlags);

            dst.setPixel(ptr);
        }
    }
}

//---------------------------------------------------------------------------------------------------
// QImage / QPixmap access

QImage DImg::copyQImage() const
{
    if (isNull())
    {
        return QImage();
    }

    if (sixteenBit())
    {
        DImg img(*this);
        img.detach();
        img.convertDepth(32);
        return img.copyQImage();
    }

    QImage img(width(), height(), QImage::Format_ARGB32);

    if (img.isNull())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Failed to allocate memory to copy DImg of size" << size() << "to QImage";
        return QImage();
    }

    uchar* sptr = bits();
    uint*  dptr = reinterpret_cast<uint*>(img.bits());

    for (uint i = 0; i < width()*height(); ++i)
    {
        *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
        sptr += 4;
    }

    // NOTE: Qt4 do not provide anymore QImage::setAlphaChannel() because
    // alpha channel is auto-detected during QImage->QPixmap conversion

    return img;
}

QImage DImg::copyQImage(const QRect& rect) const
{
    return (copyQImage(rect.x(), rect.y(), rect.width(), rect.height()));
}

QImage DImg::copyQImage(const QRectF& rel) const
{
    if (isNull() || !rel.isValid())
    {
        return QImage();
    }

    return copyQImage(QRectF(rel.x()      * m_priv->width,
                             rel.y()      * m_priv->height,
                             rel.width()  * m_priv->width,
                             rel.height() * m_priv->height)
                      .toRect());
}

QImage DImg::copyQImage(int x, int y, int w, int h) const
{
    if (isNull())
    {
        return QImage();
    }

    DImg img = copy(x, y, w, h);

    if (img.sixteenBit())
    {
        img.convertDepth(32);
    }

    return img.copyQImage();
}

// --------------------------------------------------------------------------------------

class PixmapPaintEngineDetector
{
public:

    PixmapPaintEngineDetector()
        : m_isRaster(detectRasterFromPixmap())
    {
    }

    bool isRaster() const
    {
        return m_isRaster;
    }

private:

    static bool detectRasterFromPixmap()
    {
        QPixmap pix(1, 1);
        QPainter p(&pix);
        return (p.paintEngine() && p.paintEngine()->type() == QPaintEngine::Raster);
    }

    const bool m_isRaster;
};

Q_GLOBAL_STATIC(PixmapPaintEngineDetector, pixmapPaintEngineDetector)

// --------------------------------------------------------------------------------------

QPixmap DImg::convertToPixmap() const
{
    if (isNull())
    {
        return QPixmap();
    }

    if (sixteenBit())
    {
        // make fastaaaa...
        return QPixmap::fromImage(copyQImage(0, 0, width(), height()));
    }

    if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
    {
        QImage img(width(), height(), hasAlpha() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        uchar* sptr = bits();
        uint*  dptr = reinterpret_cast<uint*>(img.bits());
        uint dim    = width() * height();

        for (uint i = 0; i < dim; ++i)
        {
            *dptr++ = qRgba(sptr[2], sptr[1], sptr[0], sptr[3]);
            sptr += 4;
        }

        // alpha channel is auto-detected during QImage->QPixmap conversion
        return QPixmap::fromImage(img);
    }
    else
    {
        // This is a temporary image operating on the DImg buffer
        QImage img(bits(), width(), height(), hasAlpha() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        // For paint engines which base the QPixmap internally on a QImage, we must use a persistent QImage
        if (pixmapPaintEngineDetector->isRaster())
        {
            img = img.copy();
        }

        // alpha channel is auto-detected during QImage->QPixmap conversion
        return QPixmap::fromImage(img);
    }
}

QPixmap DImg::convertToPixmap(IccTransform& monitorICCtrans) const
{
    if (isNull())
    {
        return QPixmap();
    }

    if (monitorICCtrans.outputProfile().isNull())
    {
        return convertToPixmap();
    }

    DImg img = copy();
    monitorICCtrans.apply(img);

    return (img.convertToPixmap());
}

QImage DImg::pureColorMask(ExposureSettingsContainer* const expoSettings) const
{
    if (isNull() || (!expoSettings->underExposureIndicator && !expoSettings->overExposureIndicator))
    {
        return QImage();
    }

    QImage img(size(), QImage::Format_ARGB32);
    img.fill(0x00000000);      // Full transparent.

    // NOTE: Qt4 do not provide anymore QImage::setAlphaChannel() because
    // alpha channel is auto-detected during QImage->QPixmap conversion

    uchar* bits = img.bits();

    // NOTE: Using DImgScale before to compute Mask clamp to 65534 | 254. Why ?

    int    max  = lround(sixteenBit() ? 65535.0 - (65535.0 * expoSettings->overExposurePercent  / 100.0)
                         : 255.0   - (255.0   * expoSettings->overExposurePercent  / 100.0));
    int    min  = lround(sixteenBit() ? 0.0     + (65535.0 * expoSettings->underExposurePercent / 100.0)
                         : 0.0     + (255.0   * expoSettings->underExposurePercent / 100.0));

    // --------------------------------------------------------

    // caching
    int u_red   = expoSettings->underExposureColor.red();
    int u_green = expoSettings->underExposureColor.green();
    int u_blue  = expoSettings->underExposureColor.blue();

    int o_red   = expoSettings->overExposureColor.red();
    int o_green = expoSettings->overExposureColor.green();
    int o_blue  = expoSettings->overExposureColor.blue();

    bool under  = expoSettings->underExposureIndicator;
    bool over   = expoSettings->overExposureIndicator;
    bool pure   = expoSettings->exposureIndicatorMode;

    // --------------------------------------------------------

    uint   dim   = m_priv->width * m_priv->height;
    uchar* dptr  = bits;
    int    s_blue, s_green, s_red;
    bool   match = false;

    if (sixteenBit())
    {
        unsigned short* sptr = reinterpret_cast<unsigned short*>(m_priv->data);

        for (uint i = 0; i < dim; ++i)
        {
            s_blue  = *sptr++;
            s_green = *sptr++;
            s_red   = *sptr++;
            sptr++;
            match = pure ? (s_red <= min) && (s_green <= min) && (s_blue <= min)
                         : (s_red <= min) || (s_green <= min) || (s_blue <= min);

            if (under && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = u_red;
                    dptr[2] = u_green;
                    dptr[3] = u_blue;
                }
                else
                {
                    dptr[0] = u_blue;
                    dptr[1] = u_green;
                    dptr[2] = u_red;
                    dptr[3] = 0xFF;
                }
            }

            match = pure ? (s_red >= max) && (s_green >= max) && (s_blue >= max)
                         : (s_red >= max) || (s_green >= max) || (s_blue >= max);

            if (over && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = o_red;
                    dptr[2] = o_green;
                    dptr[3] = o_blue;
                }
                else
                {
                    dptr[0] = o_blue;
                    dptr[1] = o_green;
                    dptr[2] = o_red;
                    dptr[3] = 0xFF;
                }
            }

            dptr += 4;
        }
    }
    else
    {
        uchar* sptr = m_priv->data;

        for (uint i = 0; i < dim; ++i)
        {
            s_blue  = *sptr++;
            s_green = *sptr++;
            s_red   = *sptr++;
            sptr++;
            match = pure ? (s_red <= min) && (s_green <= min) && (s_blue <= min)
                         : (s_red <= min) || (s_green <= min) || (s_blue <= min);

            if (under && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = u_red;
                    dptr[2] = u_green;
                    dptr[3] = u_blue;
                }
                else
                {
                    dptr[0] = u_blue;
                    dptr[1] = u_green;
                    dptr[2] = u_red;
                    dptr[3] = 0xFF;
                }
            }

            match = pure ? (s_red >= max) && (s_green >= max) && (s_blue >= max)
                         : (s_red >= max) || (s_green >= max) || (s_blue >= max);

            if (over && match)
            {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                {
                    dptr[0] = 0xFF;
                    dptr[1] = o_red;
                    dptr[2] = o_green;
                    dptr[3] = o_blue;
                }
                else
                {
                    dptr[0] = o_blue;
                    dptr[1] = o_green;
                    dptr[2] = o_red;
                    dptr[3] = 0xFF;
                }
            }

            dptr += 4;
        }
    }

    return img;
}

//---------------------------------------------------------------------------------------------------
// basic imaging operations


void DImg::crop(const QRect& rect)
{
    crop(rect.x(), rect.y(), rect.width(), rect.height());
}

void DImg::crop(int x, int y, int w, int h)
{
    if (isNull() || w <= 0 || h <= 0)
    {
        return;
    }

    uint  oldw = width();
    uint  oldh = height();
    QScopedArrayPointer<uchar> old(stripImageData());

    // set new image data, bits(), width(), height() change
    setImageDimension(w, h);
    allocateData();

    // copy image region (x|y), wxh, from old data to point (0|0) of new data
    bitBlt(old.data(), bits(), x, y, w, h, 0, 0, oldw, oldh, width(), height(), sixteenBit(), bytesDepth(), bytesDepth());
}

void DImg::resize(int w, int h)
{
    if (isNull() || w <= 0 || h <= 0)
    {
        return;
    }

    DImg image = smoothScale(w, h);

    delete [] m_priv->data;
    m_priv->data = image.stripImageData();
    setImageDimension(w, h);
}

void DImg::removeAlphaChannel()
{
    removeAlphaChannel(DColor(0xFF, 0xFF, 0xFF, 0xFF, false));
}

void DImg::removeAlphaChannel(const DColor& destColor)
{
    if (isNull() || !hasAlpha())
    {
        return;
    }

    bitBlendImageOnColor(destColor);
    // unsure if alpha value is always 0xFF now
    m_priv->alpha = false;
}

void DImg::rotate(ANGLE angle)
{
    if (isNull())
    {
        return;
    }

    bool switchDims = false;

    switch (angle)
    {
        case (ROT90):
        {
            uint w  = height();
            uint h  = width();

            if (sixteenBit())
            {
                ullong* newData = DImgLoader::new_failureTolerant<ullong>(w * h);
                ullong* from    = reinterpret_cast<ullong*>(m_priv->data);
                ullong* to      = 0;

                for (int y = w - 1; y >= 0; --y)
                {
                    to = newData + y;

                    for (uint x = 0; x < h; ++x)
                    {
                        *to = *from++;
                        to += w;
                    }
                }

                switchDims = true;

                delete [] m_priv->data;
                m_priv->data = (uchar*)newData;
            }
            else
            {
                uint* newData = DImgLoader::new_failureTolerant<uint>(w * h);
                uint* from    = reinterpret_cast<uint*>(m_priv->data);
                uint* to      = 0;

                for (int y = w - 1; y >= 0; --y)
                {
                    to = newData + y;

                    for (uint x = 0; x < h; ++x)
                    {
                        *to = *from++;
                        to += w;
                    }
                }

                switchDims = true;

                delete [] m_priv->data;
                m_priv->data = (uchar*)newData;
            }

            break;
        }

        case (ROT180):
        {
            uint w          = width();
            uint h          = height();
            int middle_line = -1;

            if (h % 2)
            {
                middle_line = h / 2;
            }

            if (sixteenBit())
            {
                ullong* line1 = 0;
                ullong* line2 = 0;
                ullong* data  = reinterpret_cast<ullong*>(bits());
                ullong  tmp;

                // can be done inplace
                uint ymax = (h + 1) / 2;

                for (uint y = 0; y < ymax; ++y)
                {
                    line1 = data + y * w;
                    line2 = data + (h - y) * w - 1;

                    for (uint x = 0; x < w; ++x)
                    {
                        tmp    = *line1;
                        *line1 = *line2;
                        *line2 = tmp;

                        ++line1;
                        --line2;

                        if ((int)y == middle_line && x * 2 >= w)
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                uint* line1 = 0;
                uint* line2 = 0;
                uint* data  = reinterpret_cast<uint*>(bits());
                uint  tmp;

                // can be done inplace
                uint ymax = (h + 1) / 2;

                for (uint y = 0; y < ymax; ++y)
                {
                    line1 = data + y * w;
                    line2 = data + (h - y) * w - 1;

                    for (uint x = 0; x < w; ++x)
                    {
                        tmp    = *line1;
                        *line1 = *line2;
                        *line2 = tmp;

                        ++line1;
                        --line2;

                        if ((int)y == middle_line && x * 2 >= w)
                        {
                            break;
                        }
                    }
                }
            }

            break;
        }

        case (ROT270):
        {
            uint w  = height();
            uint h  = width();

            if (sixteenBit())
            {
                ullong* newData = DImgLoader::new_failureTolerant<ullong>(w * h);
                ullong* from    = reinterpret_cast<ullong*>(m_priv->data);
                ullong* to      = 0;

                for (uint y = 0; y < w; ++y)
                {
                    to = newData + y + w * (h - 1);

                    for (uint x = 0; x < h; ++x)
                    {
                        *to = *from++;
                        to -= w;
                    }
                }

                switchDims = true;

                delete [] m_priv->data;
                m_priv->data = (uchar*)newData;
            }
            else
            {
                uint* newData = DImgLoader::new_failureTolerant<uint>(w * h);
                uint* from    = reinterpret_cast<uint*>(m_priv->data);
                uint* to      = 0;

                for (uint y = 0; y < w; ++y)
                {
                    to = newData + y + w * (h - 1);

                    for (uint x = 0; x < h; ++x)
                    {
                        *to = *from++;
                        to -= w;
                    }
                }

                switchDims = true;

                delete [] m_priv->data;
                m_priv->data = (uchar*)newData;
            }

            break;
        }

        default:
            break;
    }

    if (switchDims)
    {
        setImageDimension(height(), width());
        QMap<QString, QVariant>::iterator it = m_priv->attributes.find(QLatin1String("originalSize"));

        if (it != m_priv->attributes.end())
        {
            QSize size = it.value().toSize();
            it.value() = QSize(size.height(), size.width());
        }
    }
}

// 15-11-2005: This method have been tested indeep with valgrind by Gilles.

void DImg::flip(FLIP direction)
{
    if (isNull())
    {
        return;
    }

    switch (direction)
    {
        case (HORIZONTAL):
        {
            uint w  = width();
            uint h  = height();

            if (sixteenBit())
            {
                unsigned short  tmp[4];
                unsigned short* beg  = 0;
                unsigned short* end  = 0;
                unsigned short* data = reinterpret_cast<unsigned short*>(bits());

                // can be done inplace
                uint wHalf = (w / 2);

                for (uint y = 0 ; y < h ; ++y)
                {
                    beg = data + y * w * 4;
                    end = beg  + (w - 1) * 4;

                    for (uint x = 0 ; x < wHalf ; ++x)
                    {
                        memcpy(&tmp, beg, 8);
                        memcpy(beg, end, 8);
                        memcpy(end, &tmp, 8);

                        beg += 4;
                        end -= 4;
                    }
                }
            }
            else
            {
                uchar  tmp[4];
                uchar* beg  = 0;
                uchar* end  = 0;
                uchar* data = bits();

                // can be done inplace
                uint wHalf = (w / 2);

                for (uint y = 0 ; y < h ; ++y)
                {
                    beg = data + y * w * 4;
                    end = beg  + (w - 1) * 4;

                    for (uint x = 0 ; x < wHalf ; ++x)
                    {
                        memcpy(&tmp, beg, 4);
                        memcpy(beg, end, 4);
                        memcpy(end, &tmp, 4);

                        beg += 4;
                        end -= 4;
                    }
                }
            }

            break;
        }

        case (VERTICAL):
        {
            uint w  = width();
            uint h  = height();

            if (sixteenBit())
            {
                unsigned short  tmp[4];
                unsigned short* line1 = 0;
                unsigned short* line2 = 0;
                unsigned short* data  = reinterpret_cast<unsigned short*>(bits());

                // can be done inplace
                uint hHalf = (h / 2);

                for (uint y = 0 ; y < hHalf ; ++y)
                {
                    line1 = data + y * w * 4;
                    line2 = data + (h - y - 1) * w * 4;

                    for (uint x = 0 ; x < w ; ++x)
                    {
                        memcpy(&tmp, line1, 8);
                        memcpy(line1, line2, 8);
                        memcpy(line2, &tmp, 8);

                        line1 += 4;
                        line2 += 4;
                    }
                }
            }
            else
            {
                uchar  tmp[4];
                uchar* line1 = 0;
                uchar* line2 = 0;
                uchar* data  = bits();

                // can be done inplace
                uint hHalf = (h / 2);

                for (uint y = 0 ; y < hHalf ; ++y)
                {
                    line1 = data + y * w * 4;
                    line2 = data + (h - y - 1) * w * 4;

                    for (uint x = 0 ; x < w ; ++x)
                    {
                        memcpy(&tmp, line1, 4);
                        memcpy(line1, line2, 4);
                        memcpy(line2, &tmp, 4);

                        line1 += 4;
                        line2 += 4;
                    }
                }
            }

            break;
        }

        default:
            break;
    }
}

bool DImg::rotateAndFlip(int orientation)
{
    bool rotatedOrFlipped = false;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            return false;

        case DMetadata::ORIENTATION_HFLIP:
            flip(DImg::HORIZONTAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_180:
            rotate(DImg::ROT180);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_VFLIP:
            flip(DImg::VERTICAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            rotate(DImg::ROT90);
            flip(DImg::HORIZONTAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90:
            rotate(DImg::ROT90);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            rotate(DImg::ROT90);
            flip(DImg::VERTICAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_270:
            rotate(DImg::ROT270);
            rotatedOrFlipped = true;
            break;
    }

    return rotatedOrFlipped;
}
bool DImg::reverseRotateAndFlip(int orientation)
{
    bool rotatedOrFlipped = false;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            return false;

        case DMetadata::ORIENTATION_HFLIP:
            flip(DImg::HORIZONTAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_180:
            rotate(DImg::ROT180);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_VFLIP:
            flip(DImg::VERTICAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            rotate(DImg::ROT270);
            flip(DImg::HORIZONTAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90:
            rotate(DImg::ROT270);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            rotate(DImg::ROT270);
            flip(DImg::VERTICAL);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_270:
            rotate(DImg::ROT90);
            rotatedOrFlipped = true;
            break;
    }

    return rotatedOrFlipped;
}

bool DImg::transform(int transformAction)
{
    switch (transformAction)
    {
        case MetaEngineRotation::NoTransformation:
        default:
            return false;
            break;
        case MetaEngineRotation::FlipHorizontal:
            flip(DImg::HORIZONTAL);
            break;
        case MetaEngineRotation::FlipVertical:
            flip(DImg::VERTICAL);
            break;
        case MetaEngineRotation::Rotate90:
            rotate(DImg::ROT90);
            break;
        case MetaEngineRotation::Rotate180:
            rotate(DImg::ROT180);
            break;
        case MetaEngineRotation::Rotate270:
            rotate(DImg::ROT270);
            break;
    }
    return true;
}

void DImg::convertToSixteenBit()
{
    convertDepth(64);
}

void DImg::convertToEightBit()
{
    convertDepth(32);
}

void DImg::convertToDepthOfImage(const DImg* const otherImage)
{
    if (otherImage->sixteenBit())
    {
        convertToSixteenBit();
    }
    else
    {
        convertToEightBit();
    }
}

void DImg::convertDepth(int depth)
{
    if (isNull())
    {
        return;
    }

    if (depth != 32 && depth != 64)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << " : wrong color depth!";
        return;
    }

    if (((depth == 32) && !sixteenBit()) ||
        ((depth == 64) && sixteenBit()))
    {
        return;
    }

    if (depth == 32)
    {
        // downgrading from 16 bit to 8 bit

        uchar*  data = new uchar[width()*height() * 4];
        uchar*  dptr = data;
        ushort* sptr = reinterpret_cast<ushort*>(bits());
        uint dim     = width() * height() * 4;

        for (uint i = 0; i < dim; ++i)
        {
            *dptr++ = (*sptr++ * 256UL) / 65536UL;
        }

        delete [] m_priv->data;
        m_priv->data = data;
        m_priv->sixteenBit = false;
    }
    else if (depth == 64)
    {
        // upgrading from 8 bit to 16 bit

        uchar*  data = new uchar[width()*height() * 8];
        ushort* dptr = reinterpret_cast<ushort*>(data);
        uchar*  sptr = bits();

        // use default seed of the generator
        RandomNumberGenerator generator;
        ushort noise = 0;

        uint dim = width() * height() * 4;

        for (uint i = 0; i < dim; ++i)
        {
            if (i % 4 < 3)
            {
                noise = generator.number(0, 255);
            }
            else
            {
                noise = 0;
            }

            *dptr++ = (*sptr++ * 65536ULL) / 256ULL + noise;
        }

        delete [] m_priv->data;
        m_priv->data       = data;
        m_priv->sixteenBit = true;
    }
}

void DImg::fill(const DColor& color)
{
    if (isNull())
    {
        return;
    }

    // caching
    uint dim = width() * height() * 4;

    if (sixteenBit())
    {
        unsigned short* imgData16 = reinterpret_cast<unsigned short*>(m_priv->data);
        unsigned short red        = (unsigned short)color.red();
        unsigned short green      = (unsigned short)color.green();
        unsigned short blue       = (unsigned short)color.blue();
        unsigned short alpha      = (unsigned short)color.alpha();

        for (uint i = 0 ; i < dim ; i += 4)
        {
            imgData16[i    ] = blue;
            imgData16[i + 1] = green;
            imgData16[i + 2] = red;
            imgData16[i + 3] = alpha;
        }
    }
    else
    {
        uchar* imgData = m_priv->data;
        uchar red      = (uchar)color.red();
        uchar green    = (uchar)color.green();
        uchar blue     = (uchar)color.blue();
        uchar alpha    = (uchar)color.alpha();

        for (uint i = 0 ; i < dim ; i += 4)
        {
            imgData[i    ] = blue;
            imgData[i + 1] = green;
            imgData[i + 2] = red;
            imgData[i + 3] = alpha;
        }
    }
}

QByteArray DImg::getUniqueHash() const
{
    if (m_priv->attributes.contains(QLatin1String("uniqueHash")))
    {
        return m_priv->attributes[QLatin1String("uniqueHash")].toByteArray();
    }

    if (!m_priv->attributes.contains(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHash called without originalFilePath property set!";
        return QByteArray();
    }

    QString filePath = m_priv->attributes.value(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);
    QByteArray hash = DImgLoader::uniqueHash(filePath, *this, false);

    // attribute is written by DImgLoader

    return hash;
}

QByteArray DImg::getUniqueHash(const QString& filePath)
{
    return DImgLoader::uniqueHash(filePath, DImg(), true);
}

QByteArray DImg::getUniqueHashV2() const
{
    if (m_priv->attributes.contains(QLatin1String("uniqueHashV2")))
    {
        return m_priv->attributes[QLatin1String("uniqueHashV2")].toByteArray();
    }

    if (!m_priv->attributes.contains(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHash called without originalFilePath property set!";
        return QByteArray();
    }

    QString filePath = m_priv->attributes.value(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);
    return DImgLoader::uniqueHashV2(filePath, this);
}

QByteArray DImg::getUniqueHashV2(const QString& filePath)
{
    return DImgLoader::uniqueHashV2(filePath);
}

QByteArray DImg::createImageUniqueId() const
{
    NonDeterministicRandomData randomData(16);
    QByteArray imageUUID = randomData.toHex();
    imageUUID           += getUniqueHashV2();
    return imageUUID;
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 bool resetExifOrientationTag)
{
    PrepareMetadataFlags flags = PrepareMetadataFlagsAll;

    if (!resetExifOrientationTag)
    {
        flags &= ~ResetExifOrientationTag;
    }

    QUrl url = QUrl::fromLocalFile(originalFilePath());
    prepareMetadataToSave(intendedDestPath, destMimeType, url.fileName(), flags);
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 const QString& originalFileName, PrepareMetadataFlags flags)
{
    if (isNull())
    {
        return;
    }

    // Get image Exif/IPTC data.
    DMetadata meta(getMetadata());

    if (flags & RemoveOldMetadataPreviews || flags & CreateNewMetadataPreview)
    {
        // Clear IPTC preview
        meta.removeIptcTag("Iptc.Application2.Preview");
        meta.removeIptcTag("Iptc.Application2.PreviewFormat");
        meta.removeIptcTag("Iptc.Application2.PreviewVersion");

        // Clear Exif thumbnail
        meta.removeExifThumbnail();

        // Clear Tiff thumbnail
        MetaEngine::MetaDataMap tiffThumbTags = meta.getExifTagsDataList(QStringList() << QLatin1String("SubImage1"));

        for (MetaEngine::MetaDataMap::iterator it = tiffThumbTags.begin(); it != tiffThumbTags.end(); ++it)
        {
            meta.removeExifTag(it.key().toLatin1().constData());
        }
    }

    bool createNewPreview    = false;
    QSize previewSize;

    // Refuse preview creation for images with transparency
    // as long as we have no format to support this. See bug 286127
    bool skipPreviewCreation = hasTransparentPixels();

    if (flags & CreateNewMetadataPreview && !skipPreviewCreation)
    {
        const QSize standardPreviewSize(1280, 1280);
        previewSize = size();

        // Scale to standard preview size. Only scale down, not up
        if (width() > (uint)standardPreviewSize.width() && height() > (uint)standardPreviewSize.height())
        {
            previewSize.scale(standardPreviewSize, Qt::KeepAspectRatio);
        }

        // Only store a new preview if it is worth it - the original should be significantly larger than the preview
        createNewPreview = (2 * (uint)previewSize.width() <= width());
    }

    if (createNewPreview)
    {
        // Create the preview QImage
        QImage preview;
        {
            if (!IccManager::isSRGB(*this))
            {
                DImg previewDImg;

                if (previewSize.width() >= (int)width())
                {
                    previewDImg = copy();
                }
                else
                {
                    previewDImg = smoothScale(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio);
                }

                IccManager manager(previewDImg);
                manager.transformToSRGB();
                preview = previewDImg.copyQImage();
            }
            else
            {
                // Ensure that preview is not upscaled
                if (previewSize.width() >= (int)width())
                {
                    preview = copyQImage();
                }
                else
                {
                    preview = smoothScale(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio).copyQImage();
                }
            }
        }

        // Update IPTC preview.
        // see bug #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K during of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // There is no limitation with TIFF and PNG about IPTC byte array size.
        // So for a JPEG file, we don't store the IPTC preview.
        if ((destMimeType.toUpper() != QLatin1String("JPG") && destMimeType.toUpper() != QLatin1String("JPEG") &&
             destMimeType.toUpper() != QLatin1String("JPE"))
           )
        {
            // Non JPEG file, we update IPTC preview
            meta.setImagePreview(preview);
        }

        if (destMimeType.toUpper() == QLatin1String("TIFF") || destMimeType.toUpper() == QLatin1String("TIF"))
        {
            // With TIFF file, we don't store JPEG thumbnail, we even need to erase it and store
            // a thumbnail at a special location. See bug #211758
            QImage thumb = preview.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            meta.setTiffThumbnail(thumb);
        }
        else
        {
            // Update Exif thumbnail.
            QImage thumb = preview.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            meta.setExifThumbnail(thumb);
        }
    }

    // Update Exif Image dimensions.
    meta.setImageDimensions(size());

    // Update Exif Document Name tag with the original file name.
    if (!originalFileName.isEmpty())
    {
        meta.setExifTagString("Exif.Image.DocumentName", originalFileName);
    }

    // Update Exif Orientation tag if necessary.
    if (flags & ResetExifOrientationTag)
    {
        meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
    }

    if (!m_priv->imageHistory.isEmpty())
    {
        DImageHistory forSaving(m_priv->imageHistory);
        forSaving.adjustReferredImages();

        QUrl url         = QUrl::fromLocalFile(intendedDestPath);
        QString filePath = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile() + QLatin1Char('/');
        QString fileName = url.fileName();

        if (!filePath.isEmpty() && !fileName.isEmpty())
        {
            forSaving.purgePathFromReferredImages(filePath, fileName);
        }

        QString imageHistoryXml = forSaving.toXml();
        meta.setImageHistory(imageHistoryXml);
    }

    if (flags & CreateNewImageHistoryUUID)
    {
        meta.setImageUniqueId(QString::fromUtf8(createImageUniqueId()));
    }

    // Store new Exif/IPTC/XMP data into image.
    setMetadata(meta.data());
}

HistoryImageId DImg::createHistoryImageId(const QString& filePath, HistoryImageId::Type type) const
{
    HistoryImageId id = DImgLoader::createHistoryImageId(filePath, *this, DMetadata(getMetadata()));
    id.setType(type);
    return id;
}

HistoryImageId DImg::addAsReferredImage(const QString& filePath, HistoryImageId::Type type)
{
    HistoryImageId id = createHistoryImageId(filePath, type);
    m_priv->imageHistory.purgePathFromReferredImages(id.path(), id.fileName());
    addAsReferredImage(id);
    return id;
}

void DImg::addAsReferredImage(const HistoryImageId& id)
{
    m_priv->imageHistory << id;
}

void DImg::insertAsReferredImage(int afterHistoryStep, const HistoryImageId& id)
{
    m_priv->imageHistory.insertReferredImage(afterHistoryStep, id);
}

void DImg::addCurrentUniqueImageId(const QString& uuid)
{
    m_priv->imageHistory.adjustCurrentUuid(uuid);
}

void DImg::addFilterAction(const Digikam::FilterAction& action)
{
    m_priv->imageHistory << action;
}

const DImageHistory& DImg::getImageHistory() const
{
    return m_priv->imageHistory;
}

DImageHistory& DImg::getImageHistory()
{
    return m_priv->imageHistory;
}

void DImg::setImageHistory(const DImageHistory& history)
{
    m_priv->imageHistory = history;
}

bool DImg::hasImageHistory() const
{
    if (m_priv->imageHistory.isEmpty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

DImageHistory DImg::getOriginalImageHistory() const
{
    return attribute(QLatin1String("originalImageHistory")).value<DImageHistory>();
}

void DImg::setHistoryBranch(bool isBranch)
{
    setHistoryBranchAfter(getOriginalImageHistory(), isBranch);
}

void DImg::setHistoryBranchAfter(const DImageHistory& historyBeforeBranch, bool isBranch)
{
    int addedSteps = m_priv->imageHistory.size() - historyBeforeBranch.size();
    setHistoryBranchForLastSteps(addedSteps, isBranch);
}

void DImg::setHistoryBranchForLastSteps(int numberOfLastHistorySteps, bool isBranch)
{
    int firstStep = m_priv->imageHistory.size() - numberOfLastHistorySteps;

    if (firstStep < m_priv->imageHistory.size())
    {
        if (isBranch)
        {
            m_priv->imageHistory[firstStep].action.addFlag(FilterAction::ExplicitBranch);
        }
        else
        {
            m_priv->imageHistory[firstStep].action.removeFlag(FilterAction::ExplicitBranch);
        }
    }
}

QString DImg::colorModelToString(COLORMODEL colorModel)
{
    switch (colorModel)
    {
        case RGB:
            return i18nc("Color Model: RGB", "RGB");

        case GRAYSCALE:
            return i18nc("Color Model: Grayscale", "Grayscale");

        case MONOCHROME:
            return i18nc("Color Model: Monochrome", "Monochrome");

        case INDEXED:
            return i18nc("Color Model: Indexed", "Indexed");

        case YCBCR:
            return i18nc("Color Model: YCbCr", "YCbCr");

        case CMYK:
            return i18nc("Color Model: CMYK", "CMYK");

        case CIELAB:
            return i18nc("Color Model: CIE L*a*b*", "CIE L*a*b*");

        case COLORMODELRAW:
            return i18nc("Color Model: Uncalibrated (RAW)", "Uncalibrated (RAW)");

        case COLORMODELUNKNOWN:
        default:
            return i18nc("Color Model: Unknown", "Unknown");
    }
}

bool DImg::isAnimatedImage(const QString& filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);

    if (reader.supportsAnimation() && 
       (reader.imageCount() > 1))
    {
        qDebug(DIGIKAM_DIMG_LOG_QIMAGE) << "File \"" << filePath << "\" is an animated image ";
        return true;
    }

    return false;
}

}  // namespace Digikam
