/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Contructors and destructor.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "dimg_p.h"

namespace Digikam
{

/**
 * 
 * DImg is a framework to support 16bits color depth image. it doesn't aim 
 * to be a complete imaging library; it uses QImage/ImageMagick for 
 * load/save files which are not supported natively by it. 
 * some of the features:
 * 
 * - Native Image Loaders, for some imageformats which are of interest to 
 * us: JPEG (complete), TIFF (mostly complete), PNG (complete), JPEG2000 
 * (complete), RAW (complete through libraw), PGF (complete).
 * For the rest ImageMAgick codecs or qimageloader are used.
 * 
 * - Metadata preservation: when a file is loaded, its metadata like XMP, 
 * IPTC, EXIF, JFIF are read and held in memory. now when you save back the 
 * file to the original file or to a different file, the metadata is 
 * automatically written. All is delegate to Exiv2 library.
 * 
 * - Explicitly Shared Container format (see qt docs): this is necessary for 
 * performance reasons.
 * 
 * - 8 bits and 16 bits support: if the file format is 16 bits, it will load up 
 * the image in 16bits format (TIFF/PNG/JPEG2000/RAW/PGF support) and all 
 * operations are done in 16 bits format, except when the rendering to screen 
 * is done, when its converted on the fly to a temporary 8 bits image and then 
 * rendered.
 * 
 * - Basic image manipulation: rotate, flip, color modifications, crop, 
 * scale. This has been ported from Imlib2 with 16 bits scaling support
 * and support for scaling of only a section of the image.
 * 
 * - Rendering to Pixmap: using QImage/QPixmap. (see above for rendering of 
 * 16 bits images).
 * 
 * - Pixel format: the pixel format is different from QImage pixel 
 * format. In QImage the pixel data is stored as unsigned ints and to 
 * access the individual colors you need to use bit-shifting to ensure 
 * endian correctness. in DImg, the pixel data is stored as unsigned char. 
 * the color layout is B,G,R,A (blue, green, red, alpha)
 * 
 * for 8 bits images: you can access individual color components like this:
 * 
 * uchar* const pixels = image.bits();
 * 
 * for (int i = 0 ; i < image.width() * image.height() ; ++i)
 * {
 *    pixel[0] // blue
 *    pixel[1] // green
 *    pixel[2] // red
 *    pixel[3] // alpha
 * 
 *    pixel += 4; // go to next pixel
 * }
 * 
 * and for 16 bits images:
 * 
 * ushort* const pixels = (ushort*)image.bits();
 * 
 * for (int i = 0 ; i < image.width() * image.height() ; ++i)
 * {
 *    pixel[0] // blue
 *    pixel[1] // green
 *    pixel[2] // red
 *    pixel[3] // alpha
 * 
 *    pixel += 4; // go to next pixel
 * }
 * 
 * The above is true for both big and little endian platforms. What this also
 * means is that the pixel format is different from that of QImage for big 
 * endian machines. Functions are provided if you want to get a copy of the 
 * DImg as a QImage.
 * 
 */
DImg::DImg()
    : m_priv(new Private)
{
}

DImg::DImg(const QByteArray& filePath,
           DImgLoaderObserver* const observer,
           const DRawDecoding& rawDecodingSettings)
    : m_priv(new Private)
{
    load(QString::fromUtf8(filePath), observer, rawDecodingSettings);
}

DImg::DImg(const QString& filePath,
           DImgLoaderObserver* const observer,
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

} // namespace Digikam
