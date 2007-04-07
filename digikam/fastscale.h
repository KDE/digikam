/* ============================================================
 * Authors: Antonio Larrosa <larrosa at kde dot org>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-06-04
 * Description : fast smooth QImage based on Bresenham method
 *
 * Copyright 2002-2007 Antonio Larrosa
 * Copyright      2007 Gilles Caulier
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

private:

    static void fastScaleLineAvg(Q_UINT32 *Target, Q_UINT32 *Source, int SrcWidth, int TgtWidth);

    /**  Smooth 2D scaling */
    static void fastScaleRectAvg(Q_UINT32 *Target, Q_UINT32 *Source, int SrcWidth, int SrcHeight,
                                 int TgtWidth, int TgtHeight);

    /** 24-bit RGB (a pixel is packed in a 32-bit integer) */
    static inline unsigned long fastScaleAverage(unsigned long a, unsigned long b)
    {
        return ((a & 0xfefefeffUL) + (b & 0xfefefeffUL)) >> 1;
    }
};

}  // NameSpace Digikam
