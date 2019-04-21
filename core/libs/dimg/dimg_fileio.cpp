/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API
 *               Files input output
 *
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

bool DImg::loadItemInfo(const QString& filePath, bool loadMetadata, bool loadICCData, bool loadUniqueHash, bool loadImageHistory)
{
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadItemInfo;

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
    DImgLoader::LoadFlags loadFlags = DImgLoader::LoadItemInfo | DImgLoader::LoadImageData;

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

bool DImg::load(const QString& filePath,
                int loadFlagsInt,
                DImgLoaderObserver* const observer,
                const DRawDecoding& rawDecodingSettings)
{
    FORMAT format                   = fileFormat(filePath);
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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
                setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
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

#ifdef HAVE_IMAGE_MAGICK

    {
        qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : Try to load with ImageMagick";
        MagickLoader loader(this);
        loader.setLoadFlags(loadFlags);

        if (loader.load(filePath, observer))
        {
            m_priv->null       = !loader.hasLoadedData();
            m_priv->alpha      = loader.hasAlpha();
            m_priv->sixteenBit = loader.sixteenBit();
            setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
            return true;
        }
    }

#endif // HAVE_IMAGE_MAGICK

    {
        qCDebug(DIGIKAM_DIMG_LOG) << filePath << " : Try to load with QImage";
        QImageLoader loader(this);
        loader.setLoadFlags(loadFlags);

        if (loader.load(filePath, observer))
        {
            m_priv->null       = !loader.hasLoadedData();
            m_priv->alpha      = loader.hasAlpha();
            m_priv->sixteenBit = loader.sixteenBit();
            setAttribute(QLatin1String("isReadOnly"), loader.isReadOnly());
            return true;
        }
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
            // For QImage or ImageMagick based.
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
            setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
            return loader.save(filePath, observer);
        }
        else
        {
            JPEGLoader loader(this);
            setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
            return loader.save(filePath, observer);
        }
    }
    else if (frm == QLatin1String("PNG"))
    {
        PNGLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
    else if (frm == QLatin1String("TIFF") || frm == QLatin1String("TIF"))
    {
        TIFFLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }

#ifdef HAVE_JASPER
    else if (frm == QLatin1String("JP2") ||
             frm == QLatin1String("J2K") ||
             frm == QLatin1String("JPX") ||
             frm == QLatin1String("JPC") ||
             frm == QLatin1String("PGX"))
    {
        JP2KLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
#endif // HAVE_JASPER

    else if (frm == QLatin1String("PGF"))
    {
        PGFLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
    }
    else
    {
#ifdef HAVE_IMAGE_MAGICK
        setAttribute(QLatin1String("format"), format);
        MagickLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
#else
        setAttribute(QLatin1String("format"), format);
        QImageLoader loader(this);
        setAttribute(QLatin1String("savedFormat-isReadOnly"), loader.isReadOnly());
        return loader.save(filePath, observer);
#endif
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

    DRawInfo dcrawIdentify;
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

    // In others cases, ImageMagick or QImage will be used to try to open file.
    return QIMAGE;
}

} // namespace Digikam
