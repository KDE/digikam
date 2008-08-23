/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * 
 * Original Sharpen algorithm copyright 2002
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
#include <cstdlib>

// Local includes.

#include "ddebug.h"
#include "dimgimagefilters.h"
#include "dimgsharpen.h"

namespace Digikam
{

DImgSharpen::DImgSharpen(DImg *orgImage, QObject *parent, double radius, double sigma)
           : DImgThreadedFilter(orgImage, parent, "Sharpen")
{ 
    m_radius = radius;
    m_sigma  = sigma;
    initFilter();
}

DImgSharpen::DImgSharpen(DImgThreadedFilter *parentFilter,
                         const DImg &orgImage, const DImg &destImage,
                         int progressBegin, int progressEnd, double radius, double sigma)
           : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                                parentFilter->filterName() + ": Sharpen")
{
    m_radius = radius;
    m_sigma  = sigma;
    // We need to provide support for orgImage == destImage.
    // The algorithm does not support this out of the box, so use a temporary.
    if (orgImage.bits() == destImage.bits())
        m_destImage = DImg(destImage.width(), destImage.height(), destImage.sixteenBit());
    filterImage();
    if (orgImage.bits() == destImage.bits())
        memcpy(destImage.bits(), m_destImage.bits(), m_destImage.numBytes());
}

void DImgSharpen::filterImage(void)
{
    sharpenImage(m_radius, m_sigma);
}

/** Function to apply the sharpen filter on an image*/

void DImgSharpen::sharpenImage(double radius, double sigma)
{
    if (m_orgImage.isNull())
    {
       DWarning() << k_funcinfo << "No image data available!"
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

    int kernelWidth = getOptimalKernelWidth(radius, sigma);
    
    if((int)m_orgImage.width() < kernelWidth)
    {
        DWarning() << k_funcinfo << "Image is smaller than radius!"
                    << endl;
        return;
    }
    
    double *kernel = new double[kernelWidth*kernelWidth];
    
    if(!kernel)
    {
        DWarning() << k_funcinfo << "Unable to allocate memory!"
                    << endl;
        return;
    }

    for(v=(-kernelWidth/2) ; v <= (kernelWidth/2) ; v++)
    {
        for(u=(-kernelWidth/2) ; u <= (kernelWidth/2) ; u++)
        {
            alpha      = exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
            kernel[i]  = alpha/(2.0*M_PI*sigma*sigma);
            normalize += kernel[i];
            i++;
        }
    }
    
    kernel[i/2] = (-2.0)*normalize;
    convolveImage(kernelWidth, kernel);
    
    delete [] kernel;
}

bool DImgSharpen::convolveImage(const unsigned int order, const double *kernel)
{
    uint    x, y;
    int     mx, my, sx, sy, mcx, mcy, progress;
    long    kernelWidth, i;
    double  red, green, blue, alpha, normalize=0.0;
    double *k=0;
    DColor  color;
    
    kernelWidth = order;
    
    if((kernelWidth % 2) == 0)
    {
        DWarning() << k_funcinfo << "Kernel width must be an odd number!"
                    << endl;
        return(false);
    }
    
    double *normal_kernel = new double[kernelWidth*kernelWidth];
    
    if(!normal_kernel)
    {
        DWarning() << k_funcinfo << "Unable to allocate memory!"
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

            m_destImage.setPixelColor(x, y, DColor((int)(red / 257UL),  (int)(green / 257UL),
                                                   (int)(blue / 257UL), (int)(alpha / 257UL),
                                                   m_destImage.sixteenBit()));
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

}  // NameSpace Digikam
