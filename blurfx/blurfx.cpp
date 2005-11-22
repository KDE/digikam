/* ============================================================
 * File  : texture.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Texture threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * 
 * Original Blur algorithms copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

// Represents 1º 
#define ANGLE_RATIO  0.017453292519943295769236907685   
  
// C++ includes. 
 
#include <cmath>
#include <cstdlib>
#include <cstring>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "blurfx.h"

namespace DigikamBlurFXImagesPlugin
{

BlurFX::BlurFX(QImage *orgImage, QObject *parent, int blurFXType, int distance, int level)
      : Digikam::ThreadedFilter(orgImage, parent, "BlurFX")
{ 
    m_blurFXType = blurFXType;
    m_distance   = distance;
    m_level      = level;
    
    initFilter();
}

void BlurFX::filterImage(void)
{
    int   w     = m_orgImage.width();
    int   h     = m_orgImage.height();
    uchar* data = m_orgImage.bits();

    switch (m_blurFXType)
        {
        case ZoomBlur:
            zoomBlur(data, w, h, w/2, h/2, m_distance);
            break;
    
        case RadialBlur:
            radialBlur(data, w, h, w/2, h/2, m_distance);
            break;
    
        case FarBlur:
            farBlur(data, w, h, m_distance);
            break;
    
        case MotionBlur:
            motionBlur(data, w, h, m_distance, (double)m_level);
            break;
    
        case SoftenerBlur:
            softenerBlur(data, w, h);
            break;
        
        case ShakeBlur:
            shakeBlur(data, w, h, m_distance);
            break;
        
        case FocusBlur:
            focusBlur(data, w, h, w/2, h/2, m_distance, m_level*10);
            break;
    
        case SmartBlur:
            smartBlur(data, w, h, m_distance, m_level);
            break;
                    
        case FrostGlass:
            frostGlass(data, w, h, m_distance);
            break;
    
        case Mosaic:
            mosaic(data, w, h, m_distance, m_distance);
            break;                    
        }
}

/* Function to apply the ZoomBlur effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * X, Y             => Center of zoom in the image
 * Distance         => Distance value         
 * pArea            => Preview area.
 *                                                                                  
 * Theory           => Here we have a effect similar to RadialBlur mode Zoom from  
 *                     Photoshop. The theory is very similar to RadialBlur, but has one
 *                     difference. Instead we use pixels with the same radius and      
 *                     near angles, we take pixels with the same angle but near radius 
 *                     This radius is always from the center to out of the image, we   
 *                     calc a proportional radius from the center.
 */
void BlurFX::zoomBlur(uchar *data, int Width, int Height, int X, int Y, int Distance, QRect pArea)
{
    if (Distance <= 1) return;
    int progress;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;
    int nStride = GetStride(Width);
            
    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )   
       {
       xMin = pArea.x();
       xMax = pArea.x() + pArea.width();
       yMin = pArea.y();
       yMax = pArea.y() + pArea.height();
       nStride = (Width - xMax + xMin)*4;
       }
       
    uchar* pResBits = m_destImage.bits(); 

    register int h, w, nh, nw, i, j, r;
    int sumR, sumG, sumB, nCount;
    double lfRadius, lfNewRadius, lfRadMax, lfAngle;
    
    lfRadMax = sqrt (Height * Height + Width * Width);

    // total of bits to be taken is given by this formula
    nCount = 0;

    // we have to initialize all loop and positions valiables
    i = yMin * LineWidth + xMin * 4;
    j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    for (h = yMin; !m_cancel && (h < yMax); h++, i += nStride)
        {
        for (w = xMin; !m_cancel && (w < xMax); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            nw = X - w;
            nh = Y - h;

            lfRadius = sqrt (nw * nw + nh * nh);
            lfAngle = atan2 (nh, nw);
            lfNewRadius = (lfRadius * Distance) / lfRadMax;

            for (r = 0; !m_cancel && (r <= lfNewRadius); r++)
                {
                // we need to calc the positions
                nw = (int)(X - (lfRadius - r) * cos (lfAngle));
                nh = (int)(Y - (lfRadius - r) * sin (lfAngle));

                if (IsInside(Width, Height, nw, nh))
                    {
                    // we adjust the positions
                    j = SetPosition(Width, nw, nh);
                    // finally we sum the bits
                    sumR += data[ j ];
                    sumG += data[j+1];
                    sumB += data[j+2];
                    nCount++;
                    }
                }
            
            if (nCount == 0) nCount = 1;                    

            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)(h - yMin) * 100.0) / (yMax - yMin));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the radialBlur effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.     
 * X, Y             => Center of radial in the image                       
 * Distance         => Distance value                                            
 * pArea            => Preview area.
 *                                                                                  
 * Theory           => Similar to RadialBlur from Photoshop, its an amazing effect    
 *                     Very easy to understand but a little hard to implement.           
 *                     We have all the image and find the center pixel. Now, we analize
 *                     all the pixels and calc the radius from the center and find the 
 *                     angle. After this, we sum this pixel with others with the same  
 *                     radius, but different angles. Here I'm using degrees angles.
 */
