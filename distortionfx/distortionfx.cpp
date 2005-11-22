/* ============================================================
 * File  : distortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Distortion FX threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
#define ANGLE_RATIO        0.017453292519943295769236907685   // Represents 1º 

// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.
#include <qdatetime.h> 

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "distortionfx.h"

namespace DigikamDistortionFXImagesPlugin
{

DistortionFX::DistortionFX(QImage *orgImage, QObject *parent, int effectType, 
                           int level, int iteration, bool antialiasing)
            : Digikam::ThreadedFilter(orgImage, parent, "DistortionFX")
{ 
    m_effectType = effectType;
    m_level      = level;
    m_iteration  = iteration;
    m_antiAlias  = antialiasing;
        
    initFilter();
}

void DistortionFX::filterImage(void)
{
    int   w     = m_orgImage.width();
    int   h     = m_orgImage.height();
    uchar* data = (uchar*)m_orgImage.bits();
    int   l     = m_level;
    int   f     = m_iteration;
    
    switch (m_effectType)
       {
       case FishEye: 
          fisheye(data, w, h, (double)(l/5.0), m_antiAlias);
          break;
       
       case Twirl: 
          twirl(data, w, h, l, m_antiAlias);
          break;

       case CilindricalHor: 
          cilindrical(data, w, h, (double)l, true, false, m_antiAlias);
          break;

       case CilindricalVert: 
          cilindrical(data, w, h, (double)l, false, true, m_antiAlias);
          break;
                    
       case CilindricalHV: 
          cilindrical(data, w, h, (double)l, true, true, m_antiAlias);
          break;
       
       case Caricature: 
          fisheye(data, w, h, (double)(-l/5.0), m_antiAlias);
          break;
          
       case MultipleCorners:          
          multipleCorners(data, w, h, l, m_antiAlias);
          break;
       
       case WavesHorizontal: 
          waves(data, w, h, l, f, true, false);
          break;
       
       case WavesVertical: 
          waves(data, w, h, l, f, true, true);
          break;
       
       case BlockWaves1: 
          blockWaves(data, w, h, l, f, false);
          break;
       
       case BlockWaves2: 
          blockWaves(data, w, h, l, f, true);
          break;
       
       case CircularWaves1:
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 0.0, false, m_antiAlias);
          break;
       
       case CircularWaves2: 
          circularWaves(data, w, h, w/2, h/2, (double)l, (double)f, 25.0, true, m_antiAlias);
          break;
       
       case PolarCoordinates: 
          polarCoordinates(data, w, h, true, m_antiAlias);
          break;

       case UnpolarCoordinates: 
          polarCoordinates(data, w, h, false, m_antiAlias);
          break;
                    
       case Tile: 
          tile(data, w, h, 200-f, 200-f, l);
          break;

       case Neon: 
          neon(data, w, h, l, f);
          break;
          
       case FindEdges:
          findEdges(data, w, h, l, f);
          break;
       }
}

/* Function to apply the fisheye effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.    
 * Coeff            => Distortion effect coeff. Positive value render 'Fish Eyes' effect, 
 *                     and negative values render 'Caricature' effect.
 * Antialias        => Smart bluring result.                       
 *                                                                                  
 * Theory           => This is a great effect if you take employee photos
 *                     Its pure trigonometry. I think if you study hard the code you
 *                     understand very well.
 */
