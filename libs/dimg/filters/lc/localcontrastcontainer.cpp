/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * Copyright (C) 2009 by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <cmath>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

LocalContrastContainer::LocalContrastContainer()
{
    stretch_contrast = true;
    high_saturation  = 100;
    low_saturation   = 100;
    function_id      = 0;

    for (int i = 0 ; i < TONEMAPPING_MAX_STAGES ; ++i)
    {
        stage[i].enabled = (i == 0);
        stage[i].power   = 30.0;
        stage[i].blur    = 80.0;
    }

    unsharp_mask.enabled   = false;
    unsharp_mask.power     = 30.0;
    unsharp_mask.blur      = 4.0;
    unsharp_mask.threshold = 0;
}

LocalContrastContainer::~LocalContrastContainer()
{
}

float LocalContrastContainer::get_power(int nstage)
{
    float power = stage[nstage].power;
    power       = (float)(pow(power / 100.0, 1.5) * 100.0);
    return power;
}

float LocalContrastContainer::get_blur(int nstage)
{
    return stage[nstage].blur;
}

float LocalContrastContainer::get_unsharp_mask_power()
{
    float power = unsharp_mask.power;
    power       = (float)(pow(power / 100.0, 3.0) * 10.0);
    return power;
}

float LocalContrastContainer::get_unsharp_mask_blur()
{
    return unsharp_mask.blur;
}

} // namespace Digikam
