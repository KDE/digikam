/* ============================================================
 * File  : dimgsharpen.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A DImgSharpen threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimgimagefilters.h"
#include "dimgsharpen.h"

namespace Digikam
{

DImgSharpen::DImgSharpen(DImg *orgImage, QObject *parent, double radius, double sigma)
           : Digikam::DImgThreadedFilter(orgImage, parent, "Sharpen")
{ 
    m_radius = radius;
    m_sigma  = sigma;
    initFilter();
}

void DImgSharpen::filterImage(void)
{
    sharpenImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                 m_orgImage.sixteenBit(), m_radius, m_sigma);
}

/** Function to apply the sharpen filter on an image*/

void DImgSharpen::sharpenImage(uchar *data, int width, int height, bool sixteenBit, double radius, double sigma)
{
    if (!data || !width || !height)
    {
       kdWarning() << k_funcinfo << "No image data available!"
                   << endl;
       return;
    }

    if (radius <= 0.0)
    {
       m_destImage = m_orgImage;
       return;
    }

    double        alpha, normalize=0.0;
    register long i=0, u, v;

    int optimalWidth = getOptimalKernelWidth(radius, sigma);
    
    if(width < optimalWidth)
    {
        kdWarning() << k_funcinfo << "Image is smaller than radius!"
                    << endl;
        return;
    }
    
    double *kernel = new double[optimalWidth*optimalWidth];
    
    if(!kernel)
    {
        kdWarning() << k_funcinfo << "Unable to allocate memory!"
                    << endl;
        return;
    }

    for(v=(-optimalWidth/2) ; v <= (optimalWidth/2) ; v++)
    {
        for(u=(-optimalWidth/2) ; u <= (optimalWidth/2) ; u++)
        {
            alpha      = exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
            kernel[i]  = alpha/(2.0*M_PI*sigma*sigma);
            normalize += kernel[i];
            i++;
        }
    }
    
    kernel[i/2] = (-2.0)*normalize;
    convolveImage(data, width, height, sixteenBit, optimalWidth, kernel);
    
    delete [] kernel;
}

bool DImgSharpen::convolveImage(uchar *data, int w, int h, bool sixteenBit,
                                const unsigned int order,
                                const double *kernel)
{
    uint    x, y;
    int     mx, my, sx, sy, mcx, mcy, progress;
    long    width, i;
    double  red, green, blue, alpha, normalize=0.0;
    double *k=0;
    DColor  color;
    
    width = order;
    
    if((width % 2) == 0)
    {
        kdWarning() << k_funcinfo << "Kernel width must be an odd number!"
                    << endl;
        return(false);
    }
    
    double *normal_kernel = new double[width*width];
    
    if(!normal_kernel)
    {
        kdWarning() << k_funcinfo << "Unable to allocate memory!"
                    << endl;
        return(false);
    }
    
    for(i=0 ; i < (width*width) ; i++)
        normalize += kernel[i];
        
    if(fabs(normalize) <= Epsilon)
        normalize=1.0;
        
    normalize = 1.0/normalize;
    
    for(i=0 ; i < (width*width) ; i++)
        normal_kernel[i] = normalize*kernel[i];

    uchar *q = m_destImage.bits();
    
    for(y=0 ; y < m_destImage.height() ; y++)
    {
        sy = y-(width/2);

        for(x=0 ; x < m_destImage.width() ; x++)
        {
            k   = normal_kernel;
            red = green = blue = alpha = 0;
            sy  = y-(width/2);
            
            for(mcy=0 ; mcy < width ; mcy++, sy++)
            {
                my = sy < 0 ? 0 : sy > h-1 ? h-1 : sy;
                sx = x+(-width/2);
                
                for(mcx=0 ; mcx < width ; mcx++, sx++)
                {
                    mx     = sx < 0 ? 0 : sx > w-1 ? w-1 : sx;
                    color  = m_orgImage.getPixelColor(mx, my);
                    red   += (*k)*(color.red()   * 257.0);
                    green += (*k)*(color.green() * 257.0);
                    blue  += (*k)*(color.blue()  * 257.0);
                    alpha += (*k)*(color.alpha() * 257.0);
                    k++;
                }
            }

            red   =   red < 0.0 ? 0.0 :   red > 65535.0 ? 65535.0 :   red+0.5;
            green = green < 0.0 ? 0.0 : green > 65535.0 ? 65535.0 : green+0.5;
            blue  =  blue < 0.0 ? 0.0 :  blue > 65535.0 ? 65535.0 :  blue+0.5;
            alpha = alpha < 0.0 ? 0.0 : alpha > 65535.0 ? 65535.0 : alpha+0.5;

            q[0] = (uchar)(blue  / 257UL);
            q[1] = (uchar)(green / 257UL);
            q[2] = (uchar)(red   / 257UL);
            q[3] = (uchar)(alpha / 257UL);
            q+=4;
        }
        
        progress = (int)(((double)y * 100.0) / m_destImage.height());
        if ( progress%5 == 0 )
           postProgress( progress );          
    }

    delete [] normal_kernel;
    return(true);
}

int DImgSharpen::getOptimalKernelWidth(double radius, double sigma)
{
    double        normalize, value;
    long          width;
    register long u;

    if(radius > 0.0)
        return((int)(2.0*ceil(radius)+1.0));
        
    for(width=5; ;)
    {
        normalize=0.0;
        
        for(u=(-width/2) ; u <= (width/2) ; u++)
            normalize += exp(-((double) u*u)/(2.0*sigma*sigma))/(SQ2PI*sigma);

        u     = width/2;
        value = exp(-((double) u*u)/(2.0*sigma*sigma))/(SQ2PI*sigma)/normalize;
        
        if((long)(65535*value) <= 0)
            break;
            
        width+=2;
    }
    
    return((int)width-2);
}

}  // NameSpace Digikam