void DistortionFX::fisheye(uchar *data, int Width, int Height, double Coeff, bool AntiAlias)
{
    if (Coeff == 0.0) return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar*    pBits = data;
    uchar* pResBits = (uchar*)m_destImage.bits();

    register int h, w, th, tw, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height, progress;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfRadius, lfRadMax, lfAngle, lfCoeff, lfCoeffStep = Coeff / 1000.0;

    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    lfRadMax = (double)QMAX(nHeight, nWidth) / 2.0;
    lfCoeff = lfRadMax / log (fabs (lfCoeffStep) * lfRadMax + 1.0);

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = (int)(lfYScale * (double)h);
        
        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            tw = (int)(lfXScale * (double)w);

            // we find the distance from the center
            lfRadius = sqrt (th * th + tw * tw);

            if (lfRadius < lfRadMax)
                {
                lfAngle = atan2 (th, tw);

                if (Coeff > 0.0)
                    lfRadius = (exp (lfRadius / lfCoeff) - 1.0) / lfCoeffStep;
                else
                    lfRadius = lfCoeff * log (1.0 + (-1.0 * lfCoeffStep) * lfRadius);

                nw = (double)nHalfW + (lfRadius / lfXScale) * cos (lfAngle);
                nh = (double)nHalfH + (lfRadius / lfYScale) * sin (lfAngle);

                if (AntiAlias)
                    {
                    Digikam::ImageFilters::pixelAntiAliasing(data, Width, Height, nw, nh, 
                             &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                    i += 4;
                    }
                else
                    {
                    // we get the position adjusted
                    j = setPositionAdjusted (Width, Height, (int)nw, (int)nh);

                    // now we set the pixel
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    }
                }
            else
                {
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                }
            }

        // Update the progress bar in dialog.
        progress = (int) (((double)(h + nHalfH) * 100.0) / (nHeight - nHalfH));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the twirl effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Twirl            => Distance value.
 * Antialias        => Smart bluring result.    
 *                                                                                  
 * Theory           => Take spiral studies, you will understand better, I'm studying
 *                     hard on this effect, because it's not too fast.
 */
void DistortionFX::twirl(uchar *data, int Width, int Height, int Twirl, bool AntiAlias)
{
    // if twirl value is zero, we do nothing
    
    if (Twirl == 0)
        return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar*    pBits = data;
    uchar* pResBits = (uchar*)m_destImage.bits();
    
    register int h, w, i = 0, j;
    register double tw, th, nh, nw;
    int nWidth = Width, nHeight = Height, progress;
    int nStride = GetStride (Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfNewAngle, lfAngleStep, lfAngleSum, lfCurrentRadius, lfRadMax;
    
    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    // the angle step is twirl divided by 10000
    lfAngleStep = Twirl / 10000.0;
    // now, we get the minimum radius
    lfRadMax = (double)QMAX(nWidth, nHeight) / 2.0;

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = (lfYScale * (double)h);

        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            tw = (lfXScale * (double)w);

            // now, we get the distance
            lfCurrentRadius = sqrt (th * th + tw * tw);
            
            // if distance is less than maximum radius...
            if (lfCurrentRadius < lfRadMax)
                {
                // we find the angle from the center
                lfAngle = atan2 (th, tw);
                // we get the accumuled angle
                lfAngleSum = lfAngleStep * (-1.0 * (lfCurrentRadius - lfRadMax));
                // ok, we sum angle with accumuled to find a new angle
                lfNewAngle = lfAngle + lfAngleSum;

                // now we find the exact position's x and y
                nw = (double)nHalfW + cos (lfNewAngle) * (lfCurrentRadius / lfXScale);
                nh = (double)nHalfH + sin (lfNewAngle) * (lfCurrentRadius / lfYScale);

                if (AntiAlias)
                    {
                    Digikam::ImageFilters::pixelAntiAliasing(data, Width, Height, nw, nh, 
                             &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                    i += 4;
                    }
                else
                    {
                    // we get the position adjusted
                    j = setPositionAdjusted(Width, Height, (int)nw, (int)nh);

                    // now we set the pixel
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    pResBits[i++] = pBits[j++];
                    }
                }
            else
                {
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                pResBits[i++] = pBits[i];
                }
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / (nHeight - nHalfH));
        
        if (progress%5 == 0)
           postProgress(progress);
    }
}

/* Function to apply the Cilindrical effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Coeff            => Cilindrical value.
 * Horizontal       => Apply horizontally.
 * Vertical         => Apply vertically.
 * Antialias        => Smart bluring result. 
 *                                                                                  
 * Theory           => This is a great effect, similar to Spherize (Photoshop).    
 *                     If you understand FishEye, you will understand Cilindrical    
 *                     FishEye apply a logarithm function using a sphere radius,     
 *                     Spherize use the same function but in a rectangular        
 *                     enviroment.
 */
void DistortionFX::cilindrical(uchar *data, int Width, int Height, double Coeff, 
                               bool Horizontal, bool Vertical, bool AntiAlias)

{
    if ((Coeff == 0.0) || (! (Horizontal || Vertical)))
        return;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int progress;
    int BitCount = LineWidth * Height;
    uchar*    pBits = data;
    uchar* pResBits = (uchar*)m_destImage.bits();
    memcpy (pResBits, data, BitCount);     

    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height;
    int nStride = GetStride (Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfCoeffX, lfCoeffY, lfCoeffStep = Coeff / 1000.0;

    if (Horizontal)
        lfCoeffX = (double)nHalfW / log (fabs (lfCoeffStep) * nHalfW + 1.0);
    if (Vertical)
        lfCoeffY = (double)nHalfH / log (fabs (lfCoeffStep) * nHalfH + 1.0);

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++)
            {
            // we find the distance from the center
            nh = fabs ((double)h);
            nw = fabs ((double)w);

            if (Horizontal)
                {
                if (Coeff > 0.0)
                    nw = (exp (nw / lfCoeffX) - 1.0) / lfCoeffStep;
                else
                    nw = lfCoeffX * log (1.0 + (-1.0 * lfCoeffStep) * nw);
                }

            if (Vertical)
                {
                if (Coeff > 0.0)
                    nh = (exp (nh / lfCoeffY) - 1.0) / lfCoeffStep;
                else
                    nh = lfCoeffY * log (1.0 + (-1.0 * lfCoeffStep) * nh);
                }

            nw = (double)nHalfW + ((w >= 0) ? nw : -nw);
            nh = (double)nHalfH + ((h >= 0) ? nh : -nh);

            if (AntiAlias)
                {
                Digikam::ImageFilters::pixelAntiAliasing(data, Width, Height, nw, nh, 
                         &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                i += 4;
                }
            else
                {
                // we get the position adjusted
                j = setPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                }
            }
        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / (nHeight - nHalfH));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the Multiple Corners effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.       
 * Factor           => nb corners.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => This is an amazing function, you've never seen this before. 
 *                     I was testing some trigonometric functions, and I saw that if  
 *                     I multiply the angle by 2, the result is an image like this   
 *                     If we multiply by 3, we can create the SixCorners effect. 
 */
void DistortionFX::multipleCorners(uchar *data, int Width, int Height, int Factor, bool AntiAlias)
{
    if (Factor == 0) return;
    
    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height, progress;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfAngle, lfNewRadius, lfCurrentRadius, lfRadMax;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar*    pBits = (uchar*)data;
    uchar* pResBits = (uchar*)m_destImage.bits();

    lfRadMax = sqrt (nHeight * nHeight + nWidth * nWidth) / 2.0;

    // main loop
    
    for (h = 0; !m_cancel && (h < nHeight); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < nWidth); w++)
            {
            // we find the distance from the center
            nh = nHalfH - h;
            nw = nHalfW - w;

            // now, we get the distance
            lfCurrentRadius = sqrt (nh * nh + nw * nw);
            // we find the angle from the center
            lfAngle = atan2 (nh, nw) * (double)Factor;
            
            // ok, we sum angle with accumuled to find a new angle
            lfNewRadius = lfCurrentRadius * lfCurrentRadius / lfRadMax;

            // now we find the exact position's x and y
            nw = (double)nHalfW - (cos (lfAngle) * lfNewRadius);
            nh = (double)nHalfH - (sin (lfAngle) * lfNewRadius);

            if (AntiAlias)
               {
                Digikam::ImageFilters::pixelAntiAliasing(data, Width, Height, nw, nh, 
                         &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                i += 4;
                }
            else
                {
                // we get the position adjusted
                j = setPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                }
            }            
            
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / nHeight);
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the Polar Coordinates effect backported from ImageProcessing version 2
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.
 * Type             => if true Polar Coordinate to Polar else inverse.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => Similar to PolarCoordinates from Photoshop. We apply the polar   
 *                     transformation in a proportional (Height and Width) radius.
 */
void DistortionFX::polarCoordinates(uchar *data, int Width, int Height, bool Type, bool AntiAlias)
{
    register int h, w, i = 0, j;
    register double nh, nw, th, tw;
    int nWidth = Width, nHeight = Height, progress;
    int nStride = GetStride(Width);
    int nHalfW = nWidth / 2, nHalfH = nHeight / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfRadius, lfRadMax;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    uchar*    pBits = data;
    uchar* pResBits = (uchar*)m_destImage.bits();
        
    if (nWidth > nHeight)
        lfYScale = (double)nWidth / (double)nHeight;
    else if (nHeight > nWidth)
        lfXScale = (double)nHeight / (double)nWidth;

    lfRadMax = (double)QMAX(nHeight, nWidth) / 2.0;

    // main loop
    
    for (h = -nHalfH; !m_cancel && (h < nHeight - nHalfH); h++, i += nStride)
        {
        th = lfYScale * (double)h;

        for (w = -nHalfW; !m_cancel && (w < nWidth - nHalfW); w++, i += 4)
            {
            tw = lfXScale * (double)w;

            if (Type)
                {
                // now, we get the distance
                lfRadius = sqrt (th * th + tw * tw);
                // we find the angle from the center
                lfAngle = atan2 (tw, th);
            
                // now we find the exact position's x and y
                nh = lfRadius * (double)nHeight / lfRadMax;
                nw =  lfAngle * (double) nWidth / (2 * M_PI);

                nw = (double)nHalfW + nw;
                }
            else
                {
                lfRadius = (double)(h + nHalfH) * lfRadMax / (double)nHeight;
                lfAngle  = (double)(w + nHalfW) * (2 * M_PI) / (double) nWidth;

                nw = (double)nHalfW - (lfRadius / lfXScale) * sin (lfAngle);
                nh = (double)nHalfH - (lfRadius / lfYScale) * cos (lfAngle);
                }

            if (AntiAlias)
                {
                Digikam::ImageFilters::pixelAntiAliasing(data, Width, Height, nw, nh, 
                         &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                } 
            else
                {
                // we get the position adjusted
                j = setPositionAdjusted(Width, Height, (int)nw, (int)nh);

                // now we set the pixel
                pResBits[ i ] = pBits[j++];
                pResBits[i+1] = pBits[j++];
                pResBits[i+2] = pBits[j++];
                }
            }
        
        // Update the progress bar in dialog.
        progress = (int) ((double)h * 100.0) / (nHeight - nHalfH);
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the circular waves effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.     
 * X, Y             => Position of circle center on the image.                       
 * Amplitude        => Sinoidal maximum height                                        
 * Frequency        => Frequency value.
 * Phase            => Phase value.
 * WavesType        => If true  the amplitude is proportional to radius.
 * Antialias        => Smart bluring result.                      
 *                                                                                  
 * Theory           => Similar to Waves effect, but here I apply a senoidal function
 *                     with the angle point.                                                      
 */
void DistortionFX::circularWaves(uchar *data, int Width, int Height, int X, int Y, double Amplitude, 
                                 double Frequency, double Phase, bool WavesType, bool AntiAlias)
{
    if (Amplitude < 0.0) Amplitude = 0.0;
    if (Frequency < 0.0) Frequency = 0.0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

    register int h, w, i = 0, j;
    register double nh, nw;
    int nWidth = Width, nHeight = Height, progress;
    int nStride = GetStride(Width);
    double lfRadius, lfRadMax, lfNewAmp = Amplitude;
    double lfFreqAngle = Frequency * ANGLE_RATIO;
    
    uchar*    pBits = data;
    uchar* pResBits = (uchar*)m_destImage.bits();

    Phase *= ANGLE_RATIO;

    lfRadMax = sqrt (nHeight * nHeight + nWidth * nWidth);

    for (h = 0; !m_cancel && (h < nHeight); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < nWidth); w++)
            {
            nw = X - w;
            nh = Y - h;

            lfRadius = sqrt (nw * nw + nh * nh);

            if (WavesType)
                lfNewAmp = Amplitude * lfRadius / lfRadMax;

            nw = (double)w + lfNewAmp * sin(lfFreqAngle * lfRadius + Phase);
            nh = (double)h + lfNewAmp * cos(lfFreqAngle * lfRadius + Phase);

            if (AntiAlias)
                {
                Digikam::ImageFilters::pixelAntiAliasing( data, Width, Height, nw, nh, 
                         &pResBits[i+3], &pResBits[i+2], &pResBits[i+1], &pResBits[i]);
                i += 4;
                }
            else
                {
                j = setPositionAdjusted(Width, Height, (int)nw, (int)nh);

                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
                pResBits[i++] = pBits[j++];
               }
            }
                        
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / nHeight);
        
        if (progress%5 == 0)
           postProgress(progress);
        }
}

/* Function to apply the waves effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Amplitude        => Sinoidal maximum height.                                        
 * Frequency        => Frequency value.                                                
 * FillSides        => Like a boolean variable.                                        
 * Direction        => Vertical or horizontal flag.                                    
 *                                                                                    
 * Theory           => This is an amazing effect, very funny, and very simple to    
 *                     understand. You just need understand how sin and cos works.   
 */
void DistortionFX::waves(uchar *data, int Width, int Height,
                         int Amplitude, int Frequency, 
                         bool FillSides, bool Direction)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;

    QImage PicSrcDC((uchar*)data, Width, Height, 32, 0, 0, QImage::IgnoreEndian);
    QImage PicDestDC(Width, Height, 32);

    int progress;
    register int h, w;
    
    if (Direction)        // Horizontal
        {
        int tx;
        
        for (h = 0; !m_cancel && (h < Height); h++)
            {
            tx = (int)(Amplitude * sin ((Frequency * 2) * h * (M_PI / 180)));
            bitBlt(&PicDestDC, tx, h, &PicSrcDC, 0, h, Width, 1);
            
            if (FillSides)
                {
                bitBlt(&PicDestDC, 0, h, &PicSrcDC, Width - tx, h, tx, 1);
                bitBlt(&PicDestDC, Width + tx, h, &PicSrcDC, 0, h, Width - (Width - 2 * Amplitude + tx), 1);
                }
            
            // Update the progress bar in dialog.
            progress = (int) (((double)h * 100.0) / Height);
            
            if (progress%5 == 0)
                postProgress(progress);
            }
        }
    else
        {
        int ty;
        
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            ty = (int)(Amplitude * sin ((Frequency * 2) * w * (M_PI / 180)));
            bitBlt(&PicDestDC, w, ty, &PicSrcDC, w, 0, 1, Height);
            
            if (FillSides)
                {
                bitBlt(&PicDestDC, w, 0, &PicSrcDC, w, Height - ty, 1, ty);
                bitBlt(&PicDestDC, w, Height + ty, &PicSrcDC, w, 0, 1, Height - (Height - 2 * Amplitude + ty));
                }
            
            // Update the progress bar in dialog.
            progress = (int) (((double)w * 100.0) / Width);
            
            if (progress%5 == 0)
                postProgress(progress);            
            }
        }

    m_destImage = PicDestDC;        
}

