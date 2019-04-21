/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Copying operations.
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
    return copy(rect.x(),
                rect.y(),
                rect.width(),
                rect.height());
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

} // namespace Digikam
