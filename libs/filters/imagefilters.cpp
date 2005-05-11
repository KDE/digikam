/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-24-01
 * Description : image filters. 
 * 
 * Copyright 2004-2005 by Gilles Caulier
 * Normalize an Equalize algorithms fixed and adapted to work with Raw 
 * data image (ARGB).
 * 
 * Original Equalise and StretchContrast Algorithms copyright 2002
 * by Daniel M. Duley <mosfet@kde.org> from KImageEffect API.
 *
 * Original Normalize Image algorithm copyrighted 1997 by 
 * Adam D. Moss <adam@foxbox.org> from Gimp 2.0 implementation.
 *
 * Original Gaussian Blur algorithm copyrighted 2005 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
 *
 * Original channel mixer algorithm copyrighted 2002 by 
 * Martin Guldahl <mguldahl at xmission dot com> from Gimp 2.2 
 * 
 * Original sharpening filter from from gimp 2.2
 * copyright 1997-1998 Michael Sweet (mike@easysw.com)
 *
 * Original HSL algorithm from from gimp 2.2
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#define CLAMP0255(a)  QMIN(QMAX(a,0), 255) 
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))
 
// C++ includes.

#include <cstring>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>
#include <kapplication.h>

// Digikam includes.
 
#include <imagehistogram.h>
#include <imagelevels.h>
#include <imageiface.h>

// Local includes.

#include "imagefilters.h"

namespace Digikam
{

/////////////////////////////////////////////////////////////////////////////////
// Performs an histogram equalisation of the image.

void ImageFilters::equalizeImage(uint *data, int w, int h)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::equalizeImage: no image data available!") << endl;
       return;
       }
       
    struct double_packet  high, low, intensity;
    struct double_packet *map;
    struct short_packet  *equalize_map;
    int                   x, y;
    unsigned int         *q;
    register long         i;               
    unsigned char         r, g, b, a;
    
    // Create an histogram of the current image.     
    Digikam::ImageHistogram *histogram = new Digikam::ImageHistogram(data, w, h);
    
    // Memory allocation.
    map                                = new double_packet[256];
    equalize_map                       = new short_packet[256];
    
    if( !histogram || !map || !equalize_map )
       {
       if(histogram)
           delete histogram;
       
       if(map)
           delete [] map;
       
       if(equalize_map)
           delete [] equalize_map;
        
       kdWarning() << ("ImageFilters::equalizeImage: Unable to allocate memory!") << endl;
       return;
       }
    
    // Integrate the histogram to get the equalization map.
     
    memset(&intensity, 0, sizeof(struct double_packet));
    memset(&high,      0, sizeof(struct double_packet));            
    memset(&low,       0, sizeof(struct double_packet));
    
    for(i = 0 ; i <= 255 ; ++i)
       {
       intensity.red   += histogram->getValue(Digikam::ImageHistogram::RedChannel, i);
       intensity.green += histogram->getValue(Digikam::ImageHistogram::GreenChannel, i);
       intensity.blue  += histogram->getValue(Digikam::ImageHistogram::BlueChannel, i);
       intensity.alpha += histogram->getValue(Digikam::ImageHistogram::AlphaChannel, i);
       map[i] = intensity;
       }
    
    low =  map[0];
    high = map[255];
    memset(equalize_map, 0, 256*sizeof(short_packet));
    
    for(i = 0 ; i <= 255 ; ++i)
       {
       if(high.red != low.red)
          equalize_map[i].red=(unsigned short)((65535*(map[i].red-low.red))/(high.red-low.red));
       if(high.green != low.green)
          equalize_map[i].green=(unsigned short)((65535*(map[i].green-low.green))/(high.green-low.green));
       if(high.blue != low.blue)
          equalize_map[i].blue=(unsigned short)((65535*(map[i].blue-low.blue))/(high.blue-low.blue));
       if(high.alpha != low.alpha)
          equalize_map[i].alpha=(unsigned short)((65535*(map[i].alpha-low.alpha))/(high.alpha-low.alpha));
       }
    
    delete histogram;
    delete [] map;
    
    // Stretch the histogram.
    
    for(y = 0 ; y < h ; ++y)
       {
       q = data + (w * y);
            
       for(x = 0 ; x < w ; ++x)
          {
          if(low.red != high.red)
             r = (equalize_map[(unsigned char)(q[x] >> 16)].red)/257;
          else
             r = (unsigned char)(q[x] >> 16);      
          if(low.green != high.green)
             g = (equalize_map[(unsigned char)(q[x] >> 8)].green)/257;
          else
             g = (unsigned char)(q[x] >> 8);       
          if(low.blue != high.blue)
             b = (equalize_map[(unsigned char)(q[x])].blue)/257;
          else
             b = (unsigned char)(q[x]);            
          if(low.alpha != high.alpha)
             a = (equalize_map[(unsigned char)(q[x] >> 24)].alpha)/257;
          else
             a = (unsigned char)(q[x] >> 24);      
          
          q[x] = (unsigned int)(a << 24) + (unsigned int)(r << 16) + 
                 (unsigned int)(g << 8)  + (unsigned int)(b);                
          }
       }
    
    delete [] equalize_map;
}