/* Function to apply the block waves effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Amplitude        => Sinoidal maximum height                                        
 * Frequency        => Frequency value                                                
 * Mode             => The mode to be applied.                                       
 *                                                                                  
 * Theory           => This is an amazing effect, very funny when amplitude and     
 *                     frequency are small values.                                  
 */
void DistortionFX::blockWaves(uchar *data, int Width, int Height,
                              int Amplitude, int Frequency, bool Mode)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar*    Bits = data;
    uchar* NewBits = (uchar*)m_destImage.bits();

    int i = 0, j = 0, nw, nh, progress;
    double Radius;
        
    for (int w = 0; !m_cancel && (w < Width); w++)
        {
        for (int h = 0; !m_cancel && (h < Height); h++)
            {
            i = h * LineWidth + 4 * w;
            nw = Width / 2 - w;
            nh = Height / 2 - h;
            Radius = sqrt (nw * nw + nh * nh);
                
            if (Mode)
                {
                nw = (int)(w + Amplitude * sin (Frequency * nw * (M_PI / 180)));
                nh = (int)(h + Amplitude * cos (Frequency * nh * (M_PI / 180)));
                }
            else
                {
                nw = (int)(w + Amplitude * sin (Frequency * w * (M_PI / 180)));
                nh = (int)(h + Amplitude * cos (Frequency * h * (M_PI / 180)));
                }

            nw = (nw < 0) ? 0 : ((nw >= Width) ? Width - 1 : nw);
            nh = (nh < 0) ? 0 : ((nh >= Height) ? Height - 1 : nh);
            j = nh * LineWidth + 4 * nw;
            NewBits[i+2] = Bits[j+2];
            NewBits[i+1] = Bits[j+1];
            NewBits[ i ] = Bits[ j ];
            }
            
        // Update the progress bar in dialog.
        progress = (int) (((double)w * 100.0) / Width);
        
        if (progress%5 == 0)
            postProgress(progress);            
        }
}

