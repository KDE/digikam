/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 * ============================================================ */

#ifndef HSLFILTER_H
#define HSLFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class HSLFilterPriv;

class DIGIKAM_EXPORT HSLContainer
{

public:

    HSLContainer()
    {
        hue        = 0.0;
        saturation = 0.0;
        lightness  = 0.0;
    };

    ~HSLContainer(){};

public:

    double hue;
    double saturation;
    double lightness;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT HSLFilter : public DImgThreadedFilter
{

public:

    explicit HSLFilter(DImg* orgImage, QObject* parent=0, const HSLContainer& settings=HSLContainer());
    virtual ~HSLFilter();

private:

    void filterImage();

    void reset();
    void setHue(double val);
    void setSaturation(double val);
    void setLightness(double val);
    void applyHSL(DImg& image);

private:

    HSLFilterPriv* const d;
};

}  // namespace Digikam

#endif /* HSLFILTER_H */