/////////////////////////////////////////////////////////////////////////////////
// Performs histogram normalization of the image. The algorithm normalizes 
// the pixel values from an image for to span the full range 
// of color values. This is a contrast enhancement technique. 

void ImageFilters::stretchContrastImage(uint *data, int w, int h)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::stretchContrastImage: no image data available!") << endl;
       return;
       }

    struct double_packet  high, low, intensity;
    struct short_packet  *normalize_map;
    long long             number_pixels;
    int                   x, y;
    unsigned int         *q;
    register long         i;
    unsigned long         threshold_intensity;
    unsigned char         r, g, b, a;
        
    // Create an histogram of the current image.     
    Digikam::ImageHistogram *histogram = new Digikam::ImageHistogram(data, w, h);
    
    // Memory allocation.
    normalize_map = new short_packet[256];
    
    if( !histogram || !normalize_map )
       {
       if(histogram)
           delete histogram;
       
       if(normalize_map)
           delete [] normalize_map;
        
       kdWarning() << ("ImageFilters::stretchContrastImage: Unable to allocate memory!") << endl;
       return;
       }

    // Find the histogram boundaries by locating the 0.1 percent levels.
    
    number_pixels = (long long)(w*h);
    threshold_intensity = number_pixels / 1000;

    memset(&high, 0, sizeof(struct double_packet));            
    memset(&low,  0, sizeof(struct double_packet));
        
    // Red. 
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.red = 255 ; high.red != 0 ; high.red--)
       {
       intensity.red += histogram->getValue(Digikam::ImageHistogram::RedChannel, 
                                            (unsigned char)high.red);
       
       if( intensity.red > threshold_intensity )
          break;
       }
    
    if( low.red == high.red )
       {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
        
       for(low.red = 0 ; low.red < 255 ; low.red++)
          {
          intensity.red += histogram->getValue(Digikam::ImageHistogram::RedChannel, 
                                               (unsigned char)low.red);
          
          if( intensity.red > threshold_intensity )
              break;
          }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.red = 255 ; high.red != 0 ; high.red--)
          {
          intensity.red += histogram->getValue(Digikam::ImageHistogram::RedChannel, 
                                               (unsigned char)high.red);
          
          if( intensity.red > threshold_intensity )
             break;
          }
       }

    // Green.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.green = 255 ; high.green != 0 ; high.green--)
       {
       intensity.green += histogram->getValue(Digikam::ImageHistogram::GreenChannel, 
                                              (unsigned char)high.green);
       
       if( intensity.green > threshold_intensity )
          break;
       }
    
    if( low.green == high.green )
       {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(low.green = 0 ; low.green < 255 ; low.green++)
          {
          intensity.green += histogram->getValue(Digikam::ImageHistogram::GreenChannel,
                                                 (unsigned char)low.green);
          
          if( intensity.green > threshold_intensity )
             break;
          }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.green = 255 ; high.green != 0 ; high.green--)
          {
          intensity.green += histogram->getValue(Digikam::ImageHistogram::GreenChannel, 
                                                 (unsigned char)high.green);
          
          if( intensity.green > threshold_intensity )
             break;
          }
       }

    // Blue.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.blue = 255 ; high.blue != 0 ; high.blue--)
       {
       intensity.blue += histogram->getValue(Digikam::ImageHistogram::BlueChannel, 
                                             (unsigned char)high.blue);
       
       if( intensity.blue > threshold_intensity )
          break;
       }
       
    if( low.blue == high.blue )
       {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
        
       for(low.blue = 0 ; low.blue < 255 ; low.blue++)
          {
          intensity.blue += histogram->getValue(Digikam::ImageHistogram::BlueChannel, 
                                                (unsigned char)low.blue);
          
          if( intensity.blue > threshold_intensity )
              break;
          }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.blue = 255 ; high.blue != 0 ; high.blue--)
          {
          intensity.blue += histogram->getValue(Digikam::ImageHistogram::BlueChannel, 
                                                (unsigned char)high.blue);
          
          if( intensity.blue > threshold_intensity )
             break;
          }
       }

    // Alpha.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.alpha = 255 ; high.alpha != 0 ; high.alpha--)
       {
       intensity.alpha += histogram->getValue(Digikam::ImageHistogram::AlphaChannel, 
                                              (unsigned char)high.alpha);
       
       if( intensity.alpha > threshold_intensity )
          break;
       }
       
    if( low.alpha == high.alpha )
       {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(low.alpha = 0 ; low.alpha < 255 ; low.alpha++)
          {
          intensity.alpha += histogram->getValue(Digikam::ImageHistogram::AlphaChannel, 
                                                 (unsigned char)low.alpha);
          
          if( intensity.alpha > threshold_intensity )
             break;
          }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.alpha = 255 ; high.alpha != 0 ; high.alpha--)
          {
          intensity.alpha += histogram->getValue(Digikam::ImageHistogram::AlphaChannel, 
                                                 (unsigned char)high.alpha);
          
          if( intensity.alpha > threshold_intensity )
             break;
          }
       }
    
    delete histogram;

    // Stretch the histogram to create the normalized image mapping.
    
    memset(normalize_map, 0, 256*sizeof(struct short_packet));
    
    for(i = 0 ; i <= (long)255 ; ++i)
       {
       if(i < (long) low.red)
          normalize_map[i].red = 0;
       else if (i > (long) high.red)
          normalize_map[i].red = 65535;
       else if (low.red != high.red)
          normalize_map[i].red = (unsigned short)((65535*(i-low.red))/(high.red-low.red));

       if(i < (long) low.green)
          normalize_map[i].green = 0;
       else if (i > (long) high.green)
          normalize_map[i].green = 65535;
       else if (low.green != high.green)
          normalize_map[i].green = (unsigned short)((65535*(i-low.green))/(high.green-low.green));

       if(i < (long) low.blue)
          normalize_map[i].blue = 0;
       else if (i > (long) high.blue)
          normalize_map[i].blue = 65535;
       else if (low.blue != high.blue)
          normalize_map[i].blue = (unsigned short)((65535*(i-low.blue))/(high.blue-low.blue));

       if(i < (long) low.alpha)
          normalize_map[i].alpha = 0;
       else if (i > (long) high.alpha)
          normalize_map[i].alpha = 65535;
       else if (low.alpha != high.alpha)
          normalize_map[i].alpha = (unsigned short)((65535*(i-low.alpha))/(high.alpha-low.alpha));
       }

    for(y = 0 ; y < h ; ++y)
       {
       q = data + (w * y);
            
       for(x = 0 ; x < w ; ++x)
          {
          if(low.red != high.red)
             r = (normalize_map[(unsigned char)(q[x] >> 16)].red)/257;
          else
             r = (unsigned char)(q[x] >> 16);      
          if(low.green != high.green)
             g = (normalize_map[(unsigned char)(q[x] >> 8)].green)/257;
          else
             g = (unsigned char)(q[x] >> 8);       
          if(low.blue != high.blue)
             b = (normalize_map[(unsigned char)(q[x])].blue)/257;
          else
             b = (unsigned char)(q[x]);            
          if(low.alpha != high.alpha)
             a = (normalize_map[(unsigned char)(q[x] >> 24)].alpha)/257;
          else
             a = (unsigned char)(q[x] >> 24);      
          
          q[x] = (unsigned int)(a << 24) + (unsigned int)(r << 16) + 
                 (unsigned int)(g << 8)  + (unsigned int)(b);                
          }
       }
    
    delete [] normalize_map;
}

