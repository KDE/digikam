/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Properties accessors.
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
        return nullptr;
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
    return attribute(QLatin1String("isReadOnly")).toBool();
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
    return m_priv->attributes.value(QLatin1String("savedFormat")).toString();
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

    foreach (const QString& key, m_priv->fileOriginAttributes())
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
    QVariant savedformat = attribute(QLatin1String("savedFormat"));

    if (!savedformat.isNull())
    {
        map.insert(QLatin1String("format"), savedformat);
    }

    QVariant readonly = attribute(QLatin1String("savedFormat-isReadOnly"));

    if (!readonly.isNull())
    {
        map.insert(QLatin1String("isReadOnly"), readonly);
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

    foreach (const QString& key, m_priv->fileOriginAttributes())
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

    if (lanczos_func == nullptr)
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
    uchar* data = nullptr;

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

} // namespace Digikam
