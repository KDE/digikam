/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "filmgrainfilter.h"

// C++ includes

#include <cstdlib>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "globals.h"

namespace Digikam
{

FilmGrainFilter::FilmGrainFilter(DImg* orgImage, QObject* parent, const FilmGrainContainer& settings)
               : DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_settings = settings;
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImgThreadedFilter* parentFilter,
                                 const DImg& orgImage, const DImg& destImage,
                                 int progressBegin, int progressEnd,
                                 const FilmGrainContainer& settings)
               : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                                    parentFilter->filterName() + ": FilmGrain")
{
    m_settings = settings;
    filterImage();
}

void FilmGrainFilter::filterImage()
{
    // NOTE: about YCbCr (YUV) color space details, see this Url : http://en.allexperts.com/e/y/yc/ycbcr.htm
  
    if (m_settings.lum_intensity <= 0) return;
    if (m_settings.chroma_intensity <= 0) return;

    DColor color;
    int    progress;
    double local_luma_noise;
    double local_chroma_noise;

    int    width        = m_orgImage.width();
    int    height       = m_orgImage.height();
    bool   sb           = m_orgImage.sixteenBit();
    int    luma_noise   = m_settings.lum_intensity    * (sb ? 256.0 : 1.0);
    double chroma_noise = m_settings.chroma_intensity * (sb ? 256.0 : 1.0);

    qsrand(1); // noise will always be the same

    for (int x = 0; !m_cancel && x < width; ++x)
    {
        for (int y = 0; !m_cancel && y < height; ++y)
        {
            color = m_orgImage.getPixelColor(x, y);

            if (m_settings.addLuminanceNoise)
            {
                local_luma_noise = interpolate(m_settings.lum_shadows, m_settings.lum_midtones, 
                                               m_settings.lum_highlights, color) * luma_noise + 1.0;

                randomizeLuma(color, local_luma_noise);
            }

            if (m_settings.addChrominanceNoise)
            {
                // Adjust Chrominance blue noise.

                local_chroma_noise = interpolate(m_settings.chroma_shadows, m_settings.chroma_midtones, 
                                                 m_settings.chroma_highlights, color) * chroma_noise + 1.0;

                randomizeChroma(color, local_chroma_noise);
            }

            m_destImage.setPixelColor(x, y, color);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
            postProgress( progress );
    }
}

void FilmGrainFilter::randomizeLuma(DColor& col, double range)
{
    double y, cb, cr;
    col.getYCbCr(&y, &cb, &cr);
    
    double nRand = ((double)(qrand() % (int)range) - range/2.0) / (col.sixteenBit() ? 65535.0 : 255.0);
    y            = CLAMP(y + nRand, 0.0, 1.0);
    
    col.setYCbCr(y, cb, cr, col.sixteenBit());
}

void FilmGrainFilter::randomizeChroma(DColor& col, double range)
{
    double y, cb, cr;
    col.getYCbCr(&y, &cb, &cr);
  
    double nRand = ((double)(qrand() % (int)range) - range/2.0) / (col.sixteenBit() ? 65535.0 : 255.0);
    cb           = CLAMP(cb + nRand, 0.0, 1.0);
    cr           = CLAMP(cr + nRand, 0.0, 1.0);

    col.setYCbCr(y, cb, cr, col.sixteenBit());
}

double FilmGrainFilter::interpolate(int shadows, int midtones, int highlights, const DColor& col)
{
    double s = (shadows   +100)/200.0;
    double m = (midtones  +100)/200.0; 
    double h = (highlights+100)/200.0;

    double y, cb, cr;
    col.getYCbCr(&y, &cb, &cr);
        
    if (y >= 0.0 && y <= 0.5)
    {
        return (s+2*(m-s)*y);
    }
    else if (y >= 0.5 && y <= 1.0)
    {
        return (2*(h-m)*y+2*m-h);
    }
    else
    {
        return 1.0;
    }
}

}  // namespace Digikam
