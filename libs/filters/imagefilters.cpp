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
 * Original channel mixer algorithm copyrighted 2002 by 
 * Martin Guldahl <mguldahl at xmission dot com> from Gimp 2.2 
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

// C++ includes.

#include <cstring>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Digikam includes.
 
#include <imagehistogram.h>
#include <imagelevels.h>
#include <imageiface.h>

// Local includes.

#include "gaussianblur.h"
#include "sharpen.h"
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
    register long         i;               
    
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
    
    for(i = 0 ; i <= 255 ; i++)
       {
       intensity.red   += histogram->getValue(Digikam::ImageHistogram::RedChannel, i);
       intensity.green += histogram->getValue(Digikam::ImageHistogram::GreenChannel, i);
       intensity.blue  += histogram->getValue(Digikam::ImageHistogram::BlueChannel, i);
       intensity.alpha += histogram->getValue(Digikam::ImageHistogram::AlphaChannel, i);
       map[i]          = intensity;
       }
    
    low =  map[0];
    high = map[255];
    memset(equalize_map, 0, 256*sizeof(short_packet));
    
    for(i = 0 ; i <= 255 ; i++)
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

    uchar     red, green, blue, alpha;
    imageData imagedata;
    
    for(i = 0 ; i < w*h ; i++)
       {
       imagedata.raw = data[i];
       red           = imagedata.channel.red;
       green         = imagedata.channel.green;
       blue          = imagedata.channel.blue;
       alpha         = imagedata.channel.alpha;

       if(low.red != high.red)
           red = (equalize_map[red].red)/257;
                
       if(low.green != high.green)
           green = (equalize_map[green].green)/257;
            
       if(low.blue != high.blue)
           blue = (equalize_map[blue].blue)/257;
            
       if(low.alpha != high.alpha)
           alpha = (equalize_map[alpha].alpha)/257;
                    
       imagedata.channel.red   = red;
       imagedata.channel.green = green;
       imagedata.channel.blue  = blue;
       imagedata.channel.alpha = alpha;
       data[i] = imagedata.raw;
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
    register long         i;
    unsigned long         threshold_intensity;
        
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
    
    for(i = 0 ; i <= (long)255 ; i++)
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

    uchar     red, green, blue, alpha;
    imageData imagedata;
    
    for(i = 0 ; i < w*h ; i++)
       {
       imagedata.raw = data[i];
       red           = imagedata.channel.red;
       green         = imagedata.channel.green;
       blue          = imagedata.channel.blue;
       alpha         = imagedata.channel.alpha;

       if(low.red != high.red)
           red = (normalize_map[red].red)/257;
                
       if(low.green != high.green)
           green = (normalize_map[green].green)/257;
            
       if(low.blue != high.blue)
           blue = (normalize_map[blue].blue)/257;
            
       if(low.alpha != high.alpha)
           alpha = (normalize_map[alpha].alpha)/257;
                    
       imagedata.channel.red   = red;
       imagedata.channel.green = green;
       imagedata.channel.blue  = blue;
       imagedata.channel.alpha = alpha;
       data[i] = imagedata.raw;
       }
    
    delete [] normalize_map;
}

//////////////////////////////////////////////////////////////////////////////
// Simple image normalization fonction inspired from Gimp 2.0

