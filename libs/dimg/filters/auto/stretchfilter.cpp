/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : stretch contrast image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "stretchfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagehistogram.h"

namespace Digikam
{

struct double_packet
{
    double red;
    double green;
    double blue;
    double alpha;
};

struct int_packet
{
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;
};

StretchFilter::StretchFilter(DImg* orgImage, QObject* parent)
              : DImgThreadedFilter(orgImage, parent, "StretchFilter")
{
    initFilter();
}

StretchFilter::StretchFilter(uchar* bits, uint width, uint height, bool sixteenBits)
              : DImgThreadedFilter()
{
    stretchContrastImage(bits, width, height, sixteenBits);
}

StretchFilter::~StretchFilter()
{
}

void StretchFilter::filterImage()
{
    stretchContrastImage(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    m_destImage = m_orgImage;
}


/** Performs histogram normalization of the image. The algorithm normalizes
    the pixel values from an image for to span the full range
    of color values. This is a contrast enhancement technique.*/
void StretchFilter::stretchContrastImage(uchar* data, int w, int h, bool sixteenBit)
{
    if (!data || !w || !h)
    {
       kWarning() << ("no image data available!");
       return;
    }

    struct double_packet high, low, intensity;
    struct int_packet*   normalize_map;
    long long            number_pixels;
    register long        i;
    int                  progress;
    unsigned long        threshold_intensity;

    // Create an histogram of the current image.
    ImageHistogram* histogram = new ImageHistogram(data, w, h, sixteenBit);
    histogram->calculate();

    // Memory allocation.
    normalize_map = new int_packet[histogram->getHistogramSegments()];

    if( !histogram || !normalize_map )
    {
       if(histogram)
           delete histogram;

       if(normalize_map)
           delete [] normalize_map;

       kWarning() << ("Unable to allocate memory!");
       return;
    }

    // Find the histogram boundaries by locating the 0.1 percent levels.

    number_pixels = (long long)(w*h);
    threshold_intensity = number_pixels / 1000;

    memset(&high, 0, sizeof(struct double_packet));
    memset(&low,  0, sizeof(struct double_packet));

    // Red.

    memset(&intensity, 0, sizeof(struct double_packet));

    for(high.red = histogram->getMaxSegmentIndex() ; high.red != 0 ; --high.red)
    {
       intensity.red += histogram->getValue(RedChannel, (int)high.red);

       if( intensity.red > threshold_intensity )
          break;
    }

    if( low.red == high.red )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));

       for(low.red = 0 ; low.red < histogram->getMaxSegmentIndex() ; ++low.red)
       {
          intensity.red += histogram->getValue(RedChannel, (int)low.red);

          if( intensity.red > threshold_intensity )
              break;
       }

       memset(&intensity, 0, sizeof(struct double_packet));