/* Function to apply the tile effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * WSize            => Tile Width                                                        
 * HSize            => Tile Height                                                      
 * Random           => Maximum random value                                        
 *                                                                                    
 * Theory           => Similar to Tile effect from Photoshop and very easy to        
 *                     understand. We get a rectangular area using WSize and HSize and    
 *                     replace in a position with a random distance from the original    
 *                     position.                                                    
 */
void DistortionFX::tile(uchar *data, int Width, int Height, 
                        int WSize, int HSize, int Random)
{
    if (WSize < 1)  WSize = 1;
    if (HSize < 1)  HSize = 1;
    if (Random < 1) Random = 1;
        
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    srand ((uint) dt.secsTo(Y2000));
    
    QImage PicSrcDC(data, Width, Height, 32, 0, 0, QImage::IgnoreEndian);
    QImage PicDestDC(Width, Height, 32);
    
    int tx, ty, h, w, progress;
    
    for (h = 0; !m_cancel && (h < Height); h += HSize)
        {
        for (w = 0; !m_cancel && (w < Width); w += WSize)
            {
            tx = (int)(rand() % Random) - (Random / 2);
            ty = (int)(rand() % Random) - (Random / 2);
            bitBlt (&PicDestDC, w + tx, h + ty, &PicSrcDC, w, h, WSize, HSize);
            }
            
        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
            postProgress(progress);        
        }

    m_destImage = PicDestDC;      
}