//////////////////////////////////////////////////////////////////////////////
// Simple image normalization fonction inspired from Gimp 2.0

void ImageFilters::normalizeImage(uint *data, int w, int h)
{
    NormalizeParam param;
    int    x, i, b;
    uchar  range;
    uchar *p;

    // Find min. and max. values.
    
    param.min   = 255;
    param.max   = 0;

    for (i = 0 ; i < h*w ; ++i)
        {
        p = (uchar *)(data + i);
        
        for (b = 0 ; b < 3 ; ++b)
           {
           if (p[b] < param.min)
              param.min = p[b];
           if (p[b] > param.max)
              param.max = p[b];
           }
        }
    
    // Calculate LUT. 

    range = (uchar)(param.max - param.min);

    if (range != 0)
       {
       for (x = (int)param.min ; x <= (int)param.max ; ++x)
          param.lut[x] = (uchar)(255 * (x - param.min) / range);
       }
    else
       param.lut[(int)param.min] = (uchar)param.min;

    // Apply LUT to image.
       
    for (i = 0 ; i < h*w ; ++i)
        {
        p = (uchar *)(data + i);
        
        for (b = 0 ; b < 3 ; ++b)
           p[b] = param.lut[p[b]];
  
        p[3] = p[3];
        }
}

//////////////////////////////////////////////////////////////////////////////
// Performs histogram auto correction of levels.

