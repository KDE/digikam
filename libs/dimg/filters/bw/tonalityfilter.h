/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Change tonality image filters
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TONALITYFILTER_H
#define TONALITYFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class TonalityFilterPriv;

class DIGIKAM_EXPORT TonalityContainer
{

public:

    TonalityContainer()
    {
        redMask   = 0;
        greenMask = 0;
        blueMask  = 0;
    };

    ~TonalityContainer(){};

public:

    int redMask;
    int greenMask;
    int blueMask;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT TonalityFilter : public DImgThreadedFilter
{

public:

    explicit TonalityFilter(DImg* orgImage, QObject* parent=0, const TonalityContainer& settings=TonalityContainer());
    TonalityFilter(uchar* bits, uint width, uint height, bool sixteenBits, 
                   const TonalityContainer& settings=TonalityContainer());
    virtual ~TonalityFilter();

private:

    void filterImage();

    void changeTonality(DImg& image);
    void changeTonality(uchar* data, uint width, uint height, bool sixteenBit);
    
private:

    TonalityContainer m_settings;
};

}  // namespace Digikam

#endif /* TONALITYFILTER_H */
