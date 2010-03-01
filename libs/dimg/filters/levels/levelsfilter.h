/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Levels image filter
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    LevelsContainer(){};
    ~LevelsContainer(){};

public:

    int    lInput[5];
    int    hInput[5];
    int    lOutput[5];
    int    hOutput[5];
    
    double gamma[5];
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT LevelsFilter : public DImgThreadedFilter
{

public:

    LevelsFilter(DImg* orgImage, QObject* parent=0, const LevelsContainer& settings=LevelsContainer());
    virtual ~LevelsFilter();

private:

    void filterImage();

private:

    LevelsContainer m_settings;
};

}  // namespace Digikam

#endif /* LEVELSFILTER_H */
