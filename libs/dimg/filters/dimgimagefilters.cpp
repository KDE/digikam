/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-24-01
 * Description : image filters. 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

// Local includes.
 
#include "imagehistogram.h"
#include "imagelevels.h"
#include "dcolor.h"
#include "ddebug.h"
#include "dimggaussianblur.h"
#include "dimgsharpen.h"
#include "dimgimagefilters.h"

namespace Digikam
{

/** Performs an histogram equalisation of the image.
    this method adjusts the brightness of colors across the
    active image so that the histogram for the value channel
    is as nearly as possible flat, that is, so that each possible
    brightness value appears at about the same number of pixels
    as each other value. Sometimes Equalize works wonderfully at
    enhancing the contrasts in an image. Other times it gives
    garbage. It is a very powerful operation, which can either work
    miracles on an image or destroy it.*/
void DImgImageFilters::equalizeImage(uchar *data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       DWarning() << ("DImgImageFilters::equalizeImage: no image data available!") << endl;
       return;
    }
       
    struct double_packet  high, low, intensity;
    struct double_packet *map;
    struct int_packet    *equalize_map;
    register long         i;               
    
    // Create an histogram of the current image.     
    ImageHistogram *histogram = new ImageHistogram(data, w, h, sixteenBit);
    
    // Memory allocation.
    map          = new double_packet[histogram->getHistogramSegment()];
    equalize_map = new int_packet[histogram->getHistogramSegment()];
    
    if( !histogram || !map || !equalize_map )
    {
       if(histogram)
           delete histogram;
       
       if(map)
           delete [] map;
       
       if(equalize_map)
           delete [] equalize_map;
        
       DWarning() << ("DImgImageFilters::equalizeImage: Unable to allocate memory!") << endl;
       return;
    }
    
    // Integrate the histogram to get the equalization map.
     
    memset(&intensity, 0, sizeof(struct double_packet));
    memset(&high,      0, sizeof(struct double_packet));            
    memset(&low,       0, sizeof(struct double_packet));
    
    for(i = 0 ; i < histogram->getHistogramSegment() ; i++)
    {
       intensity.red   += histogram->getValue(ImageHistogram::RedChannel, i);
       intensity.green += histogram->getValue(ImageHistogram::GreenChannel, i);
       intensity.blue  += histogram->getValue(ImageHistogram::BlueChannel, i);
       intensity.alpha += histogram->getValue(ImageHistogram::AlphaChannel, i);
       map[i]          = intensity;
    }

    // Stretch the histogram.
    
    low  = map[0];
    high = map[histogram->getHistogramSegment()-1];
    memset(equalize_map, 0, histogram->getHistogramSegment()*sizeof(int_packet));
    
    for(i = 0 ; i < histogram->getHistogramSegment() ; i++)
    {
       if(high.red != low.red)
          equalize_map[i].red = (uint)(((256*histogram->getHistogramSegment() -1) *
                                (map[i].red-low.red))/(high.red-low.red));
       
       if(high.green != low.green)
          equalize_map[i].green = (uint)(((256*histogram->getHistogramSegment() -1) *
                                  (map[i].green-low.green))/(high.green-low.green));
       
       if(high.blue != low.blue)
          equalize_map[i].blue = (uint)(((256*histogram->getHistogramSegment() -1) *
                                 (map[i].blue-low.blue))/(high.blue-low.blue));
       
       if(high.alpha != low.alpha)
          equalize_map[i].alpha = (uint)(((256*histogram->getHistogramSegment() -1) *
                                  (map[i].alpha-low.alpha))/(high.alpha-low.alpha));
    }
    
    delete histogram;
    delete [] map;
    
    // Apply results to image.

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar *ptr = data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if(low.red != high.red)
                red = (equalize_map[red].red)/257;
                        
            if(low.green != high.green)
                green = (equalize_map[green].green)/257;
                    
            if(low.blue != high.blue)
                blue = (equalize_map[blue].blue)/257;
                    
