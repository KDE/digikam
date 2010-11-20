/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
 *
 * ============================================================ */

#ifndef BCGFILTER_H
#define BCGFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

namespace Digikam
{

class DImg;
class BCGFilterPriv;

class DIGIKAM_EXPORT BCGContainer
{

public:

    BCGContainer()
    {
        channel    = LuminosityChannel;
        brightness = 0.0;
        contrast   = 0.0;
        gamma      = 1.0;
    };

    ~BCGContainer(){};

public:

    int    channel;

    double brightness;
    double contrast;
    double gamma;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT BCGFilter : public DImgThreadedFilter
{

public:

    explicit BCGFilter(DImg* orgImage, QObject* parent=0, const BCGContainer& settings=BCGContainer());
    virtual ~BCGFilter();

private:

    void filterImage();

    void reset();
    void setGamma(double val);
    void setBrightness(double val);
    void setContrast(double val);
    void applyBCG(DImg& image);
    void applyBCG(uchar* bits, uint width, uint height, bool sixteenBits);

private:

    BCGFilterPriv* const d;
};

}  // namespace Digikam

#endif /* BCGFILTER_H */
