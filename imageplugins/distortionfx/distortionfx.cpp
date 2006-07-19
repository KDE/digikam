/* ============================================================
 * File  : distortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-07-18
 * Description : Distortion FX threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
 *
 * Original Distortion algorithms copyrighted 2004-2005 by 
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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
#define ANGLE_RATIO        0.017453292519943295769236907685

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

DistortionFX::DistortionFX(Digikam::DImg *orgImage, QObject *parent, int effectType,
                           int level, int iteration, bool antialiasing)
            : Digikam::DImgThreadedFilter(orgImage, parent, "DistortionFX")
{ 
    m_effectType = effectType;
    m_level      = level;
    m_iteration  = iteration;
    m_antiAlias  = antialiasing;

    initFilter();
}

void DistortionFX::filterImage(void)
{
    int w = m_orgImage.width();
    int h = m_orgImage.height();
    int   l     = m_level;
    int   f     = m_iteration;

    switch (m_effectType)
    {
        case FishEye:
            fisheye(&m_orgImage, &m_destImage, (double)(l/5.0), m_antiAlias);
            break;

        case Twirl:
            twirl(&m_orgImage, &m_destImage, l, m_antiAlias);
            break;

        case CilindricalHor:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, false, m_antiAlias);
            break;

        case CilindricalVert:
            cilindrical(&m_orgImage, &m_destImage, (double)l, false, true, m_antiAlias);
            break;

        case CilindricalHV:
            cilindrical(&m_orgImage, &m_destImage, (double)l, true, true, m_antiAlias);
            break;

        case Caricature:
            fisheye(&m_orgImage, &m_destImage, (double)(-l/5.0), m_antiAlias);
            break;

        case MultipleCorners:
            multipleCorners(&m_orgImage, &m_destImage, l, m_antiAlias);
            break;

        case WavesHorizontal:
            waves(&m_orgImage, &m_destImage, l, f, true, true);
            break;

        case WavesVertical:
            waves(&m_orgImage, &m_destImage, l, f, true, false);
            break;

        case BlockWaves1:
            blockWaves(&m_orgImage, &m_destImage, l, f, false);
            break;

        case BlockWaves2:
            blockWaves(&m_orgImage, &m_destImage, l, f, true);
            break;

        case CircularWaves1:
            circularWaves(&m_orgImage, &m_destImage, w/2, h/2, (double)l, (double)f, 0.0, false, m_antiAlias);
            break;

        case CircularWaves2:
            circularWaves(&m_orgImage, &m_destImage, w/2, h/2, (double)l, (double)f, 25.0, true, m_antiAlias);
            break;

        case PolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, true, m_antiAlias);
            break;

        case UnpolarCoordinates:
            polarCoordinates(&m_orgImage, &m_destImage, false, m_antiAlias);
            break;

        case Tile:
            tile(&m_orgImage, &m_destImage, 200-f, 200-f, l);
            break;

        case Neon:
            neon(&m_orgImage, &m_destImage, l, f);
            break;

        case FindEdges:
            findEdges(&m_orgImage, &m_destImage, l, f);
            break;
    }
}

/*
    This code is shared by six methods.
    Write value of pixel w|h in data to pixel nw|nh in pResBits.
    Antialias if requested.
*/
void DistortionFX::setPixelFromOther(int Width, int Height, bool sixteenBit, int bytesDepth,
                                            uchar *data, uchar *pResBits,
                                            int w, int h, double nw, double nh, bool AntiAlias)
{
    Digikam::DColor color;
    int offset, offsetOther;

    offset = getOffset(Width, w, h, bytesDepth);

    if (AntiAlias)
    {
        uchar *ptr = pResBits + offset;
        if (sixteenBit)
        {
            unsigned short *ptr16 = (unsigned short *)ptr;
            Digikam::DImgImageFilters().pixelAntiAliasing16((unsigned short *)data, Width, Height, nw, nh,
                    ptr16+3, ptr16+2, ptr16+1, ptr16);
        }
        else
        {
            Digikam::DImgImageFilters().pixelAntiAliasing(data, Width, Height, nw, nh,
                    ptr+3, ptr+2, ptr+1, ptr);
        }
    }
    else
    {
        // we get the position adjusted
        offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);
        // read color
        color.setColor(data + offsetOther, sixteenBit);
        // write color to destination
        color.setPixel(pResBits + offset);
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
void DistortionFX::fisheye(Digikam::DImg *orgImage, Digikam::DImg *destImage, double Coeff, bool AntiAlias)
{
    if (Coeff == 0.0) return;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw, th, tw;

    int progress;
    int nHalfW = Width / 2, nHalfH = Height / 2;

    Digikam::DColor color;
    int offset;

    double lfXScale = 1.0, lfYScale = 1.0;
    double lfRadius, lfRadMax, lfAngle, lfCoeff, lfCoeffStep = Coeff / 1000.0;

    if (Width > Height)
        lfYScale = (double)Width / (double)Height;
    else if (Height > Width)
        lfXScale = (double)Height / (double)Width;

    lfRadMax = (double)QMAX(Height, Width) / 2.0;
    lfCoeff = lfRadMax / log (fabs (lfCoeffStep) * lfRadMax + 1.0);

    // main loop

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; !m_cancel && (w < Width); w++)
        {
            tw = lfXScale * (double)(w - nHalfW);

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

                setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
            }
            else
            {
                // copy pixel
                offset = getOffset(Width, w, h, bytesDepth);
                color.setColor(data + offset, sixteenBit);
                color.setPixel(pResBits + offset);
            }
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)(h) * 100.0) / Height);

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
void DistortionFX::twirl(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Twirl, bool AntiAlias)
{
    // if twirl value is zero, we do nothing

    if (Twirl == 0)
        return;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double tw, th, nh, nw;

    Digikam::DColor color;
    int offset;

    int progress;
    int nHalfW = Width / 2, nHalfH = Height / 2;

    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfNewAngle, lfAngleStep, lfAngleSum, lfCurrentRadius, lfRadMax;

    if (Width > Height)
        lfYScale = (double)Width / (double)Height;
    else if (Height > Width)
        lfXScale = (double)Height / (double)Width;

    // the angle step is twirl divided by 10000
    lfAngleStep = Twirl / 10000.0;
    // now, we get the minimum radius
    lfRadMax = (double)QMAX(Width, Height) / 2.0;

    // main loop

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; !m_cancel && (w < Width); w++)
        {
            tw = lfXScale * (double)(w - nHalfW);

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

                setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
            }
            else
            {
                // copy pixel
                offset = getOffset(Width, w, h, bytesDepth);
                color.setColor(data + offset, sixteenBit);
                color.setPixel(pResBits + offset);
            }
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

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
void DistortionFX::cilindrical(Digikam::DImg *orgImage, Digikam::DImg *destImage, double Coeff,
                               bool Horizontal, bool Vertical, bool AntiAlias)