void ImageFilters::normalizeImage(uint *data, int w, int h)
{
    NormalizeParam  param;
    int             x, i;
    uchar           range;

    // Find min. and max. values.
    
    param.min   = 255;
    param.max   = 0;

    uchar         red, green, blue;
    imageData     imagedata;
    
    for (i = 0; i < w*h; i++)
        {
        imagedata.raw = data[i];
        red = imagedata.channel.red;
        
        if (red < param.min) param.min = red;
        if (red > param.max) param.max = red;

        green = imagedata.channel.green;
        
        if (green < param.min) param.min = green;
        if (green > param.max) param.max = green;

        blue = imagedata.channel.blue;
    
        if (blue < param.min) param.min = blue;
        if (blue > param.max) param.max = blue;
        }

    // Calculate LUT. 

    range = (uchar)(param.max - param.min);

    if (range != 0)
       {
       for (x = (int)param.min ; x <= (int)param.max ; x++)
          param.lut[x] = (uchar)(255 * (x - param.min) / range);
       }
    else
       param.lut[(int)param.min] = (uchar)param.min;

    // Apply LUT to image.
       
    for (i = 0; i < w*h; i++)
        {
        imagedata.raw = data[i];
        
        red = imagedata.channel.red;
        imagedata.channel.red = param.lut[red];

        green = imagedata.channel.green;
        imagedata.channel.green = param.lut[green];
        
        blue = imagedata.channel.blue;
        imagedata.channel.blue = param.lut[blue];
        
        data[i] = imagedata.raw;
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
       
    int           i;
    uchar         red, green, blue;
    imageData     imagedata;
    
    for (i = 0; i < w*h; i++)
        {
        imagedata.raw = data[i];
        red           = imagedata.channel.red;
        green         = imagedata.channel.green;
        blue          = imagedata.channel.blue;
    
        imagedata.channel.red   = 255 - red;
        imagedata.channel.green = 255 - green;
        imagedata.channel.blue  = 255 - blue;
        data[i] = imagedata.raw;
        }
}

//////////////////////////////////////////////////////////////////////////////
/* Function to apply the GaussianBlur on an image. This method do not use a 
 * dedicaced thread.
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
    if (Radius <= 0) return;

    QImage orgImage(Width, Height, 32);
    memcpy( orgImage.bits(), data, orgImage.numBytes() );

    Digikam::GaussianBlur *threadedFilterVar = new Digikam::GaussianBlur(&orgImage, 0L, Radius);
    QImage imDest = threadedFilterVar->getTargetImage();
    memcpy( data, imDest.bits(), imDest.numBytes() );
    delete threadedFilterVar;
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
        
    register int i;
    uchar        nGray, red, green , blue;
    imageData    imagedata;
    
    double rnorm = CalculateNorm (rrGain, rgGain, rbGain, bPreserveLum);
    double gnorm = CalculateNorm (grGain, ggGain, gbGain, bPreserveLum);
    double bnorm = CalculateNorm (brGain, bgGain, bbGain, bPreserveLum);
        
    for (i = 0; i < Width*Height; i++)
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

// Change color tonality of an image to appling a RGB color mask.

void ImageFilters::changeTonality(uint *data, int width, int height, int redMask, int greenMask, int blueMask)
{
    if (!data || !width || !height)
       {
       kdWarning() << ("ImageFilters::changeTonality: no image data available!")
                   << endl;
       return;
       }

    int       hue, sat, lig;
    float     gray;
    
    int       red, green, blue;
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
/* Function to apply the sharpen filter on an image. This method do not use a 
 * dedicaced thread.
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

    if (r > 100) r = 100;
    if (r <= 0) return;

    QImage orgImage(w, h, 32);
    memcpy( orgImage.bits(), data, orgImage.numBytes() );

    Digikam::Sharpen *threadedFilter = new Digikam::Sharpen(&orgImage, 0L, r);
    QImage imDest = threadedFilter->getTargetImage();
    memcpy( data, imDest.bits(), imDest.numBytes() );
    delete threadedFilter;
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

    int          value;
    register int i;
    HSLParam     hsl;
    
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
    
    int       red, green, blue;
    imageData imagedata;
    
    for (int i = 0; i < w*h; i++) 
       {
       imagedata.raw = data[i];
       red           = (int)imagedata.channel.red;
       green         = (int)imagedata.channel.green;
       blue          = (int)imagedata.channel.blue;
        
       Digikam::rgb_to_hsl(red, green, blue);
         
       red   = hsl.htransfer[red];
       green = hsl.stransfer[green];
       blue  = hsl.ltransfer[blue];

       Digikam::hsl_to_rgb(red, green, blue);
        
       imagedata.channel.red   = (uchar)red;
       imagedata.channel.green = (uchar)green;
       imagedata.channel.blue  = (uchar)blue;
       data[i] = imagedata.raw;
       }
}

//////////////////////////////////////////////////////////////////////////////
/* Function to perform pixel antialiasing.
 *
 * data             => The original image data in RGBA mode.  
 * w                => Width of image.                          
 * h                => Height of image.                            
 * X                => x position of pixel.                                         
 * Y                => x position of pixel.                                         
 * A, R, G, B       => RGBA color component result of pixel aliased.
 *                                                                                 
 * Theory           => this method is used to smooth target image in transformation 
 *                     method like free rotation.  
 */
void ImageFilters::pixelAntiAliasing (uchar *data, int Width, int Height, double X, double Y, 
                                             uchar *A, uchar *R, uchar *G, uchar *B)
{
    int nX, nY, j;
    double lfWeightX[2], lfWeightY[2], lfWeight;
    double lfTotalR = 0.0, lfTotalG = 0.0, lfTotalB = 0.0, lfTotalA = 0.0;

    nX = (int)X;
    nY = (int)Y;

    if (Y >= 0.0)
        lfWeightY[0] = 1.0 - (lfWeightY[1] = Y - (double)nY);
    else
        lfWeightY[1] = 1.0 - (lfWeightY[0] = -(Y - (double)nY));

    if (X >= 0.0)
        lfWeightX[0] = 1.0 - (lfWeightX[1] = X - (double)nX);
    else
        lfWeightX[1] = 1.0 - (lfWeightX[0] = -(X - (double)nX));

    for (int loopx = 0; loopx <= 1; loopx++)
        {
        for (int loopy = 0; loopy <= 1; loopy++)
            {
            lfWeight = lfWeightX[loopx] * lfWeightY[loopy];
            j = setPositionAdjusted (Width, Height, nX + loopx, nY + loopy);

            lfTotalB += ((double)data[j++] * lfWeight);
            lfTotalG += ((double)data[j++] * lfWeight);
            lfTotalR += ((double)data[j++] * lfWeight);
            lfTotalA += ((double)data[j++] * lfWeight);
            }
        }
         
    *B = CLAMP0255 ((int)lfTotalB);
    *G = CLAMP0255 ((int)lfTotalG);
    *R = CLAMP0255 ((int)lfTotalR);
    *A = CLAMP0255 ((int)lfTotalA);
}
       
}  // NameSpace Digikam
