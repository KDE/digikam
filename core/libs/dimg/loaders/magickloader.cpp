/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : ImageMagick loader for DImg framework.
 *
 * Copyright (C) 2019 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "magickloader.h"

// Qt includes

#include <QMimeDatabase>

// ImageMagick includes

// Pragma directives to reduce warnings from ImageMagick header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#include <Magick++.h>

// Restore warnings

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

using namespace Magick;

namespace Digikam
{

MagickLoader::MagickLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha = false;
}

bool MagickLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(filePath).name().startsWith(QLatin1String("image/")))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "The ImageMagick codecs support only the image mime type";
        loadingFailed();
        return false;
    }

    readMetadata(filePath, DImg::QIMAGE);

    // Loading is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(m_image, 0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to load image with ImageMagick codecs";

    try
    {
        Image image;

        if (m_loadFlags & LoadImageData)
        {
            image.read(filePath.toUtf8().constData());

            if (observer)
            {
                observer->progressInfo(m_image, 0.8F);
            }

            qCDebug(DIGIKAM_DIMG_LOG) << "IM to DImg      :" << image.columns() << image.rows();
            qCDebug(DIGIKAM_DIMG_LOG) << "IM QuantumRange :" << QuantumRange;
            qCDebug(DIGIKAM_DIMG_LOG) << "IM Format       :" << image.format().c_str();

            int depth             = (QuantumRange >= 16) ? 16 : 8;
            Blob* const pixelBlob = new Blob;
            image.write(pixelBlob, "BGRA", depth);
            qCDebug(DIGIKAM_DIMG_LOG) << "IM blob size    :" << pixelBlob->length();

            if (observer)
            {
                observer->progressInfo(m_image, 0.9F);
            }

            imageWidth()  = image.columns();
            imageHeight() = image.rows();
            imageData()   = (uchar*)pixelBlob->data();

#if MagickLibVersion < 0x700
            m_hasAlpha    = image.matte();
#else
            m_hasAlpha    = image.alpha();
#endif

            // We considering that PNG is the most representative format of an image loaded by ImageMagick
            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   depth);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
        else
        {
            image.ping(filePath.toUtf8().constData());
            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   (QuantumRange >= 16) ? 16 : 8);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
    }
    catch (Exception& error_)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error_.what();
        loadingFailed();
        return false;
    }

    return true;
}

bool MagickLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    // Saving is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(m_image, 0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to save image with ImageMagick codecs";

    try
    {
        QVariant formatAttr = imageGetAttribute(QLatin1String("format"));
        QByteArray format   = formatAttr.toByteArray();

        if (observer)
        {
            observer->progressInfo(m_image, 0.8F);
        }

        Blob pixelBlob(imageData(), imageNumBytes());

        Image image;
        image.size(Geometry(imageWidth(), imageHeight()));
        image.magick("BGRA");
        image.depth(imageBitsDepth());

#if MagickLibVersion < 0x700
        image.matte(imageHasAlpha());
#else
        image.alpha(imageHasAlpha());
#endif

        image.read(pixelBlob);
        image.magick(format.data());
        image.write(filePath.toUtf8().constData());

        if (observer)
        {
            observer->progressInfo(m_image, 0.9F);
        }

        imageSetAttribute(QLatin1String("format"), format.toUpper());

        saveMetadata(filePath);
        return true;
    }
    catch (Exception& error_)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error_.what();
        return false;
    }

    return true;
}

bool MagickLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool MagickLoader::sixteenBit() const
{
    return true;
}

bool MagickLoader::isReadOnly() const
{
    return false;
}

} // namespace Digikam
