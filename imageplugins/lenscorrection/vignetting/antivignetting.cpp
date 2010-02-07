/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Antivignetting threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Original AntiVignetting algorithm copyrighted 2003 by
 * John Walker from 'pnmctrfilt' implementation. See
 * http://www.fourmilab.ch/netpbm/pnmctrfilt for more
 * information.
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

#include "antivignetting.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimgimagefilters.h"

namespace DigikamAntiVignettingImagesPlugin
{

AntiVignetting::AntiVignetting(DImg* orgImage, QObject* parent, double density,
                               double power, double radius, double yshift, double xshift,
                               bool normalize, bool addvignetting)
              : DImgThreadedFilter(orgImage, parent, "AntiVignetting")
{
    m_density        = density;
    m_power          = power;
    m_radius         = radius;
    m_xshift         = xshift;
    m_yshift         = yshift;
    m_normalize      = normalize;
    m_add_vignetting = addvignetting;

    initFilter();
}

// This method is inspired from John Walker 'pnmctrfilt' algorithm code.

void AntiVignetting::filterImage()
{
    int     progress;
    int     col, row, xd, td, yd, p;
    int     i, xsize, ysize, diagonal, erad, xctr, yctr;
    double *ldens;

    uchar* NewBits = m_destImage.bits();
    uchar* data    = m_orgImage.bits();

    unsigned short* NewBits16 = (unsigned short*)m_destImage.bits();
    unsigned short* data16    = (unsigned short*)m_orgImage.bits();

    int Width  = m_orgImage.width();
    int Height = m_orgImage.height();

    // Determine the shift in pixels from the shift in percentage.
    m_xshift = m_xshift*Height/200.0;
    m_yshift = m_yshift*Width/200.0;

    // Determine the radius of the filter.  This is the half diagonal
    // measure of the image multiplied by the command line radius factor.


    xsize = (Height + 1) / 2;
    ysize = (Width + 1) / 2;
    erad = approx(hypothenuse(xsize,ysize)) * m_radius;

    // Build the in-memory table which maps distance from the
    // center of the image (as adjusted by the X and Y offset,
    // if any) to the density of the filter at this remove.  This
    // table needs to be as large as the diagonal from the
    // (possibly offset) center to the most distant corner
    // of the image.

    xsize    = ((Height + 1) / 2) + abs(m_xshift);
    ysize    = ((Width  + 1) / 2) + abs(m_yshift);
    diagonal = approx(hypothenuse(xsize,ysize)) +  1;

    ldens = new double[diagonal];

    for (i = 0 ; !m_cancel && (i < diagonal) ; ++i)
    {
        if ( i >= erad )
        {
            if (!m_add_vignetting)
                ldens[i] = 1.0;
            else
                ldens[i] = 200000.0; // should be infinity
        }
        else
        {
            if (!m_add_vignetting)
                ldens[i] =  (1.0 + (m_density - 1) * pow(1.0 - (((double) i) / (erad - 1)), m_power));
            else
                ldens[i] =  20.0 / (1.0 + (24 - (m_density - 1)) * pow(1.0 - (((double) i) / (erad - 1)), m_power));
        }
    }

    xctr = ((Height + 1) / 2) + m_xshift;
    yctr = ((Width  + 1) / 2) + m_yshift;

    for (row = 0 ; !m_cancel && (row < Width) ; ++row)
    {
        yd = abs(yctr - row);

        for (col = 0 ; !m_cancel && (col < Height) ; ++col)
        {
            p = (col * Width + row)*4;

            xd = abs(xctr - col);
            td = (int) (sqrt((xd * xd) + (yd * yd)) + 0.5);

            if (!m_orgImage.sixteenBit())       // 8 bits image
            {
                NewBits[ p ] = clamp8bits(data[ p ] / ldens[td]);
                NewBits[p+1] = clamp8bits(data[p+1] / ldens[td]);
                NewBits[p+2] = clamp8bits(data[p+2] / ldens[td]);
                NewBits[p+3] = data[p+3];
            }
            else               // 16 bits image.
            {
                NewBits16[ p ] = clamp16bits(data16[ p ] / ldens[td]);
                NewBits16[p+1] = clamp16bits(data16[p+1] / ldens[td]);
                NewBits16[p+2] = clamp16bits(data16[p+2] / ldens[td]);
                NewBits16[p+3] = data16[p+3];
            }
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)row * 100.0) / Width);
        if (progress%5 == 0)
            postProgress( progress );
    }

    // Normalize colors for a best rendering.
    if (m_normalize)
    {
       DImgImageFilters filters;
       filters.normalizeImage(m_destImage.bits(), Width, Height, m_destImage.sixteenBit());
    }

    delete [] ldens;
}

int AntiVignetting::approx(double x)
{
    return ((int) x+0.5);
}

double AntiVignetting::hypothenuse(double x, double y)
{
    return (sqrt (x*x + y*y));
}

uchar AntiVignetting::clamp8bits(double x)
{
    if (x < 0)
        return 0;
    else if (x > 255)
        return 255;
    else
        return ((uchar) x);
}

unsigned short  AntiVignetting::clamp16bits(double x)
{
    if (x < 0)
        return 0;
    else if (x > 65535)
        return 65535;
    else
        return ((unsigned short) x);
}

}  // namespace DigikamAntiVignettingImagesPlugin