void BlurFX::radialBlur(uchar *data, int Width, int Height, int X, int Y, int Distance, QRect pArea)
{
    if (Distance <= 1) return;
    int progress;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;
    int nStride = GetStride(Width);
            
    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )   
       {
       xMin = pArea.x();
       xMax = pArea.x() + pArea.width();
       yMin = pArea.y();
       yMax = pArea.y() + pArea.height();
       nStride = (Width - xMax + xMin)*4;
       }

    uchar* pResBits = m_destImage.bits(); 
    
    register int sumR, sumG, sumB, i, j, nw, nh;
    double Radius, Angle, AngleRad;
    
    double *nMultArray = new double[Distance * 2 + 1];
    
    for (i = -Distance; i <= Distance; i++)
        nMultArray[i + Distance] = i * ANGLE_RATIO;

    // total of bits to be taken is given by this formula
    int nCount = 0;

    // we have to initialize all loop and positions valiables
    i = yMin * LineWidth + xMin * 4;
    j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    
    for (int h = yMin; !m_cancel && (h < yMax); h++, i += nStride)
        {
        for (int w = xMin; !m_cancel && (w < xMax); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            nw = X - w;
            nh = Y - h;

            Radius = sqrt (nw * nw + nh * nh);
            AngleRad = atan2 (nh, nw);
            
            for (int a = -Distance; !m_cancel && (a <= Distance); a++)
                {
                Angle = AngleRad + nMultArray[a + Distance];
                // we need to calc the positions
                nw = (int)(X - Radius * cos (Angle));
                nh = (int)(Y - Radius * sin (Angle));

                if (IsInside(Width, Height, nw, nh))
                    {
                    // we adjust the positions
                    j = SetPosition (Width, nw, nh);
                    // finally we sum the bits
                    sumR += data[ j ];
                    sumG += data[j+1];
                    sumB += data[j+2];
                    nCount++;
                    }
                }

            if (nCount == 0) nCount = 1;                    
                
            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;
            }

        // Update the progress bar in dialog.
        progress = (int) (((double)(h - yMin) * 100.0) / (yMax - yMin));
        
        if (progress%5 == 0)
           postProgress(progress);
        }

    delete [] nMultArray;
}

/* Function to apply the focusBlur effect backported from ImageProcessing version 2                                              
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * BlurRadius       => Radius of blured image. 
 * BlendRadius      => Radius of blending effect.
 * bInversed        => If true, invert focus effect.
 * pArea            => Preview area.
 *                                                                                 
 */