void ImageFilters::autoLevelsCorrectionImage(uint *data, int w, int h)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::autoLevelsCorrectionImage: no image data available!")
                   << endl;
       return;
       }
  
    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];
       
    // Create an histogram of the current image.     
    Digikam::ImageHistogram *histogram = new Digikam::ImageHistogram(data, w, h);
  
    // Create an empty instance of levels to use.
    Digikam::ImageLevels *levels = new Digikam::ImageLevels();      
  
    // Initialize an auto levels correction of the histogram.
    levels->levelsAuto(histogram);

    // Calculate the LUT to apply on the image.
    levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel);
  
    // Apply the lut to the image.
    levels->levelsLutProcess(data, desData, w, h);
  
    memcpy (data, desData, w*h*4);

    delete [] desData;
    delete histogram;
    delete levels;
}

//////////////////////////////////////////////////////////////////////////////
// Performs image colors inversion. This tool is used for negate image 
// resulting of a positive film scanned.

void ImageFilters::invertImage(uint *data, int w, int h)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::invertImage: no image data available!")
                   << endl;
       return;
       }
       
    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];

    int LineWidth = w * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
      
    uchar* bits    = (uchar*)data;
    uchar* newBits = (uchar*)desData;

    int i = 0;
    
    for (int y = 0 ; y < h ; ++y)
        {
        for (int x = 0 ; x < w ; ++x)
            {
            i = y * LineWidth + 4 * x;

            newBits[i+3] = 255 - bits[i+3];
            newBits[i+2] = 255 - bits[i+2];
            newBits[i+1] = 255 - bits[i+1];
            newBits[ i ] = 255 - bits[ i ];
            }
        }
                        
    memcpy (data, desData, w*h*4);
    delete [] desData;
}

//////////////////////////////////////////////////////////////////////////////
// Performs blur image with less pixels.

void ImageFilters::smartBlurImage(uint *data, int Width, int Height)
{
    if (!data || !Width || !Height)
       {
       kdWarning() << ("ImageFilters::smartBlurImage: no image data available!")
                   << endl;
       return;
       }
       
    int LineWidth = Width * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar* Bits = (uchar*)data;
        
    register int i = 0, j = 0, k = 0;
    
    for (int h = 1; h < Height - 1; h++)
        {
        for (int w = 1; w < Width - 1; w++)
            {
            i = h * LineWidth + 4 * w;
            j = (h + 1) * LineWidth + 4 * w;
            k = (h - 1) * LineWidth + 4 * w;

            Bits[i+2] = (Bits[i-2] + Bits[j-2] + Bits[k-2] +
                         Bits[i+2] + Bits[j+2] + Bits[k+2] +
                         Bits[i+6] + Bits[j+6] + Bits[k+6]) / 9;
            Bits[i+1] = (Bits[i-3] + Bits[j-3] + Bits[k-3] +
                         Bits[i+1] + Bits[j+1] + Bits[k+1] +
                         Bits[i+5] + Bits[j+5] + Bits[k+5]) / 9;
            Bits[ i ] = (Bits[i-4] + Bits[j-4] + Bits[k-4] +
                         Bits[ i ] + Bits[ j ] + Bits[ k ] +
                         Bits[i+4] + Bits[j+4] + Bits[k+4]) / 9;
            }
        }
}

