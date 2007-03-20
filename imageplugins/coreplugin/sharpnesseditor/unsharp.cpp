/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier 
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

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dcolor.h"
#include "dimgimagefilters.h"
#include "dimggaussianblur.h"
#include "unsharp.h"

namespace DigikamImagesPluginCore
{

UnsharpMask::UnsharpMask(Digikam::DImg *orgImage, QObject *parent, int radius, 
                         double amount, double threshold)
           : DImgThreadedFilter(orgImage, parent, "UnsharpMask")
{ 
    m_radius    = radius;
    m_amount    = amount;
    m_threshold = threshold;
    initFilter();
}

void UnsharpMask::filterImage(void)
{
    int    progress;
    int    quantum;
    double quantumThreshold;
    double value;
    Digikam::DColor p; 
    Digikam::DColor q;

    if (m_orgImage.isNull())
    {
       DWarning() << k_funcinfo << "No image data available!" << endl;
       return;
    }

    Digikam::DImgGaussianBlur(this, m_orgImage, m_destImage, 0, 10, (int)(m_radius));

    quantum          = m_destImage.sixteenBit() ? 65535 : 255;
    quantumThreshold = quantum*m_threshold;

    for (uint y = 0 ; !m_cancel && (y < m_destImage.height()) ; y++)
    {
        for (uint x = 0 ; !m_cancel && (x < m_destImage.width()) ; x++)
        {
            p = m_orgImage.getPixelColor(x, y);
            q = m_destImage.getPixelColor(x, y);

            // Red channel.
            value = (double)(p.red())-(double)(q.red());
    
            if (fabs(2.0*value) < quantumThreshold)
                value = (double)(p.red());
            else
                value = (double)(p.red()) + value*m_amount;

            q.setRed(CLAMP(ROUND(value), 0, quantum));
    
            // Green Channel.
            value = (double)(p.green())-(double)(q.green());
    
            if (fabs(2.0*value) < quantumThreshold)
                value = (double)(p.green());
            else
                value = (double)(p.green()) + value*m_amount;

            q.setGreen(CLAMP(ROUND(value), 0, quantum));
            
            // Blue Channel.
            value = (double)(p.blue())-(double)(q.blue());
    
            if (fabs(2.0*value) < quantumThreshold)
                value = (double)(p.blue());
            else
                value = (double)(p.blue()) + value*m_amount;

            q.setBlue(CLAMP(ROUND(value), 0, quantum));
        
            // Alpha Channel.
            value = (double)(p.alpha())-(double)(q.alpha());
    
            if (fabs(2.0*value) < quantumThreshold)
                value = (double)(p.alpha());
            else
                value = (double)(p.alpha()) + value*m_amount;

            q.setAlpha(CLAMP(ROUND(value), 0, quantum));

            m_destImage.setPixelColor(x, y, q);        
        }

        progress = (int)(10.0 + ((double)y * 90.0) / m_destImage.height());
        if ( progress%5 == 0 )
           postProgress( progress );   
    }
}

}  // NameSpace DigikamImagesPluginCore
