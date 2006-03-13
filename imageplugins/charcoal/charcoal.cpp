/* ============================================================
 * File  : charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : Charcoal threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
 *
 * Original Charcoal algorithm copyright 2002
 * by Daniel M. Duley <mosfet@kde.org> from KImageEffect API.
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

#define SQ2PI   2.50662827463100024161235523934010416269302368164062
#define Epsilon 1.0e-12
 
// C++ includes. 
 
#include <cmath>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "charcoal.h"

namespace DigikamCharcoalImagesPlugin
{

Charcoal::Charcoal(Digikam::DImg *orgImage, QObject *parent, double pencil, double smooth)
        : Digikam::DImgThreadedFilter(orgImage, parent, "Charcoal")
{ 
    m_pencil = pencil;
    m_smooth = smooth;
    
    initFilter();
}

void Charcoal::filterImage(void)
{
    if (m_orgImage.isNull())
    {
       kdWarning() << k_funcinfo << "No image data available!"
                   << endl;
       return;
    }

    if (m_pencil <= 0.0)
    {
       m_destImage = m_orgImage;
       return;
    }

    // -- Applying Edge effect -----------------------------------------------

    register long i=0;
    int kernelWidth = getOptimalKernelWidth(m_pencil, m_smooth);
    
    if((int)m_orgImage.width() < kernelWidth)
    {
        kdWarning() << k_funcinfo << "Image is smaller than radius!"
                    << endl;
        return;
    }
    
    double *kernel = new double[kernelWidth*kernelWidth];
    
    if(!kernel)
    {
        kdWarning() << k_funcinfo << "Unable to allocate memory!"
                    << endl;
        return;
    }

    for(i = 0 ; i < (kernelWidth*kernelWidth) ; i++)
        kernel[i]=(-1.0);

    kernel[i/2]=kernelWidth*kernelWidth-1.0;
    convolveImage(kernelWidth, kernel);    
    delete [] kernel;

    // -- Applying Gaussian blur effect ---------------------------------------

    Digikam::DImgGaussianBlur(this, m_destImage, m_destImage, 50, 60, (int)(m_smooth/10.0));

    if (m_cancel)
        return;

    // -- Applying strech contrast color effect -------------------------------

    Digikam::DImgImageFilters().stretchContrastImage(m_destImage.bits(), m_destImage.width(), 
                                m_destImage.height(), m_destImage.sixteenBit());
    postProgress( 70 );
    if (m_cancel)
        return;

    // -- Inverting image color -----------------------------------------------

    Digikam::DImgImageFilters().invertImage(m_destImage.bits(), m_destImage.width(), 
                                m_destImage.height(), m_destImage.sixteenBit());
    postProgress( 80 );
    if (m_cancel)
        return;

    // -- Convert to neutral black & white ------------------------------------

    Digikam::DImgImageFilters().channelMixerImage(
                   m_destImage.bits(), m_destImage.width(), 
                   m_destImage.height(), m_destImage.sixteenBit(),  // Image data.
                   true,                                            // Preserve luminosity.    
                   true,                                            // Monochrome.
                   0.3, 0.59 , 0.11,                                // Red channel gains.
                   0.0, 1.0,   0.0,                                 // Green channel gains (not used).
                   0.0, 0.0,   1.0);                                // Blue channel gains (not used).
    postProgress( 90 );
    if (m_cancel)
        return;
}

bool Charcoal::convolveImage(const unsigned int order, const double *kernel)
{
    uint    x, y;
    int     mx, my, sx, sy, mcx, mcy, progress;
    long    kernelWidth, i;
    double  red, green, blue, alpha, normalize=0.0;
    double *k=0;
    Digikam::DColor  color;
    
    kernelWidth = order;
    
    if((kernelWidth % 2) == 0)
    {
        kdWarning() << k_funcinfo << "Kernel width must be an odd number!"
                    << endl;
        return(false);
    }
    
    double *normal_kernel = new double[kernelWidth*kernelWidth];
    
    if(!normal_kernel)
    {
        kdWarning() << k_funcinfo << "Unable to allocate memory!"
                    << endl;
        return(false);
    }
    
    for(i=0 ; i < (kernelWidth*kernelWidth) ; i++)
        normalize += kernel[i];
        
    if(fabs(normalize) <= Epsilon)
        normalize=1.0;
        
    normalize = 1.0/normalize;
    
    for(i=0 ; i < (kernelWidth*kernelWidth) ; i++)
        normal_kernel[i] = normalize*kernel[i];

    double maxClamp = m_destImage.sixteenBit() ? 16777215.0 : 65535.0;

    for(y=0 ; !m_cancel && (y < m_destImage.height()) ; y++)
    {
        sy = y-(kernelWidth/2);

        for(x=0 ; !m_cancel && (x < m_destImage.width()) ; x++)
        {
            k   = normal_kernel;
            red = green = blue = alpha = 0;
            sy  = y-(kernelWidth/2);
            
            for(mcy=0 ; !m_cancel && (mcy < kernelWidth) ; mcy++, sy++)
            {
                my = sy < 0 ? 0 : sy > (int)m_destImage.height()-1 ? m_destImage.height()-1 : sy;
                sx = x+(-kernelWidth/2);

                for(mcx=0 ; !m_cancel && (mcx < kernelWidth) ; mcx++, sx++)
                {
                    mx     = sx < 0 ? 0 : sx > (int)m_destImage.width()-1 ? m_destImage.width()-1 : sx;
                    color  = m_orgImage.getPixelColor(mx, my);
                    red   += (*k)*(color.red()   * 257.0);
                    green += (*k)*(color.green() * 257.0);
                    blue  += (*k)*(color.blue()  * 257.0);
                    alpha += (*k)*(color.alpha() * 257.0);
                    k++;
                }
            }

            red   =   red < 0.0 ? 0.0 :   red > maxClamp ? maxClamp :   red+0.5;
            green = green < 0.0 ? 0.0 : green > maxClamp ? maxClamp : green+0.5;
            blue  =  blue < 0.0 ? 0.0 :  blue > maxClamp ? maxClamp :  blue+0.5;
            alpha = alpha < 0.0 ? 0.0 : alpha > maxClamp ? maxClamp : alpha+0.5;

            m_destImage.setPixelColor(x, y, Digikam::DColor((int)(red / 257UL),  (int)(green / 257UL),
                                            (int)(blue / 257UL), (int)(alpha / 257UL),
                                            m_destImage.sixteenBit()));
        }

        progress = (int)(((double)y * 50.0) / m_destImage.height());
        if ( progress%5 == 0 )
           postProgress( progress );          
    }

    delete [] normal_kernel;
    return(true);
}

int Charcoal::getOptimalKernelWidth(double radius, double sigma)
{
    double        normalize, value;
    long          kernelWidth;
    register long u;

    if(radius > 0.0)
        return((int)(2.0*ceil(radius)+1.0));
        
    for(kernelWidth=5; ;)
    {
        normalize=0.0;
        
        for(u=(-kernelWidth/2) ; u <= (kernelWidth/2) ; u++)
            normalize += exp(-((double) u*u)/(2.0*sigma*sigma))/(SQ2PI*sigma);

        u     = kernelWidth/2;
        value = exp(-((double) u*u)/(2.0*sigma*sigma))/(SQ2PI*sigma)/normalize;
        
        if((long)(65535*value) <= 0)
            break;
            
        kernelWidth+=2;
    }
    
    return((int)kernelWidth-2);
}

}  // NameSpace DigikamCharcoalImagesPlugin
