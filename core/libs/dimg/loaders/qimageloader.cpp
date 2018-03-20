/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2017 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "qimageloader.h"

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QImageReader>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

QImageLoader::QImageLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha = false;
}

bool QImageLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath, DImg::QIMAGE);

    // Loading is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);

    QImage image = reader.read();

    if (observer)
    {
        observer->progressInfo(m_image, 0.9F);
    }

    if (image.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Can not load \"" << filePath << "\" using DImg::QImageLoader!";
        loadingFailed();
        return false;
    }

    int colorModel    = DImg::COLORMODELUNKNOWN;
    int originalDepth = 0;

    switch (image.format())
    {
        case QImage::Format_Invalid:
        default:
            colorModel = DImg::COLORMODELUNKNOWN;
            originalDepth = 0;
            break;

        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
            colorModel = DImg::MONOCHROME;
            originalDepth = 1;
            break;

        case QImage::Format_Indexed8:
            colorModel = DImg::INDEXED;
            originalDepth = 0;
            break;

        case QImage::Format_RGB32:
            colorModel = DImg::RGB;
            originalDepth = 8;
            break;

        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            colorModel = DImg::RGB;
            originalDepth = 8;
            break;
    }

    m_hasAlpha        = image.hasAlphaChannel();
    QImage target     = image.convertToFormat(QImage::Format_ARGB32);
    uint w            = target.width();
    uint h            = target.height();
    uchar* const data = new_failureTolerant(w, h, 4);

    if (!data)
    {
        qCWarning(DIGIKAM_DIMG_LOG_QIMAGE) << "Failed to allocate memory for loading" << filePath;
        loadingFailed();
        return false;
    }

    uint*  sptr = reinterpret_cast<uint*>(target.bits());
    uchar* dptr = data;

    for (uint i = 0 ; i < w * h ; ++i)
    {
        dptr[0] = qBlue(*sptr);
        dptr[1] = qGreen(*sptr);
        dptr[2] = qRed(*sptr);
        dptr[3] = qAlpha(*sptr);

        dptr += 4;
        sptr++;
    }

    if (observer)
    {
        observer->progressInfo(m_image, 1.0);
    }

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = data;

    // We considering that PNG is the most representative format of an image loaded by Qt
    imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   originalDepth);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(w, h));

    return true;
}

bool QImageLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
    int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    if (quality < 0)
    {
        quality = 90;
    }

    if (quality > 100)
    {
        quality = 100;
    }

    QVariant formatAttr = imageGetAttribute(QLatin1String("format"));
    QByteArray format   = formatAttr.toByteArray();
    QImage image        = m_image->copyQImage();

    if (observer)
    {
        observer->progressInfo(m_image, 0.1F);
    }

    // Saving is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values
    bool success = image.save(filePath, format.toUpper().constData(), quality);

    if (observer && success)
    {
        observer->progressInfo(m_image, 1.0F);
    }

    imageSetAttribute(QLatin1String("format"), format.toUpper());

    saveMetadata(filePath);

    return success;
}

bool QImageLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool QImageLoader::sixteenBit() const
{
    return false;
}

bool QImageLoader::isReadOnly() const
{
    return false;
}

}  // namespace Digikam