void BlurFX::focusBlur(uchar *data, int Width, int Height, int X, int Y, int BlurRadius, int BlendRadius, 
                       bool bInversed, QRect pArea)
{
    int progress;
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    // We working on full image.
    int xMin = 0;
    int xMax = Width;
    int yMin = 0;
    int yMax = Height;
    int nStride = GetStride(Width);
            
    // If we working in preview mode, else we using the preview area.
    if ( pArea.isValid() )   
       {
       xMin = pArea.x();
       xMax = pArea.x() + pArea.width();
       yMin = pArea.y();
       yMax = pArea.y() + pArea.height();
       nStride = (Width - xMax + xMin)*4;
       }
    
    int    BitCount = LineWidth * Height;
    uchar* pResBits = m_destImage.bits(); 
                
    // Gaussian blur using the BlurRadius parameter.
    
    memcpy (pResBits, data, BitCount);        
    
    // Gaussian kernel computation.
    
    int    nKSize, nCenter;
    double x, sd, factor, lnsd, lnfactor;
    register int i, j, n, h, w;

    nKSize = 2 * BlurRadius + 1;
    nCenter = nKSize / 2;
    int *Kernel = new int[nKSize];

    lnfactor = (4.2485 - 2.7081) / 10 * nKSize + 2.7081;
    lnsd = (0.5878 + 0.5447) / 10 * nKSize - 0.5447;
    factor = exp (lnfactor);
    sd = exp (lnsd);

    for (i = 0; i < nKSize; i++)
        {
        x = sqrt ((i - nCenter) * (i - nCenter));
        Kernel[i] = (int)(factor * exp (-0.5 * pow ((x / sd), 2)) / (sd * sqrt (2.0 * M_PI)));
        }
    
    // Apply Gaussian kernel.
    
    int nSumR, nSumG, nSumB, nCount;
    int nKernelWidth = BlurRadius * 2 + 1;
    
    uchar* pInBits  = pResBits;
    uchar* pOutBits = new uchar[BitCount];
    uchar* pBlur    = new uchar[BitCount];
    
    // We need to copy our bits to blur bits
    
    memcpy (pBlur, pInBits, BitCount);     

    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = Alloc2DArray (nKernelWidth, 256);
    
    for (i = 0; i < nKernelWidth; i++)
        for (j = 0; j < 256; j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to initialize all the loop and iterator variables
    
    nSumR = nSumG = nSumB = nCount = j = 0;
    i = yMin * LineWidth + xMin * 4;
    
    // Now, we enter in the main loop
    
    for (h = yMin; !m_cancel && (h < yMax); h++, i += nStride)
        {
        for (w = xMin; !m_cancel && (w < xMax); w++, i += 4)
            {
            // first of all, we need to blur the horizontal lines
                
            for (n = -BlurRadius; !m_cancel && (n <= BlurRadius); n++)
               {
               // if is inside...
               if (IsInside (Width, Height, w + n, h))
                    {
                    // we points to the pixel
                    j = i + n * 4;
                    
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + BlurRadius][pInBits[j+2]];
                    nSumG += arrMult[n + BlurRadius][pInBits[j+1]];
                    nSumB += arrMult[n + BlurRadius][pInBits[ j ]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + BlurRadius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to blur bits the horizontal blur values
            pBlur[i+2] = LimitValues (nSumR / nCount);
            pBlur[i+1] = LimitValues (nSumG / nCount);
            pBlur[ i ] = LimitValues (nSumB / nCount);
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)(h - yMin) * 25.0) / (yMax - yMin));
        
        if (progress%5 == 0)
           postProgress(progress);
        }

    // Getting the blur bits, we initialize position variables
    j = 0;
    i = yMin * LineWidth + xMin * 4;

    // We enter in the second main loop
        
    for (h = yMin; !m_cancel && (h < yMax); h++, i += nStride)
        {
        for (w = xMin; !m_cancel && (w < xMax); w++, i += 4)
            {
            // first of all, we need to blur the vertical lines
            for (n = -BlurRadius; !m_cancel && (n <= BlurRadius); n++)
                {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                    {
                    // we points to the pixel
                    j = i + n * 4;
                                          
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + BlurRadius][pBlur[j+2]];
                    nSumG += arrMult[n + BlurRadius][pBlur[j+1]];
                    nSumB += arrMult[n + BlurRadius][pBlur[ j ]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + BlurRadius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to bits the vertical blur values
            pOutBits[i+2] = LimitValues (nSumR / nCount);
            pOutBits[i+1] = LimitValues (nSumG / nCount);
            pOutBits[ i ] = LimitValues (nSumB / nCount);
                
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (25.0 + ((double)(h - yMin) * 25.0) / (yMax - yMin));
        
        if (progress%5 == 0)
           postProgress(progress);     
        }

    memcpy (pResBits, pOutBits, BitCount);   
       
    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
    delete [] pOutBits;
    delete [] Kernel;        
        
    // Blending results.  
    
    int nBlendFactor;
    double lfRadius;
        
    register int nh, nw = 0;
    i = yMin * LineWidth + xMin * 4;
    
    for (h = yMin; !m_cancel && (h < yMax); h++, i += nStride)
        {
        nh = Y - h;

        for (w = xMin; !m_cancel && (w < xMax); w++)
            {
            nw = X - w;

            lfRadius = sqrt (nh * nh + nw * nw);

            nBlendFactor = LimitValues ((int)(255.0 * lfRadius / (double)BlendRadius));

            if (bInversed)
                {
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + data[i] * nBlendFactor) >> 8;    // Blue.
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + data[i] * nBlendFactor) >> 8;    // Green.
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + data[i] * nBlendFactor) >> 8;    // Red.
                }
            else
                {
                pResBits[i++] = (data[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Blue.
                pResBits[i++] = (data[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Green.
                pResBits[i++] = (data[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Red.
                }
                
            pResBits[i++] = data[i];       // Alpha channel.
            }
        
        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)(h - yMin) * 50.0) / (yMax - yMin));
        
        if (progress%5 == 0)
           postProgress(progress);              
        }
}

/* Function to apply the farBlur effect backported from ImageProcessing version 2                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance value                                            
 *                                                                                  
 * Theory           => This is an interesting effect, the blur is applied in that   
 *                     way: (the value "1" means pixel to be used in a blur calc, ok?)
 *                     e.g. With distance = 2 
 *                                            |1|1|1|1|1| 
 *                                            |1|0|0|0|1| 
 *                                            |1|0|C|0|1| 
 *                                            |1|0|0|0|1| 
 *                                            |1|1|1|1|1| 
 *                     We sum all the pixels with value = 1 and apply at the pixel with*
 *                     the position "C".
 */
void BlurFX::farBlur(uchar *data, int Width, int Height, int Distance)
{
    if (Distance < 1) return;
    
    // we need to create our kernel
    // e.g. distance = 3, so kernel={3 1 1 2 1 1 3}
    
    int *nKern = new int[Distance * 2 + 1];
    
    for (int i = 0; i < Distance * 2 + 1; i++)
        {
        // the first element is 3
        if (i == 0)
            nKern[i] = 2;
        // the center element is 2
        else if (i == Distance)
            nKern[i] = 3;
        // the last element is 3
        else if (i == Distance * 2)
            nKern[i] = 3;
        // all other elements will be 1
        else
            nKern[i] = 1;
        }

    // now, we apply a convolution with kernel
    MakeConvolution(data, Width, Height, Distance, nKern);

    // we must delete to free memory
    delete [] nKern;
}

/* Function to apply the SmartBlur effect                                           
 *                                                                                  
 * data             => The image data in RGBA mode.  
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Radius           => blur matrix radius.                                         
 * Strenght         => Color strenght.                                         
 *                                                                                  
 * Theory           => Similar to SmartBlur from Photoshop, this function has the   
 *                     same engine as Blur function, but, in a matrix with n        
 *                     dimentions, we take only colors that pass by sensibility filter
 *                     The result is a clean image, not totally blurred, but a image  
 *                     with correction between pixels.      
 */

void BlurFX::smartBlur(uchar *data, int Width, int Height, int Radius, int Strenght)
{
    if (Radius <= 0) return;
    
    int progress;
    int nStride = GetStride (Width);
    register int sumR, sumG, sumB, nCount, i, j, w, h, a;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar* pResBits = m_destImage.bits(); 
    uchar* pBlur    = new uchar[BitCount];
    
    // We need to copy our bits to blur bits
    
    memcpy (pBlur, data, BitCount);     
    
    // we have to initialize all loop and positions valiables
    i = j = sumR = sumG = sumB = nCount = 0;

    // we have reached the main loop
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            for (a = -Radius; !m_cancel && (a <= Radius); a++)
                {
                // verify if is inside the rect
                if (IsInside( Width, Height, w + a, h))
                    {
                    // we need to find the pixel's position
                    j = i + a * 4;
                                                
                    // now, we have to check if is inside the sensibility filter
                    if (IsColorInsideTheRange (data[i+2], data[i+1], data[i],
                                               data[j+2], data[j+1], data[j],
                                               Strenght))
                        {
                        // finally we sum the bits
                        sumR += data[j+2];
                        sumG += data[j+1];
                        sumB += data[ j ];
                        }
                    else
                        {
                        // finally we sum the bits
                        sumR += data[i+2];
                        sumG += data[i+1];
                        sumB += data[ i ];
                        }

                    // increment counter
                    nCount++;
                    }
                }

                // now, we have to calc the arithmetic average
                pBlur[i+2] = sumR / nCount;
                pBlur[i+1] = sumG / nCount;
                pBlur[ i ] = sumB / nCount;
                
                // we initialize the variables
                sumR = sumG = sumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);              
        }
    
    // we need to initialize position's variables
    i = j = 0;

    // we have reached the second part of main loop
    
    for (w = 0; !m_cancel && (w < Width); w++, i = w * 4)
        {
        for (h = 0;!m_cancel && ( h < Height); h++, i += LineWidth)
            {
            // ...we enter this loop to sum the bits
            for (a = -Radius; !m_cancel && (a <= Radius); a++)
                {
                // verify if is inside the rect
                    if (IsInside (Width, Height, w, h + a))
                    {
                    // we need to find the pixel's position
                    j = i + a * LineWidth;
                        
                    // now, we have to check if is inside the sensibility filter
                    if (IsColorInsideTheRange (data[i+2], data[i+1], data[i],
                                               data[j+2], data[j+1], data[j],
                                               Strenght))
                        {
                        // finally we sum the bits
                        sumR += pBlur[j+2];
                        sumG += pBlur[j+1];
                        sumB += pBlur[ j ];
                        }
                    else
                        {
                        // finally we sum the bits
                        sumR += data[i+2];
                        sumG += data[i+1];
                        sumB += data[ i ];
                        }

                    // increment counter
                    nCount++;
                    }
                }

                // now, we have to calc the arithmetic average
                pResBits[i+2] = sumR / nCount;
                pResBits[i+1] = sumG / nCount;
                pResBits[ i ] = sumB / nCount;
                
                // we initialize the variables
                sumR = sumG = sumB = nCount = 0;
            }
    
        
        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)w * 50.0) / Width);
        
        if (progress%5 == 0)
           postProgress(progress);              
        }

    // now, we must free memory
    delete [] pBlur;
}