       for(high.red = histogram->getMaxSegmentIndex() ; high.red != 0 ; --high.red)
       {
          intensity.red += histogram->getValue(RedChannel, (int)high.red);

          if( intensity.red > threshold_intensity )
             break;
       }
    }

    // Green.

    memset(&intensity, 0, sizeof(struct double_packet));

    for(high.green = histogram->getMaxSegmentIndex() ; high.green != 0 ; --high.green)
    {
       intensity.green += histogram->getValue(GreenChannel, (int)high.green);

       if( intensity.green > threshold_intensity )
          break;
    }

    if( low.green == high.green )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));

       for(low.green = 0 ; low.green < histogram->getMaxSegmentIndex() ; ++low.green)
       {
          intensity.green += histogram->getValue(GreenChannel, (int)low.green);

          if( intensity.green > threshold_intensity )
             break;
       }

       memset(&intensity, 0, sizeof(struct double_packet));

       for(high.green = histogram->getMaxSegmentIndex() ; high.green != 0 ; --high.green)
       {
          intensity.green += histogram->getValue(GreenChannel, (int)high.green);

          if( intensity.green > threshold_intensity )
             break;
       }
    }

    // Blue.

    memset(&intensity, 0, sizeof(struct double_packet));

    for(high.blue = histogram->getMaxSegmentIndex() ; high.blue != 0 ; --high.blue)
    {
       intensity.blue += histogram->getValue(BlueChannel, (int)high.blue);

       if( intensity.blue > threshold_intensity )
          break;
    }

    if( low.blue == high.blue )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));

       for(low.blue = 0 ; low.blue < histogram->getMaxSegmentIndex() ; ++low.blue)
       {
          intensity.blue += histogram->getValue(BlueChannel, (int)low.blue);

          if( intensity.blue > threshold_intensity )
              break;
       }

       memset(&intensity, 0, sizeof(struct double_packet));

       for(high.blue = histogram->getMaxSegmentIndex() ; high.blue != 0 ; --high.blue)
       {
          intensity.blue += histogram->getValue(BlueChannel, (int)high.blue);

          if( intensity.blue > threshold_intensity )
             break;
       }
    }

    // Alpha.

    memset(&intensity, 0, sizeof(struct double_packet));

    for(high.alpha = histogram->getMaxSegmentIndex() ; high.alpha != 0 ; --high.alpha)
    {
       intensity.alpha += histogram->getValue(AlphaChannel, (int)high.alpha);

       if( intensity.alpha > threshold_intensity )
          break;
    }

    if( low.alpha == high.alpha )
    {
       threshold_intensity = 0;
       memset(&intensity, 0, sizeof(struct double_packet));

       for(low.alpha = 0 ; low.alpha < histogram->getMaxSegmentIndex() ; ++low.alpha)
       {
          intensity.alpha += histogram->getValue(AlphaChannel, (int)low.alpha);

          if( intensity.alpha > threshold_intensity )
             break;
       }

       memset(&intensity, 0, sizeof(struct double_packet));

       for(high.alpha = histogram->getMaxSegmentIndex() ; high.alpha != 0 ; --high.alpha)
       {
          intensity.alpha += histogram->getValue(AlphaChannel, (int)high.alpha);

          if( intensity.alpha > threshold_intensity )
             break;
       }
    }

    // Stretch the histogram to create the normalized image mapping.

    memset(normalize_map, 0, histogram->getHistogramSegments()*sizeof(struct int_packet));

    // TODO magic number 256
    for(i = 0 ; !m_cancel && (i <= (long)histogram->getMaxSegmentIndex()) ; ++i)
    {
       if(i < (long) low.red)
          normalize_map[i].red = 0;
       else if (i > (long) high.red)
          normalize_map[i].red = (256*histogram->getHistogramSegments() -1);
       else if (low.red != high.red)
          normalize_map[i].red = (int)(((256*histogram->getHistogramSegments() -1)*(i-low.red))/(high.red-low.red));

       if(i < (long) low.green)
          normalize_map[i].green = 0;
       else if (i > (long) high.green)
          normalize_map[i].green = (256*histogram->getHistogramSegments() -1);
       else if (low.green != high.green)
          normalize_map[i].green = (int)(((256*histogram->getHistogramSegments() -1)*(i-low.green))/(high.green-low.green));

       if(i < (long) low.blue)
          normalize_map[i].blue = 0;
       else if (i > (long) high.blue)
          normalize_map[i].blue = (256*histogram->getHistogramSegments() -1);
       else if (low.blue != high.blue)
          normalize_map[i].blue = (int)(((256*histogram->getHistogramSegments() -1)*(i-low.blue))/(high.blue-low.blue));

       if(i < (long) low.alpha)
          normalize_map[i].alpha = 0;
       else if (i > (long) high.alpha)
          normalize_map[i].alpha = (256*histogram->getHistogramSegments() -1);
       else if (low.alpha != high.alpha)
          normalize_map[i].alpha = (int)(((256*histogram->getHistogramSegments() -1)*(i-low.alpha))/(high.alpha-low.alpha));
    }

    int size = w*h;

    // Apply result to image.
    // TODO magic number 257
    if (!sixteenBit)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar *ptr = data;

        for (i = 0 ; !m_cancel && (i < size) ; ++i)
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

            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short *ptr = (unsigned short *)data;

        for (i = 0 ; !m_cancel && (i < size) ; ++i)
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

            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }

    delete histogram;
    delete [] normalize_map;
}

}  // namespace Digikam
