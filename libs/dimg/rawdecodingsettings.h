/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-06
 * Description : Raw file decoding options used with dcraw.
 * 
 * Copyright 2006 by Gilles Caulier
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

#ifndef RAWDECODINGSETTINGS_H
#define RAWDECODINGSETTINGS_H

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RawDecodingSettings
{

public:
    
    RawDecodingSettings()
    {
        enableRAWQuality      = false;
        RAWQuality            = 0;
        RGBInterpolate4Colors = false;
        cameraColorBalance    = true;
        automaticColorBalance = true;
    };
    
    ~RawDecodingSettings(){};

public:

    // Use the color balance specified by the camera. If this can't be found, reverts to the default.
    bool cameraColorBalance;
    
    // Automatic color balance. The default is to use a fixed color balance
    // based on a white card photographed in sunlight.
    bool automaticColorBalance;
    
    // RAW file decoding using RGB interpolation as four colors.
    bool RGBInterpolate4Colors;
    
    // RAW file decoding using quality factor. 
    bool enableRAWQuality;

    // RAW quality decoding factor value.
    int  RAWQuality;
    
};

}  // namespace Digikam

#endif  // RAWDECODINGSETTINGS_H
