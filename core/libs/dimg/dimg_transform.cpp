/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Transform operations.
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

#include "dimg_p.h"

namespace Digikam
{

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
            flip(DImg::HORIZONTAL);
            rotate(DImg::ROT270);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90:
            rotate(DImg::ROT270);
            rotatedOrFlipped = true;
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            flip(DImg::VERTICAL);
            rotate(DImg::ROT270);
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

} // namespace Digikam
