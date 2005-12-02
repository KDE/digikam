/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-29
 * Description : image levels manipulation methods.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Some code parts are inspired from gimp 2.0
 * app/base/levels.c, gimplut.c, and app/base/gimpleveltool.c 
 * source files.
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

// Qt includes.

#include <qfile.h>

// C++ includes. 
 
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// KDE includes.

#include <kdebug.h>

// Local includes.
 
#include "imagehistogram.h"
#include "imagelevels.h"

namespace Digikam
{

ImageLevels::ImageLevels(bool sixteenBit)
{ 
    m_lut        = new _Lut;
    m_levels     = new _Levels;
    m_sixteenBit = sixteenBit;
    
    memset(m_levels, 0, sizeof(struct _Levels));    
    m_lut->luts      = NULL;
    m_lut->nchannels = 0;

    for (int channel = 0 ; channel < 5 ; channel++)
       levelsChannelReset(channel);
}

ImageLevels::~ImageLevels()
{ 
    if (m_lut)
    {
       if (m_lut->luts)
       {
          for (int i = 0 ; i < m_lut->nchannels ; i++)
              delete [] m_lut->luts[i];

          delete [] m_lut->luts;
       }
       
       delete m_lut;
    }
    
    if (m_levels)
       delete m_levels;
}

void ImageLevels::levelsChannelReset(int channel)
{
    if (!m_levels) return;

    m_levels->gamma[channel]       = 1.0;
    m_levels->low_input[channel]   = 0;
    m_levels->high_input[channel]  = m_sixteenBit ? 65535 : 255;
    m_levels->low_output[channel]  = 0;
    m_levels->high_output[channel] = m_sixteenBit ? 65535 : 255;
}


void ImageLevels::levelsAuto(Digikam::ImageHistogram *hist)
{
    if (!m_levels || !hist) return;

    levelsChannelReset(Digikam::ImageHistogram::ValueChannel);

    for (int channel = Digikam::ImageHistogram::RedChannel ;
         channel <= Digikam::ImageHistogram::BlueChannel ;
         channel++)
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
    m_levels->high_output[channel] = m_sixteenBit ? 65535 : 255;

    count = hist->getCount(channel, 0, m_sixteenBit ? 65535 : 255);

    if (count == 0.0)
    {
       m_levels->low_input[channel]  = 0;
       m_levels->high_input[channel] = 0;
    }
    else
    {
       //  Set the low input  
       
       new_count = 0.0;

       for (i = 0 ; i < (m_sixteenBit ? 65535 : 255) ; i++)
       {
          new_count       += hist->getValue(channel, i);
          percentage      = new_count / count;
          next_percentage = (new_count + hist->getValue(channel, i + 1)) / count;
      
          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
          {
             m_levels->low_input[channel] = i + 1;
             break;
          }
       }
      
       //  Set the high input  
       
       new_count = 0.0;
      
       for (i = (m_sixteenBit ? 65535 : 255) ; i > 0 ; i--)
       {
          new_count       += hist->getValue(channel, i);
          percentage      = new_count / count;
          next_percentage = (new_count + hist->getValue(channel, i - 1)) / count;
          
          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
          {
             m_levels->high_input[channel] = i - 1;
             break;
          }
       }
    }
}

int ImageLevels::levelsInputFromColor(int channel, DColor color)
{
    switch (channel)
    {
       case Digikam::ImageHistogram::ValueChannel:
          return QMAX (QMAX (color.red(), color.green()), color.blue());
       
       case Digikam::ImageHistogram::RedChannel:
          return color.red();
       
       case Digikam::ImageHistogram::GreenChannel:
          return color.green();
    
       case Digikam::ImageHistogram::BlueChannel:
          return color.blue();
    }

    return 0;  // just to please the compiler.
}

void ImageLevels::levelsBlackToneAdjustByColors(int channel, DColor color)
{
    if (!m_levels) return;

    m_levels->low_input[channel] = levelsInputFromColor(channel, color);
}

void ImageLevels::levelsWhiteToneAdjustByColors(int channel, DColor color)
{
    if (!m_levels) return;

    m_levels->high_input[channel] = levelsInputFromColor(channel, color);
}

void ImageLevels::levelsGrayToneAdjustByColors(int channel, DColor color)
{
    if (!m_levels) return;

    int            input;
    int            range;
    double         inten;
    double         out_light;
    unsigned short lightness;

    // Calculate lightness value.
       
    lightness = (unsigned short)LEVELS_RGB_INTENSITY (color.red(), color.green(), color.blue());

    input     = levelsInputFromColor(channel, color);

    range     = m_levels->high_input[channel] - m_levels->low_input[channel];
      
    if (range <= 0)
       return;

    input -= m_levels->low_input[channel];
       
    if (input < 0)
       return;

    // Normalize input and lightness.
       
    inten     = (double) input / (double) range;
    out_light = (double) lightness/ (double) range;

    if (out_light <= 0)
       return;

    // Map selected color to corresponding lightness.
       
    m_levels->gamma[channel] = log (inten) / log (out_light); 
} 

void ImageLevels::levelsCalculateTransfers()
{
    double inten;
    int    i, j;

    if (!m_levels) return;

    // Recalculate the levels arrays.
    
    for (j = 0 ; j < 5 ; j++)
    {
      for (i = 0; i <= (m_sixteenBit ? 65535 : 255); i++)
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
  
    for ( ; j >= 0 ; j -= (channel + 1) )
    {
       // Don't apply the overall curve to the alpha channel.
      
       if (j == 0 && (n_channels == 2 || n_channels == 4)
          && channel == n_channels -1)
          return inten;

       //  Determine input intensity.
       
       if (m_levels->high_input[j] != m_levels->low_input[j])
          inten = ((double) ((float)(m_sixteenBit ? 65535 : 255) * inten - m_levels->low_input[j]) /
                  (double) (m_levels->high_input[j] - m_levels->low_input[j]));
       else
          inten = (double) ((float)(m_sixteenBit ? 65535 : 255) * inten - m_levels->low_input[j]);

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

       inten /= (float)(m_sixteenBit ? 65535 : 255);
    }

    return inten;
}

void ImageLevels::levelsLutSetup(int nchannels, bool overIndicator)
{
    int    i; 
    uint   v;
    double val;

    if (m_lut->luts)
    {
       for (i = 0 ; i < m_lut->nchannels ; i++)
           delete [] m_lut->luts[i];

       delete [] m_lut->luts;
    }

    m_lut->nchannels = nchannels;
    m_lut->luts      = new unsigned short*[m_lut->nchannels];
    
    for (i = 0 ; i < m_lut->nchannels ; i++)
    {
       m_lut->luts[i] = new unsigned short[(m_sixteenBit ? 65535 : 255) + 1];

       for (v = 0 ; v <= (m_sixteenBit ? 65535 : 255) ; v++)
       {
          // to add gamma correction use func(v ^ g) ^ 1/g instead.

          val = (float)(m_sixteenBit ? 65535 : 255) *
                levelsLutFunc( m_lut->nchannels, i, v/(float)(m_sixteenBit ? 65535 : 255)) + 0.5;
          if (overIndicator && val > (m_sixteenBit ? 65535 : 255)) val = 0;
          m_lut->luts[i][v] = (unsigned short)CLAMP (val, 0, (m_sixteenBit ? 65535 : 255));
       }
    }
}

void ImageLevels::levelsLutProcess(uchar *srcPR, uchar *destPR, int w, int h)
{
    unsigned short *lut0 = NULL, *lut1 = NULL, *lut2 = NULL, *lut3 = NULL;

    int   i;
    
    if (m_lut->nchannels > 0)
       lut0 = m_lut->luts[0];
    if (m_lut->nchannels > 1)
       lut1 = m_lut->luts[1];
    if (m_lut->nchannels > 2)
       lut2 = m_lut->luts[2];
    if (m_lut->nchannels > 3)
       lut3 = m_lut->luts[3];

    if (!m_sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar *ptr = srcPR;
        uchar *dst = destPR;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if ( m_lut->nchannels > 0 )
               red = lut0[red];
            
            if ( m_lut->nchannels > 1 )
               green = lut1[green];
            
            if ( m_lut->nchannels > 2 )
               blue = lut2[blue];
        
            if ( m_lut->nchannels > 3 )
               alpha = lut3[alpha];
                                
            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short *ptr = (unsigned short *)srcPR;
        unsigned short *dst = (unsigned short *)destPR;

        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if ( m_lut->nchannels > 0 )
               red = lut0[red];
            
            if ( m_lut->nchannels > 1 )
               green = lut1[green];
            
            if ( m_lut->nchannels > 2 )
               blue = lut2[blue];
        
            if ( m_lut->nchannels > 3 )
               alpha = lut3[alpha];
                                
            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
}

void ImageLevels::setLevelGammaValue(int Channel, double val)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       m_levels->gamma[Channel] = val;
}

void ImageLevels::setLevelLowInputValue(int Channel, int val)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       m_levels->low_input[Channel] = val;
}

void ImageLevels::setLevelHighInputValue(int Channel, int val)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       m_levels->high_input[Channel] = val;
}

