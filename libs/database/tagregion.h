/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-26
 * Description : Tag region formatting
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef TAGREGION_H
#define TAGREGION_H

// Qt includes

#include <QRect>
#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_DATABASE_EXPORT TagRegion
{
public:

    /**
     * Use this small class to convert between the formatted
     * textual representation of a tag region in the database
     * and the corresponding object.
     */

    /// Construct with the textual descriptor
    explicit TagRegion(const QString& descriptor);
    /// Construct with the region
    explicit TagRegion(const QRect& rect);

    enum Type
    {
        Invalid,
        Rect
    };

    Type type() const;
    bool isValid() const;

    QString toXml() const;
    QRect toRect() const;
    QRect toCandyRect() const;

    /**
     * Converts detail rectangles taken from a reduced size image to the original size, and vice versa
     */
    static QRect mapToOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& reducedSizeDetail);
    static QRect mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail);
    /// Takes the original and reduced size from the DImg
    static QRect mapToOriginalSize(const DImg& reducedSizeImage, const QRect& reducedSizeDetail);
    static QRect mapFromOriginalSize(const DImg& reducedSizeImage, const QRect& fullSizeDetail);

protected:

    QVariant m_value;
    Type     m_type;
};

}

#endif

