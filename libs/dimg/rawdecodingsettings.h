/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-06
 * Description : Raw file decoding options to use with dcraw.
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

// Qt includes.

#include <qstring.h>

namespace Digikam
{

class DIGIKAM_EXPORT RawDecodingSettings
{

public:
    
    enum ICCCOLORCORRECTIONMODE
    {
        NO_ICC = 0,          // No color correction during Raw file decoding.
        SRGB_WORKSPACE,      // Using default workspace color profile (sRGB) 
        EMBED_WORKSPACE,     // Using embedded workspace color profile in RAW file.
        USER_PROFILES        // Using user input and workspace profile files.
    };

    RawDecodingSettings()
    {
        ICCColorCorrectionMode  = SRGB_WORKSPACE;
        cameraICCProfilePath    = QString::null;
        outputICCProfilePath    = QString::null;
    
        enableNoiseReduction    = false;
        NRSigmaDomain           = 2.0;
        NRSigmaRange            = 4.0;

        enableRAWQuality        = true;
        RAWQuality              = 0;

        RGBInterpolate4Colors   = false;
        SuperCCDsecondarySensor = false;
        unclipColors            = false;
        cameraColorBalance      = true;
        automaticColorBalance   = true;
        halfSizeColorImage      = false;
        sixteenBitsImage        = true;
    };
    
    ~RawDecodingSettings(){};

    void optimizeTimeLoading(void)
    {
        ICCColorCorrectionMode  = NO_ICC;
        enableNoiseReduction    = false;
        enableRAWQuality        = false;
        RGBInterpolate4Colors   = false;
        SuperCCDsecondarySensor = false;
        unclipColors            = false;
        cameraColorBalance      = true;
        automaticColorBalance   = true;
        halfSizeColorImage      = true;
        sixteenBitsImage        = true;
    };

public:
    
    /** Half-size color image decoding (twice as fast as "enableRAWQuality"). Use this option to 
        reduce time loading to render histogram for example, no to render an image to screen. */
    bool  halfSizeColorImage;

    /**  If true, decode RAW file in 16 bits per color per pixel else 8 bits. */
    bool  sixteenBitsImage;

    /**  Use the color balance specified by the camera. If this can't be found, reverts to the default. */
    bool  cameraColorBalance;
    
    /** Automatic color balance. The default is to use a fixed color balance based on a white card 
        photographed in sunlight. */
    bool  automaticColorBalance;
    
    /** RAW file decoding using RGB interpolation as four colors. */
    bool  RGBInterpolate4Colors;

    /** For Fuji Super CCD SR cameras, use the secondary sensors. In effect underexposing the image 
        by four stops to reveal detail in the highlights. */
    bool  SuperCCDsecondarySensor;
    
    /** By default, dcraw clips all colors to prevent pink highlights. Use -n -b 0.25 to leave 
        the image data completely unclipped. */
    bool  unclipColors;

    /** RAW file decoding using quality factor. */
    bool  enableRAWQuality;

    /** RAW quality decoding factor value. */
    int   RAWQuality;

    /** RAW file decoding using bilateral filter to reduce noise. This is '-B sigma_domain sigma_range' 
        dcraw option to smooth noise while preserving edges. sigma_domain is in units of pixels, while
        sigma_range is in units of CIELab colorspace. */
    bool  enableNoiseReduction;

    /** Noise reduction sigma domain value. */
    float NRSigmaDomain;
    
    /** Noise reduction sigma range value. */
    float NRSigmaRange;    
    
    /**  Input ICC profile color correction mode to use (dcraw -p option). */
    int   ICCColorCorrectionMode;
    
    /** The file path to ICC camera color profile (input profile). Set to 'embed' to use the ICC 
        profile embedded in the raw photo. */
    QString cameraICCProfilePath;
    
    /** The file path to ICC output color profile (output profile). Set empty to use default 
        sRGB color space. */
    QString outputICCProfilePath;    
};

}  // namespace Digikam

#endif  // RAWDECODINGSETTINGS_H