            if(low.alpha != high.alpha)
                alpha = (equalize_map[alpha].alpha)/257;
                            
            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short *ptr = (unsigned short *)data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if(low.red != high.red)
                red = (equalize_map[red].red)/257;
                        
            if(low.green != high.green)
                green = (equalize_map[green].green)/257;
                    
            if(low.blue != high.blue)
                blue = (equalize_map[blue].blue)/257;
                    
            if(low.alpha != high.alpha)
                alpha = (equalize_map[alpha].alpha)/257;
                            
            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr += 4;
        }
    }
    
    delete [] equalize_map;
}

/** Performs histogram normalization of the image. The algorithm normalizes
    the pixel values from an image for to span the full range
    of color values. This is a contrast enhancement technique.*/
void DImgImageFilters::stretchContrastImage(uchar *data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       DWarning() << ("DImgImageFilters::stretchContrastImage: no image data available!") << endl;
       return;
    }

    struct double_packet  high, low, intensity;
    struct int_packet    *normalize_map;
    long long             number_pixels;
    register long         i;
    unsigned long         threshold_intensity;
        
    // Create an histogram of the current image.     
    ImageHistogram *histogram = new ImageHistogram(data, w, h, sixteenBit);
    
    // Memory allocation.
    normalize_map = new int_packet[histogram->getHistogramSegment()];
    
    if( !histogram || !normalize_map )
    {
       if(histogram)
           delete histogram;
       
       if(normalize_map)
           delete [] normalize_map;
        
       DWarning() << ("DImgImageFilters::stretchContrastImage: Unable to allocate memory!") << endl;
       return;
    }

    // Find the histogram boundaries by locating the 0.1 percent levels.
    
    number_pixels = (long long)(w*h);
    threshold_intensity = number_pixels / 1000;

    memset(&high, 0, sizeof(struct double_packet));            
    memset(&low,  0, sizeof(struct double_packet));
        
    // Red. 
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.red = histogram->getHistogramSegment()-1 ; high.red != 0 ; high.red--)
    {
       intensity.red += histogram->getValue(ImageHistogram::RedChannel, (int)high.red);
       
       if( intensity.red > threshold_intensity )
          break;
    }
    
    if( low.red == high.red )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
        
       for(low.red = 0 ; low.red < histogram->getHistogramSegment()-1 ; low.red++)
       {
          intensity.red += histogram->getValue(ImageHistogram::RedChannel, (int)low.red);
          
          if( intensity.red > threshold_intensity )
              break;
       }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.red = histogram->getHistogramSegment()-1 ; high.red != 0 ; high.red--)
       {
          intensity.red += histogram->getValue(ImageHistogram::RedChannel, (int)high.red);
          
          if( intensity.red > threshold_intensity )
             break;
       }
    }

    // Green.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.green = histogram->getHistogramSegment()-1 ; high.green != 0 ; high.green--)
    {
       intensity.green += histogram->getValue(ImageHistogram::GreenChannel, (int)high.green);
       
       if( intensity.green > threshold_intensity )
          break;
    }
    
    if( low.green == high.green )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(low.green = 0 ; low.green < histogram->getHistogramSegment()-1 ; low.green++)
       {
          intensity.green += histogram->getValue(ImageHistogram::GreenChannel, (int)low.green);
          
          if( intensity.green > threshold_intensity )
             break;
       }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.green = histogram->getHistogramSegment()-1 ; high.green != 0 ; high.green--)
       {
          intensity.green += histogram->getValue(ImageHistogram::GreenChannel, (int)high.green);
          
          if( intensity.green > threshold_intensity )
             break;
       }
    }

    // Blue.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.blue = histogram->getHistogramSegment()-1 ; high.blue != 0 ; high.blue--)
    {
       intensity.blue += histogram->getValue(ImageHistogram::BlueChannel, (int)high.blue);
       
       if( intensity.blue > threshold_intensity )
          break;
    }
       
    if( low.blue == high.blue )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
        
       for(low.blue = 0 ; low.blue < histogram->getHistogramSegment()-1 ; low.blue++)
       {
          intensity.blue += histogram->getValue(ImageHistogram::BlueChannel, (int)low.blue);
          
          if( intensity.blue > threshold_intensity )
              break;
       }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.blue = histogram->getHistogramSegment()-1 ; high.blue != 0 ; high.blue--)
       {
          intensity.blue += histogram->getValue(ImageHistogram::BlueChannel, (int)high.blue);
          
          if( intensity.blue > threshold_intensity )
             break;
       }
    }

    // Alpha.
    
    memset(&intensity, 0, sizeof(struct double_packet));
    
    for(high.alpha = histogram->getHistogramSegment()-1 ; high.alpha != 0 ; high.alpha--)
    {
       intensity.alpha += histogram->getValue(ImageHistogram::AlphaChannel, (int)high.alpha);
       
       if( intensity.alpha > threshold_intensity )
          break;
    }
       
    if( low.alpha == high.alpha )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(low.alpha = 0 ; low.alpha < histogram->getHistogramSegment()-1 ; low.alpha++)
       {
          intensity.alpha += histogram->getValue(ImageHistogram::AlphaChannel, (int)low.alpha);
          
          if( intensity.alpha > threshold_intensity )
             break;
       }
       
       memset(&intensity, 0, sizeof(struct double_packet));
       
       for(high.alpha = histogram->getHistogramSegment()-1 ; high.alpha != 0 ; high.alpha--)
       {
          intensity.alpha += histogram->getValue(ImageHistogram::AlphaChannel, (int)high.alpha);
          
          if( intensity.alpha > threshold_intensity )
             break;
       }
    }
    
    delete histogram;

    // Stretch the histogram to create the normalized image mapping.
    
    memset(normalize_map, 0, histogram->getHistogramSegment()*sizeof(struct int_packet));
    
    for(i = 0 ; i <= (long)histogram->getHistogramSegment()-1 ; i++)
    {
       if(i < (long) low.red)
          normalize_map[i].red = 0;
       else if (i > (long) high.red)
          normalize_map[i].red = (256*histogram->getHistogramSegment() -1);
       else if (low.red != high.red)
          normalize_map[i].red = (int)(((256*histogram->getHistogramSegment() -1)*(i-low.red))/(high.red-low.red));

       if(i < (long) low.green)
          normalize_map[i].green = 0;
       else if (i > (long) high.green)
          normalize_map[i].green = (256*histogram->getHistogramSegment() -1);
       else if (low.green != high.green)
          normalize_map[i].green = (int)(((256*histogram->getHistogramSegment() -1)*(i-low.green))/(high.green-low.green));

       if(i < (long) low.blue)
          normalize_map[i].blue = 0;
       else if (i > (long) high.blue)
          normalize_map[i].blue = (256*histogram->getHistogramSegment() -1);
       else if (low.blue != high.blue)
          normalize_map[i].blue = (int)(((256*histogram->getHistogramSegment() -1)*(i-low.blue))/(high.blue-low.blue));

       if(i < (long) low.alpha)
          normalize_map[i].alpha = 0;
       else if (i > (long) high.alpha)
          normalize_map[i].alpha = (256*histogram->getHistogramSegment() -1);
       else if (low.alpha != high.alpha)
          normalize_map[i].alpha = (int)(((256*histogram->getHistogramSegment() -1)*(i-low.alpha))/(high.alpha-low.alpha));
    }

    // Apply result to image.

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar *ptr = data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if(low.red != high.red)
                red = (normalize_map[red].red)/257;
                    
            if(low.green != high.green)
                green = (normalize_map[green].green)/257;
                
            if(low.blue != high.blue)
                blue = (normalize_map[blue].blue)/257;
                
            if(low.alpha != high.alpha)
                alpha = (normalize_map[alpha].alpha)/257;
                                
            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short *ptr = (unsigned short *)data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if(low.red != high.red)
                red = (normalize_map[red].red)/257;
                    
            if(low.green != high.green)
                green = (normalize_map[green].green)/257;
                
            if(low.blue != high.blue)
                blue = (normalize_map[blue].blue)/257;
                
            if(low.alpha != high.alpha)
                alpha = (normalize_map[alpha].alpha)/257;
                            
            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr += 4;
        }
    }
    
    delete [] normalize_map;
}

