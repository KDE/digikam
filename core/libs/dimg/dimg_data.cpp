/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Data management.
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
        m_priv->null   = true;
        m_priv->width  = 0;
        m_priv->height = 0;
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

} // namespace Digikam