/* Function to apply the Neon effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * Intensity        => Intensity value                                                
 * BW               => Border Width                            
 *                                                                                  
 * Theory           => Wow, this is a great effect, you've never seen a Neon effect   
 *                     like this on PSC. Is very similar to Growing Edges (photoshop)  
 *                     Some pictures will be very interesting   
 */
void DistortionFX::neon(uchar *data, int Width, int Height, int Intensity, int BW)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;
    
    uchar* Bits = data;
    int i = 0, j = 0, color_1, color_2, progress;
        
    for (int h = 0; h < Height; h++)
        {
        for (int w = 0; w < Width; w++)
            {
            for (int k = 0; k <= 2; k++)
                {
                i = h * LineWidth + 4 * w;
                j = h * LineWidth + 4 * (w + Lim_Max (w, BW, Width));
                color_1 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                j = (h + Lim_Max (h, BW, Height)) * LineWidth + 4 * w;
                color_2 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                Bits[i+k] = CLAMP0255 ((int)(sqrt ((color_1 + color_2) << Intensity)));
                }
            }
            
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
            postProgress(progress);                
        }
        
    m_destImage = m_orgImage;    
}

/* Function to apply the Find Edges effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * Intensity        => Intensity value                                                
 * BW               => Border Width                            
 *                                                                                  
 * Theory           => Wow, another Photoshop filter (FindEdges). Do you understand  
 *                     Neon effect ? This is the same engine, but is inversed with   
 *                     255 - color.  
 */
