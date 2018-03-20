/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009      by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrastcontainer.h"

// C++ includes

#include <cstdio>
#include <cstring>
#include <cstdlib>

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

LocalContrastContainer::LocalContrastContainer()
{
    stretchContrast = true;
    highSaturation  = 100;
    lowSaturation   = 100;
    functionId      = 0;

    for (int i = 0 ; i < TONEMAPPING_MAX_STAGES ; ++i)
    {
        stage[i].enabled = (i == 0);
        stage[i].power   = 30.0;
        stage[i].blur    = 80.0;
    }
}

LocalContrastContainer::~LocalContrastContainer()
{
}

double LocalContrastContainer::getPower(int nstage) const
{
    float power = stage[nstage].power;
    power       = (float)(qPow(power / 100.0, 1.5) * 100.0);
    return power;
}

double LocalContrastContainer::getBlur(int nstage) const
{
    return stage[nstage].blur;
}

} // namespace Digikam