/* Function to apply the motionBlur effect backported from ImageProcessing version 2                                              
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance value 
 * Angle            => Angle direction (degrees)                                               
 *                                                                                  
 * Theory           => Similar to MotionBlur from Photoshop, the engine is very       
 *                     simple to undertand, we take a pixel (duh!), with the angle we   
 *                     will taking near pixels. After this we blur (add and do a       
 *                     division).
 */
void BlurFX::motionBlur(uchar *data, int Width, int Height, int Distance, double Angle)
{
    if (Distance == 0) return;
    int progress;
    
    // we try to avoid division by 0 (zero)
    if (Angle == 0.0) Angle = 360.0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int nStride = GetStride(Width);
    register int sumR, sumG, sumB, nCount, i, j;
    double nAngX, nAngY, nw, nh;

    uchar* pResBits = m_destImage.bits(); 

    // we initialize cos and sin for a best performance
    nAngX = cos ((2.0 * M_PI) / (360.0 / Angle));
    nAngY = sin ((2.0 * M_PI) / (360.0 / Angle));
    
    // total of bits to be taken is given by this formula
    nCount = Distance * 2 + 1;

    // we will alloc size and calc the possible results
    double *lpXArray = new double[nCount];
    double *lpYArray = new double[nCount];
    
    for (i = 0; i < nCount; i++)
        {
        lpXArray[i] = (i - Distance) * nAngX;
        lpYArray[i] = (i - Distance) * nAngY;
        }

    // we have to initialize all loop and positions valiables
    i = j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    
    for (int h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (int w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            for (int a = -Distance; !m_cancel && (a <= Distance); a++)
                {
                // we need to calc the positions
                nw = ((double)w + lpXArray[a + Distance]);
                nh = ((double)h + lpYArray[a + Distance]);
                    
                // we adjust the positions
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);
                // finally we sum the bits
                sumR += data[ j ];
                sumG += data[j+1];
                sumB += data[j+2];
                }
            
            if (nCount == 0) nCount = 1;                    

            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = 0;
            
            pResBits[i+3] = data[i+3];         // Alpha channel.
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);              
        }

    delete [] lpXArray;
    delete [] lpYArray;
}

