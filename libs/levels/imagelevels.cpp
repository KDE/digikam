/* ============================================================
 * File  : imagelevels.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-29
 * Description : image levels manipulation methods.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Some code parts are inspired from gimp 2.0
 * app/base/levels.c and gimplut.c source files.
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
 
#include <cstdio>
#include <cmath>
#include <cstring>

// KDE includes.

#include <kdebug.h>

// Local includes.
 
#include <imagehistogram.h>
#include "imagelevels.h"

namespace Digikam
{

ImageLevels::ImageLevels()
{ 
    m_lut    = new _Lut;
    m_levels = new _Levels;
        
    m_lut->luts      = NULL;
    m_lut->nchannels = 0;

    for (int channel = Digikam::ImageHistogram::ValueChannel ;
         channel <= Digikam::ImageHistogram::AlphaChannel ;
         ++channel)
       {
       levelsChannelReset(channel);
       }

}

ImageLevels::~ImageLevels()
{ 
    if (m_lut)
       delete m_lut;
    
    if (m_levels)
       delete m_levels;
}

void ImageLevels::levelsChannelReset(int channel)
{
    if (!m_levels) return;

    m_levels->gamma[channel]       = 1.0;
    m_levels->low_input[channel]   = 0;
    m_levels->high_input[channel]  = 255;
    m_levels->low_output[channel]  = 0;
    m_levels->high_output[channel] = 255;
}


void ImageLevels::levelsAuto(Digikam::ImageHistogram *hist)
{
    if (!m_levels || !hist) return;

    levelsChannelReset(Digikam::ImageHistogram::ValueChannel);

    for (int channel = Digikam::ImageHistogram::RedChannel ;
         channel <= Digikam::ImageHistogram::BlueChannel ;
         ++channel)
       {
       levelsChannelAuto(hist, channel);
       }
}

void ImageLevels::levelsChannelAuto(Digikam::ImageHistogram *hist, int channel)
{
    int    i;
    double count, new_count, percentage, next_percentage;

    if (!m_levels || !hist) return;

    m_levels->gamma[channel]       = 1.0;
    m_levels->low_output[channel]  = 0;
    m_levels->high_output[channel] = 255;

    count = hist->getCount(channel, 0, 255);

    if (count == 0.0)
       {
       m_levels->low_input[channel]  = 0;
       m_levels->high_input[channel] = 0;
       }
    else
       {
       //  Set the low input  
       
       new_count = 0.0;

       for (i = 0 ; i < 255 ; ++i)
          {
          new_count += hist->getValue(channel, i);
          percentage = new_count / count;
          next_percentage = (new_count + hist->getValue(channel, i + 1)) / count;
      
          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
             {
             m_levels->low_input[channel] = i + 1;
             break;
             }
          }
      
       //  Set the high input  
       
       new_count = 0.0;
      
       for (i = 255 ; i > 0 ; --i)
          {
          new_count += hist->getValue(channel, i);
          percentage = new_count / count;
          next_percentage = (new_count + hist->getValue(channel, i - 1)) / count;
          
          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
             {
             m_levels->high_input[channel] = i - 1;
             break;
             }
          }
       }
}

int ImageLevels::levelsInputFromColor(int channel, uchar *color)
{
    switch (channel)
       {
       case Digikam::ImageHistogram::ValueChannel:
          return QMAX (QMAX (color[Digikam::ImageLevels::RedPixel],
                             color[Digikam::ImageLevels::GreenPixel]),
                       color[Digikam::ImageLevels::BluePixel]);
       
       case Digikam::ImageHistogram::RedChannel:
          return color[Digikam::ImageLevels::RedPixel];
       
       case Digikam::ImageHistogram::GreenChannel:
          return color[Digikam::ImageLevels::GreenPixel];
    
       case Digikam::ImageHistogram::BlueChannel:
          return color[Digikam::ImageLevels::BluePixel];
    
       case Digikam::ImageHistogram::AlphaChannel:
          return color[Digikam::ImageLevels::AlphaPixel];
       }

    return 0;  // just to please the compiler.
}

void ImageLevels::levelsAdjustByColors(int channel, uchar *black, uchar *gray, uchar *white)
{
    if (!m_levels) return;

    if (black)
       m_levels->low_input[channel] = levelsInputFromColor(channel, black);

    if (white)
       m_levels->high_input[channel] = levelsInputFromColor(channel, white);

    if (gray)
       {
       int    input;
       int    range;
       double inten;
       double out_light;
       uchar  lightness;

       // Calculate lightness value.
       
       lightness = GIMP_RGB_INTENSITY (gray[0], gray[1], gray[2]);  // Renchi, why used 0, 1, 2 index.

       input = levelsInputFromColor(channel, gray);

       range = m_levels->high_input[channel] - m_levels->low_input[channel];
      
       if (range <= 0)
          return;

       input -= m_levels->low_input[channel];
       
       if (input < 0)
          return;

       // Normalize input and lightness.
       
       inten = (double) input / (double) range;
       out_light = (double) lightness/ (double) range;

       if (out_light <= 0)
          return;

       // Map selected color to corresponding lightness.
       
       m_levels->gamma[channel] = log (inten) / log (out_light); 
       }
} 

void ImageLevels::levelsCalculateTransfers()
{
    double inten;
    int    i, j;

    if (!m_levels) return;

    // Recalculate the levels arrays.
    
    for (j = 0 ; j < 5 ; ++j)
      {
      for (i = 0; i < 256; ++i)
          {
          //  determine input intensity.
          
          if (m_levels->high_input[j] != m_levels->low_input[j])
             {
             inten = ((double) (i - m_levels->low_input[j]) /
                      (double) (m_levels->high_input[j] - m_levels->low_input[j]));
             }
          else
             {
             inten = (double) (i - m_levels->low_input[j]);
             }

          inten = CLAMP (inten, 0.0, 1.0);

          if (m_levels->gamma[j] != 0.0)
             inten = pow (inten, (1.0 / m_levels->gamma[j]));

          m_levels->input[j][i] = (uchar) (inten * 255.0 + 0.5);
          }
      }
}

float ImageLevels::levelsLutFunc(int n_channels, int channel, float value)
{
    double inten;
    int    j;

    if (!m_levels) return 0.0;
    
    if (n_channels == 1)
       j = 0;
    else
       j = channel + 1;

    inten = value;

    // For color  images this runs through the loop with j = channel +1
    // the first time and j = 0 the second time.
    //
    // For bw images this runs through the loop with j = 0 the first and
    // only time.
    //
  
    for ( ; j >= 0 ; j -= (channel + 1) )
      {
      // Don't apply the overall curve to the alpha channel.
      
      if (j == 0 && (n_channels == 2 || n_channels == 4)
          && channel == n_channels -1)
         return inten;

      //  Determine input intensity.
      
      if (m_levels->high_input[j] != m_levels->low_input[j])
         inten = ((double) (255.0 * inten - m_levels->low_input[j]) /
                 (double) (m_levels->high_input[j] - m_levels->low_input[j]));
      else
         inten = (double) (255.0 * inten - m_levels->low_input[j]);

      if (m_levels->gamma[j] != 0.0)
         {
         if (inten >= 0.0)
            inten =  pow ( inten, (1.0 / m_levels->gamma[j]));
         else
            inten = -pow (-inten, (1.0 / m_levels->gamma[j]));
         }

      //  determine the output intensity.
      
      if (m_levels->high_output[j] >= m_levels->low_output[j])
         inten = (double) (inten * (m_levels->high_output[j] -
                 m_levels->low_output[j]) + m_levels->low_output[j]);
                 
      else if (m_levels->high_output[j] < m_levels->low_output[j])
         inten = (double) (m_levels->low_output[j] - inten *
                 (m_levels->low_output[j] - m_levels->high_output[j]));

      inten /= 255.0;
      }

    return inten;
}

// This method is inspired of Gimp2.0 
// app/base/gimplut.c::gimp_lut_setup 
void ImageLevels::levelsLutSetup(int nchannels)
{
    int    i; 
    uint   v;
    double val;

    if (m_lut->luts)
       {
       for (i = 0 ; i < m_lut->nchannels ; ++i)
           delete [] m_lut->luts[i];

       delete [] m_lut->luts;
       }

    m_lut->nchannels = nchannels;
    m_lut->luts      = new (uchar *)[m_lut->nchannels];
    
    for (i = 0 ; i < m_lut->nchannels ; ++i)
       {
       m_lut->luts[i]      = new (uchar)[256];

       for (v = 0 ; v < 256 ; ++v)
          {
          // to add gamma correction use func(v ^ g) ^ 1/g instead. 
          val = 255.0 * levelsLutFunc( m_lut->nchannels, i, v/255.0) + 0.5;
          
          m_lut->luts[i][v] = CLAMP (val, 0, 255);
          }
       }
}

// This method is inspired of Gimp2.0 
// app/base/gimplut.c::gimp_lut_process
void ImageLevels::levelLutProcess(uint *srcPR, uint *destPR, int w, int h)
{
    uint   height, width, src_r_i, dest_r_i;
    uchar *src, *dest;
    uchar *lut0 = NULL, *lut1 = NULL, *lut2 = NULL, *lut3 = NULL;

    if (m_lut->nchannels > 0)
       lut0 = m_lut->luts[0];
    if (m_lut->nchannels > 1)
       lut1 = m_lut->luts[1];
    if (m_lut->nchannels > 2)
       lut2 = m_lut->luts[2];
    if (m_lut->nchannels > 3)
       lut3 = m_lut->luts[3];

    height   = h;
    src      = (uchar*)srcPR;
    dest     = (uchar*)destPR;
    width    = w;
    src_r_i  = 0;
    dest_r_i = 0;

    if (src_r_i == 0 && dest_r_i == 0)
       {
       width *= h;
       h = 1;
       }

  while (h--)
    {
    switch (m_lut->nchannels)
    {
    case 1:
      while (width--)
        {
          *dest = lut0[*src];
          src++;
          dest++;
        }
      break;
    case 2:
      while (width--)
        {
          dest[0] = lut0[src[0]];
          dest[1] = lut1[src[1]];
          src  += 2;
          dest += 2;
        }
      break;
    case 3:
      while (width--)
        {
          dest[0] = lut0[src[0]];
          dest[1] = lut1[src[1]];
          dest[2] = lut2[src[2]];
          src  += 3;
          dest += 3;
        }
      break;
    case 4:
      while (width--)
        {
          dest[0] = lut0[src[0]];
          dest[1] = lut1[src[1]];
          dest[2] = lut2[src[2]];
          dest[3] = lut3[src[3]];
          src  += 4;
          dest += 4;
        }
      break;
    default:
      kdWarning() << "ImageLevels::levelLutProcess: Error: nchannels = " << m_lut->nchannels << endl;
    }

    width = w;
    src  += src_r_i;
    dest += dest_r_i;
    }
}

}  // NameSpace Digikam
