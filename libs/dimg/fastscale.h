/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-04
 * Description : fast smooth QImage based on Bresenham method
 *
 * Copyright (C) 2002-2007 Antonio Larrosa <larrosa at kde dot org>
 * Copyright (C)      2007 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FASTSCALE_H
#define FASTSCALE_H

// Qt includes.

#include <qimage.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FastScale
{

public:

    /**
    * @returns an image of size @p width x @p height, resulting from resizing the @p img image
    */
    static QImage fastScaleQImage(const QImage &img, int width, int height);

    /**
    * Resizes the @p img QImage to the size of @p tgt and stores the result into @p tgt
    */
    static void fastScaleQImage(const QImage &img, QImage &tgt);

    /**
    * @returns an image of size @p dw x @p dh, resulting from resizing the 
      section ( @p sx , @p sy - @p sw x @p sh ) of @p img image.
    */
    static QImage fastScaleSectionQImage(const QImage &img, int sx, int sy, int sw, int sh, int dw, int dh);

private:

    static void fastScaleLineAvg(quint32 *Target, quint32 *Source, int SrcWidth, int TgtWidth);

    /**  Smooth 2D scaling */
    static void fastScaleRectAvg(quint32 *Target, quint32 *Source, int SrcWidth, int SrcHeight,
                                 int TgtWidth, int TgtHeight);

    /** 24-bit RGB (a pixel is packed in a 32-bit integer) */
    static inline unsigned long fastScaleAverage(unsigned long a, unsigned long b)
    {
        return ((a & 0xfefefeffUL) + (b & 0xfefefeffUL)) >> 1;
    }
};

}  // NameSpace Digikam

#endif /* FASTSCALE_H */