/* Function to apply the softenerBlur effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 *                                                                                  
 * Theory           => An interesting blur-like function. In dark tones we apply a   
 *                     blur with 3x3 dimentions, in light tones, we apply a blur with   
 *                     5x5 dimentions. Easy, hun?
 */
void BlurFX::softenerBlur(uchar *data, int Width, int Height)
{
    int progress;
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    int SomaR = 0, SomaG = 0, SomaB = 0;
    int i, j, Gray;
        
    for (int h = 0; !m_cancel && (h < Height); h++)
        {
        for (int w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            Gray = (data[i+2] + data[i+1] + data[i]) / 3;
            
            if (Gray > 127)
                {
                for (int a = -3; !m_cancel && (a <= 3); a++)
                    {
                    for (int b = -3; !m_cancel && (b <= 3); b++)
                        {
                        j = (h + Lim_Max (h, a, Height)) * LineWidth + 4 * (w + Lim_Max (w, b, Width));  
                        
                        if ((h + a < 0) || (w + b < 0))
                           j = i;
                        
                        SomaR += data[j+2];
                        SomaG += data[j+1];
                        SomaB += data[ j ];
                        }
                    } 
                   
                data[i+2] = SomaR / 49;
                data[i+1] = SomaG / 49;
                data[ i ] = SomaB / 49;
                SomaR = SomaG = SomaB = 0;
                }
            else
                {
                for (int a = -1; !m_cancel && (a <= 1); a++)
                    {
                    for (int b = -1; !m_cancel && (b <= 1); b++)
                        {
                        j = (h + Lim_Max (h, a, Height)) * LineWidth + 4 * (w + Lim_Max (w, b, Width));
                        
                        if ((h + a < 0) || (w + b < 0))
                           j = i;
                        
                        SomaR += data[j+2];
                        SomaG += data[j+1];
                        SomaB += data[ j ];
                        }
                    }
                                
                data[i+2] = SomaR / 9;
                data[i+1] = SomaG / 9;
                data[ i ] = SomaB / 9;
                SomaR = SomaG = SomaB = 0;
                }
            }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }
    
    if (!m_cancel) 
       memcpy (m_destImage.bits(), data, Width*Height*sizeof(int));        
}

