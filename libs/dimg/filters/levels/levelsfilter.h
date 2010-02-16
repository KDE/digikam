/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-29
 * Description : Histogram Levels color correction.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LEVELSFILTER_H
#define LEVELSFILTER_H

// Local includes

#include "digikam_export.h"
#include "dcolor.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class LevelsFilterPriv;

class DIGIKAM_EXPORT LevelsContainer
{

public:

    LevelsContainer()
    {
    };

    ~LevelsContainer(){};

public:


};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LevelsFilter : public DImgThreadedFilter
{

public:

    explicit LevelsFilter(DImg* orgImage, QObject* parent=0, const LevelsContainer& settings=LevelsContainer());
    LevelsFilter(uchar* data, uint width, uint height, bool sixteenBit, const LevelsContainer& settings=LevelsContainer());
    virtual ~LevelsFilter();

    static void autoLevelsAdjustement(DImg *img, double& black, double& expo);
    static void autoLevelsAdjustement(uchar* data, int width, int height, bool sb, double& black, double& expo);

protected:

    virtual void filterImage();

  protected:

    LevelsContainer m_settings;

private:



private:

    LevelsFilterPriv* const d;
};

}  // namespace Digikam

#endif /* LEVELSFILTER_H */
