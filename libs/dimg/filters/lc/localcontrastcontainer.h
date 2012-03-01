/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Local Contrast settings container
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2009      by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOCALCONTRASTCONTAINER_H
#define LOCALCONTRASTCONTAINER_H

#define TONEMAPPING_MAX_STAGES 4

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastContainer
{

public:

    LocalContrastContainer();
    ~LocalContrastContainer();

    float get_power(int nstage);
    float get_blur(int nstage);

    float get_unsharp_mask_power();
    float get_unsharp_mask_blur();

public:

    bool stretch_contrast;

    int  low_saturation;
    int  high_saturation;
    int  function_id;

    struct
    {
        bool  enabled;
        float power;
        float blur;
    }
    stage[TONEMAPPING_MAX_STAGES];

    struct
    {
        bool  enabled;       // digiKam Unsharp Mask settings:
        float blur;          // Radius    : 0.00 - 120.00
        float power;         // Amount    : 0.0  - 5.0
        int   threshold;     // threshold : 0.00 - 1.00
    }
    unsharp_mask;
};

} // namespace Digikam

#endif // LOCALCONTRASTCONTAINER_H