/* Function to apply the shake blur effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance between layers (from origin)                        
 *                                                                                  
 * Theory           => Similar to Fragment effect from Photoshop. We create 4 layers
 *                    each one has the same distance from the origin, but have       
 *                    different positions (top, botton, left and right), with these 4 
 *                    layers, we join all the pixels.                 
 */
void BlurFX::shakeBlur(uchar *data, int Width, int Height, int Distance)
{
    int progress;
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar* Layer1 = new uchar[BitCount];
    uchar* Layer2 = new uchar[BitCount];
    uchar* Layer3 = new uchar[BitCount];
    uchar* Layer4 = new uchar[BitCount];
    
    register int i = 0, j = 0, h, w, nw, nh;
        
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;

            nh = (h + Distance >= Height) ? Height - 1 : h + Distance;
            j = nh * LineWidth + 4 * w;
            Layer1[i+2] = data[j+2];
            Layer1[i+1] = data[j+1];
            Layer1[ i ] = data[ j ];

            nh = (h - Distance < 0) ? 0 : h - Distance;
            j = nh * LineWidth + 4 * w;
            Layer2[i+2] = data[j+2];
            Layer2[i+1] = data[j+1];
            Layer2[ i ] = data[ j ];

            nw = (w + Distance >= Width) ? Width - 1 : w + Distance;
            j = h * LineWidth + 4 * nw;
            Layer3[i+2] = data[j+2];
            Layer3[i+1] = data[j+1];
            Layer3[ i ] = data[ j ];

            nw = (w - Distance < 0) ? 0 : w - Distance;
            j = h * LineWidth + 4 * nw;
            Layer4[i+2] = data[j+2];
            Layer4[i+1] = data[j+1];
            Layer4[ i ] = data[ j ];
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }
        
    for (int h = 0; !m_cancel && (h < Height); h++)
        {
        for (int w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            data[i+2] = (Layer1[i+2] + Layer2[i+2] + Layer3[i+2] + Layer4[i+2]) / 4;
            data[i+1] = (Layer1[i+1] + Layer2[i+1] + Layer3[i+1] + Layer4[i+1]) / 4;
            data[ i ] = (Layer1[ i ] + Layer2[ i ] + Layer3[ i ] + Layer4[ i ]) / 4;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)h * 50.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }

    if (!m_cancel) 
       memcpy (m_destImage.bits(), data, BitCount);        
            
    delete [] Layer1;
    delete [] Layer2;
    delete [] Layer3;
    delete [] Layer4;
}