/** This method scales brightness values across the active 
    image so that the darkest point becomes black, and the 
    brightest point becomes as bright as possible without
    altering its hue. This is often a magic fix for
    images that are dim or washed out.*/
void DImgImageFilters::normalizeImage(uchar *data, int w, int h, bool sixteenBit)
{
    NormalizeParam  param;
    int             x, i;
    unsigned short  range;

    int segments = sixteenBit ? 65536 : 256;

    // Memory allocation.

    param.lut = new unsigned short[segments];

    // Find min. and max. values.

    param.min = segments-1;
    param.max = 0;

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue;
        uchar *ptr = data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            if (red < param.min) param.min = red;
            if (red > param.max) param.max = red;
    
            if (green < param.min) param.min = green;
            if (green > param.max) param.max = green;
    
            if (blue < param.min) param.min = blue;
            if (blue > param.max) param.max = blue;

            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue;
        unsigned short *ptr = (unsigned short *)data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            if (red < param.min) param.min = red;
            if (red > param.max) param.max = red;
    
            if (green < param.min) param.min = green;
            if (green > param.max) param.max = green;
    
            if (blue < param.min) param.min = blue;
            if (blue > param.max) param.max = blue;

            ptr += 4;
        }
    }

    // Calculate LUT.

    range = (unsigned short)(param.max - param.min);

    if (range != 0)
    {
       for (x = (int)param.min ; x <= (int)param.max ; x++)
          param.lut[x] = (unsigned short)((segments-1) * (x - param.min) / range);
    }
    else
       param.lut[(int)param.min] = (unsigned short)param.min;

    // Apply LUT to image.

    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue;
        uchar *ptr = data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            ptr[0] = param.lut[blue];
            ptr[1] = param.lut[green];
            ptr[2] = param.lut[red];

            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue;
        unsigned short *ptr = (unsigned short *)data;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            ptr[0] = param.lut[blue];
            ptr[1] = param.lut[green];
            ptr[2] = param.lut[red];

            ptr += 4;
        }
    }

     delete [] param.lut;
}

