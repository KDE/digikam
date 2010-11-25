/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-18
 * Description : color balance filter
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CBFILTER_H
#define CBFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class CBFilterPriv;

class DIGIKAM_EXPORT CBContainer
{

public:

    CBContainer()
    {
        red   = 1.0;
        green = 1.0;
        blue  = 1.0;
        alpha = 1.0;
        gamma = 1.0;
    };

    ~CBContainer() {};

public:

    double red;
    double green;
    double blue;
    double alpha;
    double gamma;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT CBFilter : public DImgThreadedFilter
{

public:

    explicit CBFilter(DImg* orgImage, QObject* parent=0, const CBContainer& settings=CBContainer());
    virtual ~CBFilter();

private:

    void filterImage();

    void reset();
    void setGamma(double val);
    void setTables(int* redMap, int* greenMap, int* blueMap, int* alphaMap, bool sixteenBit);
    void getTables(int* redMap, int* greenMap, int* blueMap, int* alphaMap, bool sixteenBit);
    void adjustRGB(double r, double g, double b, double a, bool sixteenBit);
    void applyCBFilter(DImg& image, double r, double g, double b, double a);

private:

    CBFilterPriv* const d;
};

}  // namespace Digikam

#endif /* CBFILTER_H */
