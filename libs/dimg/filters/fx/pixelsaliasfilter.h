/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : pixels antialiasing filter
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PIXELSALIASFILTERS_H
#define PIXELSALIASFILTERS_H

// C++ includes

#include <cmath>

// Local includes

#include "digikam_export.h"
#include "digikam_globals.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT PixelsAliasFilter
{
public:

    PixelsAliasFilter() {};
    ~PixelsAliasFilter() {};

public:   // Public methods.

    void pixelAntiAliasing(uchar* data, int Width, int Height, double X, double Y,
                           uchar* A, uchar* R, uchar* G, uchar* B);

    void pixelAntiAliasing16(unsigned short* data, int Width, int Height, double X, double Y,
                             unsigned short* A, unsigned short* R, unsigned short* G, unsigned short* B);

private:

    inline int setPositionAdjusted (int Width, int Height, int X, int Y);
};

}  // namespace Digikam

#endif /* PIXELSALIASFILTERS_H */