/* Function to apply the GaussianBlur on an image
 *
 * data             => The image data in RGBA mode.  
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Radius           => blur matrix radius                                         
 *                                                                                 
 * Theory           => this is the famous gaussian blur like in photoshop or gimp.  
 */
void ImageFilters::gaussianBlurImage(uint *data, int Width, int Height, int Radius)
{
    if (!data || !Width || !Height)
       {
       kdWarning() << ("ImageFilters::gaussianBlurImage: no image data available!")
                   << endl;
       return;
       }

    if (Radius > 100) Radius = 100;
    
    // Gaussian kernel computation using the Radius parameter.
      
    int    nKSize, nCenter;
    double x, sd, factor, lnsd, lnfactor;
    register int i, j, n, h, w;

    nKSize = 2 * Radius + 1;
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
    
    // Now, we need to convolve the image descriptor.
    // I've worked hard here, but I think this is a very smart       
    // way to convolve an array, its very hard to explain how I reach    
    // this, but the trick here its to store the sum used by the       
    // previous pixel, so we sum with the other pixels that wasn't get.
    
    int nSumR, nSumG, nSumB, nCount;
    int nKernelWidth = Radius * 2 + 1;
    int nStride = GetStride(Width);
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar* pInBits  = (uchar*)data;
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
    
    nSumR = nSumG = nSumB = nCount = i = j = 0;

    // Now, we enter in the main loop
    
    for (h = 0; h < Height; h++, i += nStride)
        {
        for (w = 0; w < Width; w++, i += 4)
            {
            // first of all, we need to blur the horizontal lines
                
            for (n = -Radius; n <= Radius; n++)
               {
               // if is inside...
               if (IsInside (Width, Height, w + n, h))
                    {
                    // we points to the pixel
                    j = i + n * 4;
                    
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][pInBits[j+2]];
                    nSumG += arrMult[n + Radius][pInBits[j+1]];
                    nSumB += arrMult[n + Radius][pInBits[ j ]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to blur bits the horizontal blur values
            pBlur[i+2] = (uchar)CLAMP (nSumR / nCount, 0, 255);
            pBlur[i+1] = (uchar)CLAMP (nSumG / nCount, 0, 255);
            pBlur[ i ] = (uchar)CLAMP (nSumB / nCount, 0, 255);
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; w < Width; w++, i = w * 4)
        {
        for (h = 0; h < Height; h++, i += LineWidth)
            {
            // first of all, we need to blur the vertical lines
            for (n = -Radius; n <= Radius; n++)
                {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                    {
                    // we points to the pixel
                    j = i + n * LineWidth;
                      
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][pBlur[j+2]];
                    nSumG += arrMult[n + Radius][pBlur[j+1]];
                    nSumB += arrMult[n + Radius][pBlur[ j ]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to bits the vertical blur values
            pOutBits[i+2] = (uchar)CLAMP (nSumR / nCount, 0, 255);
            pOutBits[i+1] = (uchar)CLAMP (nSumG / nCount, 0, 255);
            pOutBits[ i ] = (uchar)CLAMP (nSumB / nCount, 0, 255);
                
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        }

    memcpy (data, pOutBits, BitCount);   
       
    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
    delete [] pOutBits;
    delete [] Kernel;
}

void ImageFilters::channelMixerImage(uint *data, int Width, int Height, bool bPreserveLum, bool bMonochrome,
                                     float rrGain, float rgGain, float rbGain,
                                     float grGain, float ggGain, float gbGain,
                                     float brGain, float bgGain, float bbGain, 
                                     bool overIndicator)
{
    if (!data || !Width || !Height)
       {
       kdWarning() << ("ImageFilters::channelMixerImage: no image data available!")
                   << endl;
       return;
       }
        
    register int h, w, i = 0;
    uchar        nGray, red, green , blue;
    imageData    imagedata;
    
    double rnorm = CalculateNorm (rrGain, rgGain, rbGain, bPreserveLum);
    double gnorm = CalculateNorm (grGain, ggGain, gbGain, bPreserveLum);
    double bnorm = CalculateNorm (brGain, bgGain, bbGain, bPreserveLum);
        
    for (h = 0; h < Height; h++)
        {
        for (w = 0; w < Width; w++, i++)
            {
            imagedata.raw = data[i];
            red           = imagedata.channel.red;
            green         = imagedata.channel.green;
            blue          = imagedata.channel.blue;
            
            if (bMonochrome)
                {
                nGray = MixPixel (rrGain, rgGain, rbGain, red, green, blue, rnorm, overIndicator);
                imagedata.channel.red = imagedata.channel.green = imagedata.channel.blue = nGray;
                }
            else
                {
                imagedata.channel.red   = MixPixel (rrGain, rgGain, rbGain, red, green, blue, rnorm, overIndicator);
                imagedata.channel.green = MixPixel (grGain, ggGain, gbGain, red, green, blue, gnorm, overIndicator);
                imagedata.channel.blue  = MixPixel (brGain, bgGain, bbGain, red, green, blue, bnorm, overIndicator);
                }
            
            data[i] = imagedata.raw;
            }
        }
}

// Change color tonality of an image to appling a RGB color mask.

void ImageFilters::changeTonality(uint *data, int width, int height, int redMask, int greenMask, int blueMask)
{
    if (!data || !width || !height)
       {
       kdWarning() << ("ImageFilters::changeTonality: no image data available!")
                   << endl;
       return;
       }

    int       red, green , blue;
    int       hue, sat, lig;
    float     gray;
    imageData imagedata;
    
    hue = redMask;
    sat = greenMask;
    lig = blueMask;
    
    Digikam::rgb_to_hsl(hue, sat, lig);

    for (int i = 0; i < width*height; i++) 
       {
       imagedata.raw = data[i];
       red           = (int)imagedata.channel.red;
       green         = (int)imagedata.channel.green;
       blue          = (int)imagedata.channel.blue;
        
       // Convert to grayscale using tonal mask
        
       gray  = 0.3 * red + 0.59 * green + 0.11 * blue;
       red   = ROUND (gray);
       green = red;
       blue  = red;

       red   = hue;
       green = sat;
        
       Digikam::hsl_to_rgb(red, green, blue);
        
       imagedata.channel.red   = (uchar)red;
       imagedata.channel.green = (uchar)green;
       imagedata.channel.blue  = (uchar)blue;
       data[i] = imagedata.raw;
       }
}

//////////////////////////////////////////////////////////////////////////////
/* Function to apply the sharpen filter on an image
 *
 * data             => The image data in RGBA mode.  
 * w                => Width of image.                          
 * h                => Height of image.                            
 * r                => sharpen matrix radius                                         
 *                                                                                 
 * Theory           => this is the famous sharpen image filter like in photoshop or gimp.  
 */
void ImageFilters::sharpenImage(uint* data, int w, int h, int r)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::sharpenImage: no image data available!")
                   << endl;
       return;
       }
    
    // initialize the LUTs

    int fact = 100 - r;
    if (fact < 1)
        fact = 1;

    int negLUT[256];
    int posLUT[256];
    
    for (int i = 0; i < 256; i++)
    {
        posLUT[i] = 800 * i / fact;
        negLUT[i] = (4 + posLUT[i] - (i << 3)) >> 3;
    }

    unsigned int* dstData = new unsigned int[w*h];
    
    // work with four rows at one time

    unsigned char* src_rows[4];
    unsigned char* src_ptr;
    unsigned char* dst_row;
    int*           neg_rows[4]; 
    int*           neg_ptr;
    int            row;
    int            count;

    int  width = sizeof(unsigned int)*w;

    for (row = 0; row < 4; row++)
    {
        src_rows[row] = new unsigned char[width];
        neg_rows[row] = new int[width];
    }       

    dst_row = new unsigned char[width];
    
    // Pre-load the first row for the filter...

    memcpy(src_rows[0], data, width); 

    int i;
    for ( i = width, src_ptr = src_rows[0], neg_ptr = neg_rows[0];
         i > 0;
         i--, src_ptr++, neg_ptr++)
        *neg_ptr = negLUT[*src_ptr]; 

    row   = 1;
    count = 1;                                    

    for (int y = 0; y < h; y++)
    {
        // Load the next pixel row...

        if ((y + 1) < h)
        {
            
            // Check to see if our src_rows[] array is overflowing yet...

            if (count >= 3)
                count--;

            // Grab the next row...

            memcpy(src_rows[row], data + y*w, width); 
            for (i = width, src_ptr = src_rows[row], neg_ptr = neg_rows[row];
                 i > 0;
                 i--, src_ptr++, neg_ptr++)
                *neg_ptr = negLUT[*src_ptr];

            count++;
            row = (row + 1) & 3;
        }

        else
        {
            // No more pixels at the bottom...  Drop the oldest samples...

            count--;
        }

        // Now sharpen pixels and save the results...

        if (count == 3)
        {
            uchar* src  = src_rows[(row + 2) & 3];
            uchar* dst  = dst_row;
            int*   neg0 = neg_rows[(row + 1) & 3] + 4;
            int*   neg1 = neg_rows[(row + 2) & 3] + 4;
            int*   neg2 = neg_rows[(row + 3) & 3] + 4;
            
            // New pixel value 
            int pixel;         

            int wm = w;
            
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            wm -= 2;

            while (wm > 0)
            {
                pixel = (posLUT[*src++] - neg0[-4] - neg0[0] - neg0[4] -
                         neg1[-4] - neg1[4] -
                         neg2[-4] - neg2[0] - neg2[4]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                pixel = (posLUT[*src++] - neg0[-3] - neg0[1] - neg0[5] -
                         neg1[-3] - neg1[5] -
                         neg2[-3] - neg2[1] - neg2[5]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                pixel = (posLUT[*src++] - neg0[-2] - neg0[2] - neg0[6] -
                         neg1[-2] - neg1[6] -
                         neg2[-2] - neg2[2] - neg2[6]);
                pixel = (pixel + 4) >> 3;
                *dst++ = CLAMP0255 (pixel);

                *dst++ = *src++;

                neg0 += 4;
                neg1 += 4;
                neg2 += 4;
                wm --;
            }

            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            
            
            // Set the row...
            memcpy(dstData + y*w, dst_row, width); 
        }
        else if (count == 2)
        {
            if (y == 0)
            {
                // first row 
                memcpy(dstData + y*w, src_rows[0], width);
            }
            else
            {
                // last row 
                memcpy(dstData + y*w, src_rows[(h-1) & 3], width);
            }
        }

    }

    memcpy(data, dstData, w*h*sizeof(uint));
    delete [] dstData;
}

void ImageFilters::hueSaturationLightnessImage(uint* data, int w, int h, double hu, double sa, double li)
{
    if (!data || !w || !h)
       {
       kdWarning() << ("ImageFilters::hueSaturationLightnessImage: no image data available!")
                   << endl;
       return;
       }
    
    // Calculate HSL Transfers.

    int value;
    register int i;
    HSLParam hsl;
    
    for (i = 0; i < 256; i++)
       {
       value = (int)(hu * 255.0 / 360.0);
      
       if ((i + value) < 0)
          hsl.htransfer[i] = 255 + (i + value);
       else if ((i + value) > 255)
          hsl.htransfer[i] = i + value - 255;
       else
          hsl.htransfer[i] = i + value;

       //  Lightness  
       value = (int)(li * 127.0 / 100.0);
       value = CLAMP (value, -255, 255);

       if (value < 0)
          hsl.ltransfer[i] = ((i * (255 + value)) / 255);
       else
          hsl.ltransfer[i] = (i + ((255 - i) * value) / 255);

       //  Saturation  
       value = (int)(sa * 255.0 / 100.0);
       value = CLAMP (value, -255, 255);

       /* This change affects the way saturation is computed. With the
          old code (different code for value < 0), increasing the
          saturation affected muted colors very much, and bright colors
          less. With the new code, it affects muted colors and bright
          colors more or less evenly. For enhancing the color in photos,
          the new behavior is exactly what you want. It's hard for me
          to imagine a case in which the old behavior is better.
       */
       hsl.stransfer[i] = CLAMP ((i * (255 + value)) / 255, 0, 255);
       }
        
    // Apply HSL.
    uchar* c;
    int r, g, b;

    unsigned int* ptr = data;

    for (i = 0 ; i < w*h ; i++) 
       {
       c = (unsigned char*) ptr;

       b = c[0];
       g = c[1];
       r = c[2];

       Digikam::rgb_to_hsl(r, g, b);
         
       r = hsl.htransfer[r];
       g = hsl.stransfer[g];
       b = hsl.ltransfer[b];

       Digikam::hsl_to_rgb (r, g, b);

       c[0] = b;
       c[1] = g;
       c[2] = r;

       ptr++;
       }
}
       
}  // NameSpace Digikam