/** Performs histogram auto correction of levels.
    This method maximizes the tonal range in the Red,
    Green, and Blue channels. It search the image shadow and highlight 
    limit values and adjust the Red, Green, and Blue channels
    to a full histogram range.*/
void DImgImageFilters::autoLevelsCorrectionImage(uchar *data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       DWarning() << ("DImgImageFilters::autoLevelsCorrectionImage: no image data available!")
                   << endl;
       return;
    }
    uchar* desData;

    // Create the new empty destination image data space.
    if (sixteenBit)
       desData = new uchar[w*h*8];
    else
       desData = new uchar[w*h*4];
       
    // Create an histogram of the current image.     
    ImageHistogram *histogram = new ImageHistogram(data, w, h, sixteenBit);
  
    // Create an empty instance of levels to use.
    ImageLevels *levels = new ImageLevels(sixteenBit);
  
    // Initialize an auto levels correction of the histogram.
    levels->levelsAuto(histogram);

    // Calculate the LUT to apply on the image.
    levels->levelsLutSetup(ImageHistogram::AlphaChannel);
  
    // Apply the lut to the image.
    levels->levelsLutProcess(data, desData, w, h);
  
    if (sixteenBit)
       memcpy (data, desData, w*h*8);
    else
       memcpy (data, desData, w*h*4);

    delete [] desData;
    delete histogram;
    delete levels;
}

/** Performs image colors inversion. This tool is used for negate image
    resulting of a positive film scanned.*/