/* Function to apply the frostGlass effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Frost            => Frost value 
 *                                                                                  
 * Theory           => Similar to Diffuse effect, but the random byte is defined   
 *                     in a matrix. Diffuse uses a random diagonal byte.
 */
void BlurFX::frostGlass(uchar *data, int Width, int Height, int Frost)
{
    int progress;
    Frost = (Frost < 1) ? 1 : (Frost > 10) ? 10 : Frost;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar* NewBits = m_destImage.bits(); 

    register int i = 0, h, w; 
    QRgb color;
        
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            color = RandomColor (data, Width, Height, w, h, Frost);
            NewBits[ i ] = qRed(color);
            NewBits[i+1] = qGreen(color);
            NewBits[i+2] = qBlue(color);
            }
            
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }
}

/* Function to apply the mosaic effect backported from ImageProcessing version 2                                             
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Size             => Size of mosaic .
 *                                                                                  
 * Theory           => Ok, you can find some mosaic effects on PSC, but this one   
 *                     has a great feature, if you see a mosaic in other code you will
 *                     see that the corner pixel doesn't change. The explanation is   
 *                     simple, the color of the mosaic is the same as the first pixel 
 *                     get. Here, the color of the mosaic is the same as the mosaic   
 *                     center pixel. 
 *                     Now the function scan the rows from the top (like photoshop).
 */
void BlurFX::mosaic(uchar *data, int Width, int Height, int SizeW, int SizeH)
{
    int progress;
    
    // we need to check for valid values
    if (SizeW < 1) SizeW = 1;
    if (SizeH < 1) SizeH = 1;
    if ((SizeW == 1) && (SizeH == 1)) return;

    int i, j, k;            
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar* pResBits = m_destImage.bits(); 
    
    // this loop will never look for transparent colors
    
    for (int h = 0; !m_cancel && (h < Height); h += SizeH)
        {
        for (int w = 0; !m_cancel && (w < Width); w += SizeW)
            {
            // we store the top-left corner position
            i = k = SetPosition(Width, w, h);
            
            // now, we have to find the center pixel for mosaic's rectangle
            
            j = SetPositionAdjusted(Width, Height, w + (SizeW / 2), h + (SizeH / 2));

            // now, we fill the mosaic's rectangle with the center pixel color
            
            for (int subw = w; !m_cancel && (subw <= w + SizeW); subw++, i = k += 4)
                {
                for (int subh = h; !m_cancel && (subh <= h + SizeH); subh++, i += LineWidth)
                    {
                    // if is inside...
                    
                    if (IsInside(Width, Height, subw, subh))
                        {
                        // ...we attrib the colors
                        pResBits[i+2] = data[j+2];
                        pResBits[i+1] = data[j+1];
                        pResBits[ i ] = data[ j ];
                        }
                    }
                }
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }
}

/* Function to get a color in a matriz with a determined size                       
 *                                                                                  
 * Bits              => Bits array                                                   
 * Width             => Image width                                                  
 * Height            => Image height                                                
 * X                 => Position horizontal                                          
 * Y                 => Position vertical                                            
 * Radius            => The radius of the matrix to be created                     
 *                                                                                 
 * Theory            => This function takes from a distinct matrix a random color  
 */
QRgb BlurFX::RandomColor(uchar *Bits, int Width, int Height, int X, int Y, int Radius)
{
    int w, h, counter = 0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar I;
    const uchar MAXINTENSITY = 255;

    uchar IntensityCount[MAXINTENSITY + 1];
    uint AverageColorR[MAXINTENSITY + 1];
    uint AverageColorG[MAXINTENSITY + 1];
    uint AverageColorB[MAXINTENSITY + 1];

    memset(IntensityCount, 0, sizeof(IntensityCount) ); 
    memset(AverageColorR,  0, sizeof(AverageColorR) ); 
    memset(AverageColorG,  0, sizeof(AverageColorG) ); 
    memset(AverageColorB,  0, sizeof(AverageColorB) ); 
    
    for (w = X - Radius; !m_cancel && (w <= X + Radius); w++)
        {
        for (h = Y - Radius; !m_cancel && (h <= Y + Radius); h++)
            {
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                {
                int i = h * LineWidth + 4 * w;
                I = GetIntensity (Bits[i], Bits[i+1], Bits[i+2]);
                IntensityCount[I]++;
                counter++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = Bits[i];
                    AverageColorG[I] = Bits[i+1];
                    AverageColorB[I] = Bits[i+2];
                    }
                else
                    {
                    AverageColorR[I] += Bits[i];
                    AverageColorG[I] += Bits[i+1];
                    AverageColorB[I] += Bits[i+2];
                    }
                }
            }
        }

    int RandNumber, count, Index, ErrorCount = 0;
    uchar J;
    
    do
        {
        RandNumber = abs( (int)((rand() + 1) * ((double)counter / (RAND_MAX + 1))) );
        count = 0;
        Index = 0;
        
        do
            {
            count += IntensityCount[Index];
            Index++;
            }
        while (count < RandNumber && !m_cancel);

        J = Index - 1;
        ErrorCount++;
        }
    while ((IntensityCount[J] == 0) && (ErrorCount <= counter)  && !m_cancel);

    int R, G, B;

    if (ErrorCount >= counter)
        {
        R = AverageColorR[J] / counter;
        G = AverageColorG[J] / counter;
        B = AverageColorB[J] / counter;
        }
    else
        {
        R = AverageColorR[J] / IntensityCount[J];
        G = AverageColorG[J] / IntensityCount[J];
        B = AverageColorB[J] / IntensityCount[J];
        }
        
    return ( qRgb(R, G, B) );
}

