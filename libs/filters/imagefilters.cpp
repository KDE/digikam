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
#include <cstring>
#include <cstdlib>

// KDE includes.

#include <kdebug.h>

// Digikam includes.
 
#include <imagehistogram.h>
#include <imagelevels.h>

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
// Performs image antialiasing. This is a blur with less pixels.

void ImageFilters::antiAliasImage(uint *data, int Width, int Height, int /*Sensibility*/)
{
    if (!data || !Width || !Height)
       {
       kdWarning() << ("ImageFilters::antiAliasImage: no image data available!")
                   << endl;
       return;
       }
       
    int LineWidth = Width * 4;
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar* Bits = (uchar*)data;
        
    int i = 0, j = 0, k = 0;
    
    // Simple Bluring method.
    
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
                                                                                                              
/*  
    // Advanced bluring method using sensitivity value (not yet working correctly).
    
    uchar Temp[3][9];
    
    if (Sensibility > 255)        // MAX = 255
        Sensibility = 255;        
    if (Sensibility < 0)          // MIN = 0
        Sensibility = 0;
                                                           
    int y, i = 0, j = 0, k = 0, GrayCmp, Gray, add;
    
    for (int h = 1; h < Height - 1; h++)
        {
        for (int w = 1; w < Width - 1; w++)
            {
            i = h * LineWidth + 4 * w;
            j = (h + 1) * LineWidth + 4 * w;
            k = (h - 1) * LineWidth + 4 * w;
            Gray = (Bits[i+2] + Bits[i+1] + Bits[i]) / 3;

            for (y = 0; y < 3; y++)
                {
                add = y * 4;
                GrayCmp = (Bits[j+add-2] + Bits[j+add-3] + Bits[j+add-4]) / 3;
                
                if (! ((GrayCmp > Gray + Sensibility) || (GrayCmp < Gray - Sensibility)))
                    {
                    Temp[0][y] = Bits[ i ];
                    Temp[1][y] = Bits[i+1];
                    Temp[2][y] = Bits[i+2];
                    }
                else
                    {
                    Temp[0][y] = Bits[j+add-4];
                    Temp[1][y] = Bits[j+add-3];
                    Temp[2][y] = Bits[j+add-2];
                    }
                }

            for (y = 0; y < 3; y++)
                {
                add = y * 4;
                GrayCmp = (Bits[i+add-2] + Bits[i+add-3] + Bits[i+add-4]) / 3;
                    
                if (! ((GrayCmp > Gray + Sensibility) || (GrayCmp < Gray - Sensibility)))
                    {
                    Temp[0][y+3] = Bits[ i ];
                    Temp[1][y+3] = Bits[i+1];
                    Temp[2][y+3] = Bits[i+2];
                    }
                else
                    {
                    Temp[0][y+3] = Bits[i+add-4];
                    Temp[1][y+3] = Bits[i+add-3];
                    Temp[2][y+3] = Bits[i+add-2];
                    }
                }

             for (y = 0; y < 3; y++)
                {
                add = y * 4;
                GrayCmp = (Bits[k+add-2] + Bits[k+add-3] + Bits[k+add-4]) / 3;
                    
                if (! ((GrayCmp > Gray + Sensibility) || (GrayCmp < Gray - Sensibility)))
                    {
                    Temp[0][y+6] = Bits[ i ];
                    Temp[1][y+6] = Bits[i+1];
                    Temp[2][y+6] = Bits[i+2];
                    }
                else
                    {
                    Temp[0][y+6] = Bits[k+add-4];
                    Temp[1][y+6] = Bits[k+add-3];
                    Temp[2][y+6] = Bits[k+add-2];
                    }
                }

            Bits[i+2] = (Temp[2][0] + Temp[2][1] + Temp[2][2] +
                         Temp[2][3] + Temp[2][4] + Temp[2][5] +
                         Temp[2][6] + Temp[2][7] + Temp[2][8]) / 9;
            Bits[i+1] = (Temp[1][0] + Temp[1][1] + Temp[1][2] +
                         Temp[1][3] + Temp[1][4] + Temp[1][5] +
                         Temp[1][6] + Temp[1][7] + Temp[1][8]) / 9;
            Bits[ i ] = (Temp[0][0] + Temp[0][1] + Temp[0][2] +
                         Temp[0][3] + Temp[0][4] + Temp[0][5] +
                         Temp[0][6] + Temp[0][7] + Temp[0][8]) / 9;
            }
        }*/
}

}  // NameSpace Digikam
