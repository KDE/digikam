/* ============================================================
 * File  : imageeffect_colorsenhance.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-20
 * Description : image contrast enhancement techniques. 
 * 
 * Copyright 2004 by Gilles Caulier
 * Normalize an Equalize algorithms fixed and adapted for to work with Raw 
 * data image (ARGB).
 * 
 * Algorithms are taken from KImageEffect API of KDE project.
 * Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
 *           (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
 *           (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>
 *           (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *           (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
 *           (C) 2004 Zack Rusin <zack@kde.org>
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

// For to test KImageEffect API...

#include <qimage.h>
#include <kimageeffect.h>       

// KDE includes.

#include <kdebug.h>

// Digikam includes.
 
#include <imagehistogram.h>
#include <imagelevels.h>
#include <imageiface.h>

// Local includes.

#include "imageeffect_colorsenhance.h"

/////////////////////////////////////////////////////////////////////////////////
// Performs an histogram equalisation of the image.

void ImageEffect_ColorsEnhance::equalizeImage()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();

    if (!data || !w || !h)
       {
       kdWarning() << ("ImageEffect_ColorsEnhance::equalizeImage: no image data available!") << endl;
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
        
       kdWarning() << ("ImageEffect_ColorsEnhance::equalizeImage: Unable to allocate memory!") << endl;
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
    
    iface.putOriginalData(data);
    delete [] data;       
}

/////////////////////////////////////////////////////////////////////////////////
// Performs histogram normalization of the image. The algorithm normalizes 
// the pixel values from an image for to span the full range 
// of color values. This is a contrast enhancement technique. 

void ImageEffect_ColorsEnhance::normalizeImage()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();

    if (!data || !w || !h)
       {
       kdWarning() << ("ImageEffect_ColorsEnhance::normalizeImage: no image data available!") << endl;
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
        
       kdWarning() << ("ImageEffect_ColorsEnhance::normalizeImage: Unable to allocate memory!") << endl;
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

    iface.putOriginalData(data);
    delete [] data;
}

//////////////////////////////////////////////////////////////////////////////
// Performs histogram auto correction of levels.

void ImageEffect_ColorsEnhance::autoLevelsCorrectionImage()
{
    Digikam::ImageIface iface(0, 0);

    uint* orgData = iface.getOriginalData();
    int   w       = iface.originalWidth();
    int   h       = iface.originalHeight();

    if (!orgData || !w || !h)
       {
       kdWarning() << ("ImageEffect_ColorsEnhance::autoLevelsCorrectionImage: no image data available!")
                   << endl;
       return;
       }
  
    // Create the new empty destination image data space.
    uint* desData = new uint[w*h];
       
    // Create an histogram of the current image.     
    Digikam::ImageHistogram *histogram = new Digikam::ImageHistogram(orgData, w, h);
  
    // Create an empty instance of levels to use.
    Digikam::ImageLevels *levels = new Digikam::ImageLevels();      
  
    // Initialize an auto levels correction of the histogram.
    levels->levelsAuto(histogram);

    // Calculate the LUT to apply on the image.
    levels->levelsLutSetup(Digikam::ImageHistogram::AlphaChannel);
  
    // Apply the lut to the image.
    levels->levelLutProcess(orgData, desData, w, h);
  
    iface.putOriginalData(desData);
  
    delete [] orgData;
    delete [] desData;
    delete histogram;
    delete levels;
}

//////////////////////////////////////////////////////////////////////////////
// For to test KImageEffect API

void ImageEffect_ColorsEnhance::testKImageEffect()
{
    /*
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth();
    int   h    = iface.originalHeight();

    if (!data || !w || !h)
        return;

    QImage image;
    image.create( w, h, 32 );
    image.setAlphaBuffer(true) ;
    memcpy(image.bits(), data, image.numBytes());

    // This is the KImageEffect call...
    KImageEffect::emboss(image, 0.3, 0.3);    
    
    memcpy(data, image.bits(), image.numBytes());
    iface.putOriginalData(data);
    delete [] data;
    */
}