/* Function to simple convolve a unique pixel with a determined radius                
 *                                                                                    
 * data             => The image data in RGBA mode.  
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Radius           => kernel radius, e.g. rad=1, so array will be 3X3               
 * Kernel           => kernel array to apply.
 *                                                                                    
 * Theory           => I've worked hard here, but I think this is a very smart       
 *                     way to convolve an array, its very hard to explain how I reach    
 *                     this, but the trick here its to store the sum used by the       
 *                     previous pixel, so we sum with the other pixels that wasn't get 
 */
void BlurFX::MakeConvolution (uchar *data, int Width, int Height, int Radius, int Kernel[])
{
    if (Radius <= 0) return;
    
    int progress;
    register int i, j, n, h, w;
    
    int nSumR, nSumG, nSumB, nCount;
    int nKernelWidth = Radius * 2 + 1;
    int nStride = GetStride(Width);
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar* pOutBits = m_destImage.bits(); 
    uchar* pBlur    = new uchar[BitCount];
    
    // We need to copy our bits to blur bits
    
    memcpy (pBlur, data, BitCount);     

    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = Alloc2DArray (nKernelWidth, 256);
    
    for (i = 0; i < nKernelWidth; i++)
        for (j = 0; j < 256; j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to initialize all the loop and iterator variables
    
    nSumR = nSumG = nSumB = nCount = i = j = 0;

    // Now, we enter in the main loop
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // first of all, we need to blur the horizontal lines
                
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
               {
               // if is inside...
               if (IsInside (Width, Height, w + n, h))
                    {
                    // we points to the pixel
                    j = i + n * 4;
                    
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][data[ j ]];
                    nSumG += arrMult[n + Radius][data[j+1]];
                    nSumB += arrMult[n + Radius][data[j+2]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to blur bits the horizontal blur values
            pBlur[ i ] = LimitValues (nSumR / nCount);
            pBlur[i+1] = LimitValues (nSumG / nCount);
            pBlur[i+2] = LimitValues (nSumB / nCount);
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
            
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 50.0) / Height);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; !m_cancel && (w < Width); w++, i = w * 4)
        {
        for (h = 0; !m_cancel && (h < Height); h++, i += LineWidth)
            {
            // first of all, we need to blur the vertical lines
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
                {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                    {
                    // we points to the pixel
                    j = i + n * LineWidth;
                      
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][pBlur[ j ]];
                    nSumG += arrMult[n + Radius][pBlur[j+1]];
                    nSumB += arrMult[n + Radius][pBlur[j+2]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to bits the vertical blur values
            pOutBits[ i ] = LimitValues (nSumR / nCount);
            pOutBits[i+1] = LimitValues (nSumG / nCount);
            pOutBits[i+2] = LimitValues (nSumB / nCount);
                
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        progress = (int) (50.0 + ((double)w * 50.0) / Width);
        
        if (progress%5 == 0)
           postProgress(progress);            
        }

    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
}

}  // NameSpace DigikamBlurFXImagesPlugin
