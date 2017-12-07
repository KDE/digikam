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

    double getPower(int nstage)  const;
    double getBlur(int nstage)   const;

public:

    bool stretchContrast;

    int  lowSaturation;
    int  highSaturation;
    int  functionId;

    struct
    {
        bool  enabled;
        double power;
        double blur;
    }
    stage[TONEMAPPING_MAX_STAGES];
};

} // namespace Digikam

#endif // LOCALCONTRASTCONTAINER_H