void DistortionFX::findEdges(uchar *data, int Width, int Height, int Intensity, int BW)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;

    uchar* Bits = data;
    int i = 0, j = 0, color_1, color_2, progress;
    
    for (int h = 0; h < Height; h++)
        {
        for (int w = 0; w < Width; w++)
            {
            for (int k = 0; k <= 2; k++)
                {
                i = h * LineWidth + 4 * w;
                j = h * LineWidth + 4 * (w + Lim_Max (w, BW, Width));
                color_1 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                j = (h + Lim_Max (h, BW, Height)) * LineWidth + 4 * w;
                color_2 = (int)((Bits[i+k] - Bits[j+k]) * (Bits[i+k] - Bits[j+k]));
                Bits[i+k] = 255 - CLAMP0255 ((int)(sqrt ((color_1 + color_2) << Intensity)));
                }
            }
                
        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);
        
        if (progress%5 == 0)
            postProgress(progress);                
        }

    m_destImage = m_orgImage;            
}

/* Function to return the maximum radius with a determined angle                    
 *                                                                                  
 * Height           => Height of the image                                         
 * Width            => Width of the image                                           
 * Angle            => Angle to analize the maximum radius                          
 *                                                                                  
 * Theory           => This function calcule the maximum radius to that angle      
 *                     so, we can build an oval circunference                        
 */                                                                                   
double DistortionFX::maximumRadius(int Height, int Width, double Angle)
{
    double MaxRad, MinRad;
    double Radius, DegAngle = fabs (Angle * 57.295);    // Rads -> Degrees

    MinRad = QMIN (Height, Width) / 2.0;                // Gets the minor radius
    MaxRad = QMAX (Height, Width) / 2.0;                // Gets the major radius

    // Find the quadrant between -PI/2 and PI/2
    if (DegAngle > 90.0)
        Radius = proportionalValue (MinRad, MaxRad, (DegAngle * (255.0 / 90.0)));
    else
        Radius = proportionalValue (MaxRad, MinRad, ((DegAngle - 90.0) * (255.0 / 90.0)));
    return (Radius);
}

}  // NameSpace DigikamDistortionFXImagesPlugin
