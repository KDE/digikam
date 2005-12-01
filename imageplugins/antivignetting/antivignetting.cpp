/* ============================================================
 * File  : antivignetting.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Antivignetting threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original AntiVignetting algorithm copyrighted 2003 by 
 * John Walker from 'pnmctrfilt' implementation. See 
 * http://www.fourmilab.ch/netpbm/pnmctrfilt for more 
 * informations.
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
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "antivignetting.h"

namespace DigikamAntiVignettingImagesPlugin
{

AntiVignetting::AntiVignetting(QImage *orgImage, QObject *parent, double density, 
                               double power, double radius, int xshift, int yshift, bool normalize)
              : Digikam::ThreadedFilter(orgImage, parent, "AntiVignetting")
{ 
    m_density   = density;
    m_power     = power;
    m_radius    = radius;
    m_xshift    = xshift;
    m_yshift    = yshift;
    m_normalize = normalize;
    
    initFilter();
}

// This method is inspired from John Walker 'pnmctrfilt' algorithm code.

void AntiVignetting::filterImage(void)
{
    int     col, row, xd, td, yd, p;
    int     i, xsize, ysize, diagonal, erad, xctr, yctr;
    double *ldens;
    
    // Big/Little endian compatibility.
    int red, green, blue;
    Digikam::ImageFilters::imageData imageData; 
        
    uint* NewBits = (uint*)m_destImage.bits();
    uint* data    = (uint*)m_orgImage.bits();
    int   Width   = m_orgImage.width();
    int   Height  = m_orgImage.height();

    // Determine the radius of the filter.  This is the half diagonal
    // measure of the image multiplied by the command line radius factor. 
    
    xsize = (Height + 1) / 2;
    ysize = (Width + 1) / 2;
    erad = (int)((sqrt((xsize * xsize) + (ysize * ysize)) + 0.5) * m_radius);    
        
    // Build the in-memory table which maps distance from the
    // centre of the image (as adjusted by the X and Y offset,
    // if any) to the density of the filter at this remove.  This
    // table needs to be as large as the diagonal from the
    // (possibly offset) centre to the most distant corner
    // of the image. 

    xsize    = ((Height + 1) / 2) + abs(m_xshift);
    ysize    = ((Width + 1)  / 2) + abs(m_yshift);
    diagonal = ((int) (sqrt((xsize * xsize) + (ysize * ysize)) + 0.5)) +  1;
    
    ldens = new double[diagonal];
    
    for (i = 0 ; !m_cancel && (i < diagonal) ; i++)
        {
        if ( i >= erad )
           ldens[i] = 1;
        else 
           ldens[i] =  (1.0 + (m_density - 1) * pow(1.0 - (((double) i) / (erad - 1)), m_power));
        }

    xctr = ((Height + 1) / 2) + m_xshift;
    yctr = ((Width + 1) / 2) + m_yshift;
    
    for (row = 0 ; !m_cancel && (row < Width) ; row++) 
        {
        yd = abs(yctr - row);

        for (col = 0 ; !m_cancel && (col < Height) ; col++) 
            {
            p = col * Width + row;         

            xd = abs(xctr - col);
            td = (int) (sqrt((xd * xd) + (yd * yd)) + 0.5);

            imageData.raw = data[p];               
            red           = (int)imageData.channel.red;
            green         = (int)imageData.channel.green;
            blue          = (int)imageData.channel.blue;
            
            imageData.channel.red   = (uchar)(red   / ldens[td]);
            imageData.channel.green = (uchar)(green / ldens[td]);
            imageData.channel.blue  = (uchar)(blue  / ldens[td]);
            
            NewBits[p] = imageData.raw;
            }
        
        // Update the progress bar in dialog.
        
        if (m_parent)
           postProgress( (int) (((double)row * 100.0) / Width) );      
        }

    // FIXME to support 16 bits images properly

    // Normalize colors for a best rendering.   
    if (m_normalize)
       Digikam::ImageFilters::normalizeImage((uchar*)NewBits, Width, Height, false);
        
    delete [] ldens;        
}

}  // NameSpace DigikamAntiVignettingImagesPlugin
