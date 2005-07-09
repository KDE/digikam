/* ============================================================
 * File  : lensdistortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : LensDistortion threaded image filter.
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

#include "pixelaccess.h"
#include "lensdistortion.h"

namespace DigikamLensDistortionImagesPlugin
{

LensDistortion::LensDistortion(QImage *orgImage, QObject *parent, double main, 
                               double edge, double rescale, double brighten,
                               int centre_x, int centre_y)
              : Digikam::ThreadedFilter(orgImage, parent, "LensDistortion")
{ 
    m_main     = main;
    m_edge     = edge;
    m_rescale  = rescale;
    m_brighten = brighten;
    m_centre_x = centre_x;
    m_centre_y = centre_y;
    
    initFilter();
}

void LensDistortion::filterImage(void)
{
    memcpy ((uint*)m_destImage.bits(), (uint*)m_orgImage.bits(), m_orgImage.numBytes() );
    uint* data    = (uint*)m_destImage.bits();
    int   Width   = m_orgImage.width();
    int   Height  = m_orgImage.height();

    double normallise_radius_sq = 4.0 / (Width * Width + Height * Height);
    double centre_x             = Width * (100.0 + m_centre_x) / 200.0;
    double centre_y             = Height * (100.0 + m_centre_y) / 200.0;
    double mult_sq              = m_main / 200.0;
    double mult_qd              = m_edge / 200.0;
    double rescale              = pow(2.0, - m_rescale / 100.0);
    double brighten             = - m_brighten / 10.0;
    
    PixelAccess *pa = new PixelAccess(data, Width, Height);

    /*
     * start at image (i, j), increment by (step, step)
     * output goes to dst, which is w x h x d in size
     * NB: d <= image.bpp
     */
   
    // We working on the full image.
    int    i = 0, j = 0, dstWidth = Width, dstHeight = Height, dstDepth = 4;
    uchar* dst = (uchar*)data;
    int    step = 1, progress;
  
    int    dstI, dstJ;
    int    iLimit, jLimit;
    double srcX, srcY, mag;

    iLimit = i + dstWidth * step;
    jLimit = j + dstHeight * step;

    for (dstJ = j ; !m_cancel && (dstJ < jLimit) ; dstJ += step) 
       {
       for (dstI = i ; !m_cancel && (dstI < iLimit) ; dstI += step) 
          {
          // Get source Coordinates.
          double radius_sq;
          double off_x;
          double off_y;
          double radius_mult;

          off_x       = dstI - centre_x;
          off_y       = dstJ - centre_y;
          radius_sq   = (off_x * off_x) + (off_y * off_y);

          radius_sq  *= normallise_radius_sq;

          radius_mult = radius_sq * mult_sq + radius_sq * radius_sq * mult_qd;
          mag         = radius_mult;
          radius_mult = rescale * (1.0 + radius_mult);

          srcX        = centre_x + radius_mult * off_x;
          srcY        = centre_y + radius_mult * off_y;

          brighten = 1.0 + mag * brighten;
          pa->pixelAccessGetCubic(srcX, srcY, brighten, dst, dstDepth);
          dst += dstDepth;
          }
     
       // Update progress bar in dialog.
       
       progress = (int) (((double)dstJ * 100.0) / jLimit);
       if (m_parent && progress%5 == 0)
          postProgress(progress);
       }
       
    delete pa;
}

}  // NameSpace DigikamLensDistortionImagesPlugin