void ImageLevels::setLevelLowOutputValue(int Channel, int val)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       m_levels->low_output[Channel] = val;
}

void ImageLevels::setLevelHighOutputValue(int Channel, int val)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       m_levels->high_output[Channel] = val;
}

double ImageLevels::getLevelGammaValue(int Channel)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       return (m_levels->gamma[Channel]);
    
    return 0.0;
}

int ImageLevels::getLevelLowInputValue(int Channel)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       return (m_levels->low_input[Channel]);
    
    return 0;
}

int ImageLevels::getLevelHighInputValue(int Channel)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       return (m_levels->high_input[Channel]);
    
    return 0;
}

int ImageLevels::getLevelLowOutputValue(int Channel)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       return (m_levels->low_output[Channel]);
    
    return 0;
}

int ImageLevels::getLevelHighOutputValue(int Channel)
{
    if ( m_levels && Channel>=0 && Channel<5 )
       return (m_levels->high_output[Channel]);
    
    return 0;
}

bool ImageLevels::loadLevelsFromGimpLevelsFile(KURL fileUrl)
{
    // TODO : support KURL !
    
    FILE          *file;
    int            low_input[5];
    int            high_input[5];
    int            low_output[5];
    int            high_output[5];
    double         gamma[5];
    int            i, fields;
    char           buf[50];
    char          *nptr;

    file = fopen(QFile::encodeName(fileUrl.path()), "r");
    
    if (!file)
       return false;
    
    if (! fgets (buf, sizeof (buf), file))
       {
       fclose(file);
       return false;
       }

    if (strcmp (buf, "# GIMP Levels File\n") != 0)
       {
       fclose(file);
       return false;
       }

    for (i = 0 ; i < 5 ; i++)
      {
      fields = fscanf (file, "%d %d %d %d ",
                       &low_input[i],
                       &high_input[i],
                       &low_output[i],
                       &high_output[i]);

      if (fields != 4)
        {
        fclose(file);
        return false;
        }

      if (! fgets (buf, 50, file))
        {
        fclose(file);
        return false;
        }

      gamma[i] = strtod (buf, &nptr);

      if (buf == nptr || errno == ERANGE)
        {
        fclose(file);
        return false;
        }
      }

    for (i = 0 ; i < 5 ; i++)
      {
      setLevelGammaValue(i, gamma[i]);
      setLevelLowInputValue(i, low_input[i]);
      setLevelHighInputValue(i, high_input[i]);
      setLevelLowOutputValue(i, low_output[i]);
      setLevelHighOutputValue(i, high_output[i]);
      }

    fclose(file);
    return true;
}

bool ImageLevels::saveLevelsToGimpLevelsFile(KURL fileUrl)
{
    // TODO : support KURL !
  
    FILE          *file;
    int            i;

    file = fopen(QFile::encodeName(fileUrl.path()), "w");
    
    if (!file)
       return false;

    fprintf (file, "# GIMP Levels File\n");

    for (i = 0 ; i < 5 ; i++)
      {
      char buf[256];
      sprintf (buf, "%f", getLevelGammaValue(i));
      
      fprintf (file, "%d %d %d %d %s\n",
               getLevelLowInputValue(i),
               getLevelHighInputValue(i),
               getLevelLowOutputValue(i),
               getLevelHighInputValue(i), 
               buf);
      }

    fflush(file);
    fclose(file);
  
    return true;
}

}  // NameSpace Digikam