void DImgImageFilters::invertImage(uchar *data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       DWarning() << ("DImgImageFilters::invertImage: no image data available!")
                   << endl;
       return;
    }

    if (!sixteenBit)        // 8 bits image.
    {
        uchar *ptr = data;
        
        for (int i = 0 ; i < w*h ; i++)
        {
            ptr[0] = 255 - ptr[0];
            ptr[1] = 255 - ptr[1];
            ptr[2] = 255 - ptr[2];
            ptr[3] = 255 - ptr[3];
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)data;
        
        for (int i = 0 ; i < w*h ; i++)
        {
            ptr[0] = 65535 - ptr[0];
            ptr[1] = 65535 - ptr[1];
            ptr[2] = 65535 - ptr[2];
            ptr[3] = 65535 - ptr[3];
            ptr += 4;
        }
    }
}

/** Mix RGB channel color from image*/
void DImgImageFilters::channelMixerImage(uchar *data, int Width, int Height, bool sixteenBit,
                                     bool bPreserveLum, bool bMonochrome,
                                     float rrGain, float rgGain, float rbGain,
                                     float grGain, float ggGain, float gbGain,
                                     float brGain, float bgGain, float bbGain, 
                                     bool overIndicator)
{
    if (!data || !Width || !Height)
    {
       DWarning() << ("DImgImageFilters::channelMixerImage: no image data available!")
                   << endl;
       return;
    }
        
    register int i;

    double rnorm = CalculateNorm (rrGain, rgGain, rbGain, bPreserveLum);
    double gnorm = CalculateNorm (grGain, ggGain, gbGain, bPreserveLum);
    double bnorm = CalculateNorm (brGain, bgGain, bbGain, bPreserveLum);

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  nGray, red, green, blue;
        uchar *ptr = data;

        for (i = 0 ; i < Width*Height ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            if (bMonochrome)
            {
                nGray = MixPixel (rrGain, rgGain, rbGain, (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                  sixteenBit, rnorm, overIndicator);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = (uchar)MixPixel (brGain, bgGain, bbGain, (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, bnorm, overIndicator);
                ptr[1] = (uchar)MixPixel (grGain, ggGain, gbGain, (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, gnorm, overIndicator);
                ptr[2] = (uchar)MixPixel (rrGain, rgGain, rbGain, (unsigned short)red, (unsigned short)green, (unsigned short)blue,
                                          sixteenBit, rnorm, overIndicator);
            }
                                
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short  nGray, red, green, blue;
        unsigned short *ptr = (unsigned short *)data;
        
        for (i = 0 ; i < Width*Height ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
        
            if (bMonochrome)
            {
                nGray = MixPixel (rrGain, rgGain, rbGain, red, green, blue, sixteenBit, rnorm, overIndicator);
                ptr[0] = ptr[1] = ptr[2] = nGray;
            }
            else
            {
                ptr[0] = MixPixel (brGain, bgGain, bbGain, red, green, blue, sixteenBit, bnorm, overIndicator);
                ptr[1] = MixPixel (grGain, ggGain, gbGain, red, green, blue, sixteenBit, gnorm, overIndicator);
                ptr[2] = MixPixel (rrGain, rgGain, rbGain, red, green, blue, sixteenBit, rnorm, overIndicator);
            }
                            
            ptr += 4;
        }
    }
}

/** Change color tonality of an image to appling a RGB color mask.*/
void DImgImageFilters::changeTonality(uchar *data, int width, int height, bool sixteenBit,
                                      int redMask, int greenMask, int blueMask)
{
    if (!data || !width || !height)
    {
       DWarning() << ("DImgImageFilters::changeTonality: no image data available!")
                   << endl;
       return;
    }

    int hue, sat, lig;
    
    DColor mask(redMask, greenMask, blueMask, 0, sixteenBit);
    mask.getHSL(&hue, &sat, &lig);

    if (!sixteenBit)        // 8 bits image.
    {
        uchar *ptr = data;
        
        for (int i = 0 ; i < width*height ; i++)
        {
            // Convert to grayscale using tonal mask
                
            lig = ROUND (0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);
            
            mask.setRGB(hue, sat, lig, sixteenBit);

            ptr[0] = (uchar)mask.blue();
            ptr[1] = (uchar)mask.green();
            ptr[2] = (uchar)mask.red();
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)data;
        
        for (int i = 0 ; i < width*height ; i++)
        {
            // Convert to grayscale using tonal mask
                
            lig = ROUND (0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);
            
            mask.setRGB(hue, sat, lig, sixteenBit);
                                
            ptr[0] = (unsigned short)mask.blue();
            ptr[1] = (unsigned short)mask.green();
            ptr[2] = (unsigned short)mask.red();
            ptr += 4;
        }
    }
}

/** Function to apply the GaussianBlur on an image. This method do not use a
    dedicaced thread.*/
void DImgImageFilters::gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
       DWarning() << ("DImgImageFilters::gaussianBlurImage: no image data available!")
                   << endl;
       return;
    }

    if (radius > 100) radius = 100;
    if (radius <= 0) return;

    DImg orgImage(width, height, sixteenBit, true, data);
    DImgGaussianBlur *filter = new DImgGaussianBlur(&orgImage, 0L, radius);
    DImg imDest = filter->getTargetImage();
    memcpy( data, imDest.bits(), imDest.numBytes() );
    delete filter;
}

/** Function to apply the sharpen filter on an image. This method do not use a
    dedicaced thread.*/
void DImgImageFilters::sharpenImage(uchar *data, int width, int height, bool sixteenBit, int radius)
{
    if (!data || !width || !height)
    {
       DWarning() << ("DImgImageFilters::sharpenImage: no image data available!")
                   << endl;
       return;
    }

    if (radius > 100) radius = 100;
    if (radius <= 0) return;

    DImg orgImage(width, height, sixteenBit, true, data);
    DImgSharpen *filter = new DImgSharpen(&orgImage, 0L, radius);
    DImg imDest = filter->getTargetImage();
    memcpy( data, imDest.bits(), imDest.numBytes() );
    delete filter;
}

/** Function to perform pixel antialiasing with 8 bits/color/pixel images. This method is used to smooth target
    image in transformation  method like free rotation or shear tool. */
void DImgImageFilters::pixelAntiAliasing(uchar *data, int Width, int Height, double X, double Y,
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

            lfTotalB += ((double)data[j] * lfWeight);
            j++;
            lfTotalG += ((double)data[j] * lfWeight);
            j++;
            lfTotalR += ((double)data[j] * lfWeight);
            j++;
            lfTotalA += ((double)data[j] * lfWeight);
            j++;
        }
    }
         
    *B = CLAMP0255((int)lfTotalB);
    *G = CLAMP0255((int)lfTotalG);
    *R = CLAMP0255((int)lfTotalR);
    *A = CLAMP0255((int)lfTotalA);
}

/** Function to perform pixel antialiasing with 16 bits/color/pixel images. This method is used to smooth target
    image in transformation  method like free rotation or shear tool. */
void DImgImageFilters::pixelAntiAliasing16(unsigned short *data, int Width, int Height, double X, double Y,
                                           unsigned short *A, unsigned short *R, unsigned short *G,
                                           unsigned short *B)
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

            lfTotalB += ((double)data[j] * lfWeight);
            j++;
            lfTotalG += ((double)data[j] * lfWeight);
            j++;
            lfTotalR += ((double)data[j] * lfWeight);
            j++;
            lfTotalA += ((double)data[j] * lfWeight);
            j++;
        }
    }
         
    *B = CLAMP065535((int)lfTotalB);
    *G = CLAMP065535((int)lfTotalG);
    *R = CLAMP065535((int)lfTotalR);
    *A = CLAMP065535((int)lfTotalA);
}

}  // NameSpace Digikam