{
    if ((Coeff == 0.0) || (! (Horizontal || Vertical)))
        return;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int progress;

    int h, w;
    double nh, nw;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfCoeffX = 1.0, lfCoeffY = 1.0, lfCoeffStep = Coeff / 1000.0;

    if (Horizontal)
        lfCoeffX = (double)nHalfW / log (fabs (lfCoeffStep) * nHalfW + 1.0);
    if (Vertical)
        lfCoeffY = (double)nHalfH / log (fabs (lfCoeffStep) * nHalfH + 1.0);

    // initial copy
    memcpy (pResBits, data, orgImage->numBytes());

    // main loop

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        for (w = 0; !m_cancel && (w < Width); w++)
        {
            // we find the distance from the center
            nh = fabs ((double)(h - nHalfH));
            nw = fabs ((double)(w - nHalfW));

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

            nw = (double)nHalfW + ((w >= nHalfW) ? nw : -nw);
            nh = (double)nHalfH + ((h >= nHalfH) ? nh : -nh);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

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
void DistortionFX::multipleCorners(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Factor, bool AntiAlias)
{
    if (Factor == 0) return;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw;
    int progress;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfAngle, lfNewRadius, lfCurrentRadius, lfRadMax;

    lfRadMax = sqrt (Height * Height + Width * Width) / 2.0;

    // main loop

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        for (w = 0; !m_cancel && (w < Width); w++)
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

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

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
void DistortionFX::polarCoordinates(Digikam::DImg *orgImage, Digikam::DImg *destImage, bool Type, bool AntiAlias)
{
    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw, th, tw;
    int progress;

    int nHalfW = Width / 2, nHalfH = Height / 2;
    double lfXScale = 1.0, lfYScale = 1.0;
    double lfAngle, lfRadius, lfRadMax;

    if (Width > Height)
        lfYScale = (double)Width / (double)Height;
    else if (Height > Width)
        lfXScale = (double)Height / (double)Width;

    lfRadMax = (double)QMAX(Height, Width) / 2.0;

    // main loop

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        th = lfYScale * (double)(h - nHalfH);

        for (w = 0; !m_cancel && (w < Width); w++)
        {
            tw = lfXScale * (double)(w - nHalfW);

            if (Type)
            {
                // now, we get the distance
                lfRadius = sqrt (th * th + tw * tw);
                // we find the angle from the center
                lfAngle = atan2 (tw, th);

                // now we find the exact position's x and y
                nh = lfRadius * (double) Height / lfRadMax;
                nw =  lfAngle * (double)  Width / (2 * M_PI);

                nw = (double)nHalfW + nw;
            }
            else
            {
                lfRadius = (double)(h) * lfRadMax / (double)Height;
                lfAngle  = (double)(w) * (2 * M_PI) / (double) Width;

                nw = (double)nHalfW - (lfRadius / lfXScale) * sin (lfAngle);
                nh = (double)nHalfH - (lfRadius / lfYScale) * cos (lfAngle);
            }

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

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
void DistortionFX::circularWaves(Digikam::DImg *orgImage, Digikam::DImg *destImage, int X, int Y, double Amplitude, 
                                 double Frequency, double Phase, bool WavesType, bool AntiAlias)
{
    if (Amplitude < 0.0) Amplitude = 0.0;
    if (Frequency < 0.0) Frequency = 0.0;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int h, w;
    double nh, nw;
    int progress;

    double lfRadius, lfRadMax, lfNewAmp = Amplitude;
    double lfFreqAngle = Frequency * ANGLE_RATIO;

    Phase *= ANGLE_RATIO;

    lfRadMax = sqrt (Height * Height + Width * Width);

    for (h = 0; !m_cancel && (h < Height); h++)
    {
        for (w = 0; !m_cancel && (w < Width); w++)
        {
            nw = X - w;
            nh = Y - h;

            lfRadius = sqrt (nw * nw + nh * nh);

            if (WavesType)
                lfNewAmp = Amplitude * lfRadius / lfRadMax;

            nw = (double)w + lfNewAmp * sin(lfFreqAngle * lfRadius + Phase);
            nh = (double)h + lfNewAmp * cos(lfFreqAngle * lfRadius + Phase);

            setPixelFromOther(Width, Height, sixteenBit, bytesDepth, data, pResBits, w, h, nw, nh, AntiAlias);
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

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
void DistortionFX::waves(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                         int Amplitude, int Frequency,
                         bool FillSides, bool Direction)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;

    int Width       = orgImage->width();
    int Height      = orgImage->height();

    int progress;
    int h, w;

    if (Direction)        // Horizontal
    {
        int tx;

        for (h = 0; !m_cancel && (h < Height); h++)
        {
            tx = lround(Amplitude * sin ((Frequency * 2) * h * (M_PI / 180)));
            destImage->bitBltImage(orgImage, 0, h,  Width, 1,  tx, h);

            if (FillSides)
            {
                destImage->bitBltImage(orgImage, Width - tx, h,  tx, 1,  0, h);
                destImage->bitBltImage(orgImage, 0, h,  Width - (Width - 2 * Amplitude + tx), 1,  Width + tx, h);
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
            ty = lround(Amplitude * sin ((Frequency * 2) * w * (M_PI / 180)));
            destImage->bitBltImage(orgImage, w, 0, 1, Height, w, ty);

            if (FillSides)
            {
                destImage->bitBltImage(orgImage, w, Height - ty,  1, ty,  w, 0);
                destImage->bitBltImage(orgImage, w, 0,  1, Height - (Height - 2 * Amplitude + ty),  w, Height + ty);
            }

            // Update the progress bar in dialog.
            progress = (int) (((double)w * 100.0) / Width);

            if (progress%5 == 0)
                postProgress(progress);
        }
    }
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
void DistortionFX::blockWaves(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                              int Amplitude, int Frequency, bool Mode)
{
    if (Amplitude < 0) Amplitude = 0;
    if (Frequency < 0) Frequency = 0;

    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    int nw, nh, progress;
    double Radius;

    Digikam::DColor color;
    int offset, offsetOther;

    int nHalfW = Width / 2, nHalfH = Height / 2;

    for (int w = 0; !m_cancel && (w < Width); w++)
    {
        for (int h = 0; !m_cancel && (h < Height); h++)
        {
            nw = nHalfW - w;
            nh = nHalfH - h;

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

            offset = getOffset(Width, w, h, bytesDepth);
            offsetOther = getOffsetAdjusted(Width, Height, (int)nw, (int)nh, bytesDepth);

            // read color
            color.setColor(data + offsetOther, sixteenBit);
            // write color to destination
            color.setPixel(pResBits + offset);
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
void DistortionFX::tile(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                        int WSize, int HSize, int Random)
{
    if (WSize < 1)  WSize = 1;
    if (HSize < 1)  HSize = 1;
    if (Random < 1) Random = 1;

    int Width       = orgImage->width();
    int Height      = orgImage->height();

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    uint seed = dt.secsTo(Y2000);

    int tx, ty, h, w, progress;

    for (h = 0; !m_cancel && (h < Height); h += HSize)
    {
        for (w = 0; !m_cancel && (w < Width); w += WSize)
        {
            tx = (int)(rand_r(&seed) % Random) - (Random / 2);
            ty = (int)(rand_r(&seed) % Random) - (Random / 2);
            destImage->bitBltImage(orgImage, w, h,   WSize, HSize,   w + tx, h + ty);
        }

        // Update the progress bar in dialog.
        progress = (int)(((double)h * 100.0) / Height);

        if (progress%5 == 0)
            postProgress(progress);
    }
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
void DistortionFX::neon(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Intensity, int BW)
{
    neonFindEdges(orgImage, destImage, true, Intensity, BW);
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
void DistortionFX::findEdges(Digikam::DImg *orgImage, Digikam::DImg *destImage, int Intensity, int BW)
{
    neonFindEdges(orgImage, destImage, false, Intensity, BW);
}

// Implementation of neon and FindEdges. They share 99% of their code.
void DistortionFX::neonFindEdges(Digikam::DImg *orgImage, Digikam::DImg *destImage, bool neon, int Intensity, int BW)
{
    int Width       = orgImage->width();
    int Height      = orgImage->height();
    uchar* data     = orgImage->bits();
    bool sixteenBit = orgImage->sixteenBit();
    int bytesDepth  = orgImage->bytesDepth();
    uchar* pResBits = destImage->bits();

    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;

    uchar *ptr, *ptr1, *ptr2;

    // these must be uint, we need full 2^32 range for 16 bit
    uint color_1, color_2, colorPoint, colorOther1, colorOther2, progress;

    // initial copy
    memcpy (pResBits, data, orgImage->numBytes());

    double intensityFactor = sqrt( 1 << Intensity );

    for (int h = 0; h < Height; h++)
    {
        for (int w = 0; w < Width; w++)
        {
            ptr  = pResBits + getOffset(Width, w, h, bytesDepth);
            ptr1 = pResBits + getOffset(Width, w + Lim_Max (w, BW, Width), h, bytesDepth);
            ptr2 = pResBits + getOffset(Width, w, h + Lim_Max (h, BW, Height), bytesDepth);

            if (sixteenBit)
            {
                for (int k = 0; k <= 2; k++)
                {
                    colorPoint  = ((unsigned short *)ptr)[k];
                    colorOther1 = ((unsigned short *)ptr1)[k];
                    colorOther2 = ((unsigned short *)ptr2)[k];
                    color_1 = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2 = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    // old algorithm was
                    // sqrt ((color_1 + color_2) << Intensity)
                    // As (a << I) = a * (1 << I) = a * (2^I), and we can split the square root

                    if (neon)
                        ((unsigned short *)ptr)[k] = CLAMP065535 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                    else
                        ((unsigned short *)ptr)[k] = 65535 - CLAMP065535 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                }
            }
            else
            {
                for (int k = 0; k <= 2; k++)
                {
                    colorPoint  = ptr[k];
                    colorOther1 = ptr1[k];
                    colorOther2 = ptr2[k];
                    color_1 = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2 = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    if (neon)
                        ptr[k] = CLAMP0255 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                    else
                        ptr[k] = 255 - CLAMP0255 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                }
            }
        }

        // Update the progress bar in dialog.
        progress = (int) (((double)h * 100.0) / Height);

        if (progress%5 == 0)
            postProgress(progress);
    }
}

// UNUSED
/* Function to return the maximum radius with a determined angle                    
 *                                                                                  
 * Height           => Height of the image                                         
 * Width            => Width of the image                                           
 * Angle            => Angle to analize the maximum radius                          
 *                                                                                  
 * Theory           => This function calcule the maximum radius to that angle      
 *                     so, we can build an oval circunference                        
 */                                                                                   
 /*
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
 */

}  // NameSpace DigikamDistortionFXImagesPlugin
