/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : equalize image filter.
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

#include "equalizefilter.h"

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

EqualizeFilter::EqualizeFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}


EqualizeFilter::EqualizeFilter(DImg* orgImage, const DImg* refImage, QObject* parent)
    : DImgThreadedFilter(orgImage, parent, "EqualizeFilter"),
      m_refImage(*refImage)
{
    initFilter();
}

EqualizeFilter::~EqualizeFilter()
{
}

void EqualizeFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    equalizeImage();
    m_destImage = m_orgImage;
}

/** Performs an histogram equalization of the image.
    this method adjusts the brightness of colors across the
    active image so that the histogram for the value channel
    is as nearly as possible flat, that is, so that each possible
    brightness value appears at about the same number of pixels
    as each other value. Sometimes Equalize works wonderfully at
    enhancing the contrasts in an image. Other times it gives
    garbage. It is a very powerful operation, which can either work
    miracles on an image or destroy it.*/
void EqualizeFilter::equalizeImage()
{
    struct double_packet  high, low, intensity;
    struct double_packet* map;
    struct int_packet*    equalize_map;
    register int          i;
    int                   progress;

    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        kDebug() << "Ref. image and Org. has different bits depth";
        return;
    }

    // Create an histogram of the reference image.
    ImageHistogram* histogram = new ImageHistogram(m_refImage.bits(), m_refImage.width(),
            m_refImage.height(), m_refImage.sixteenBit());
    histogram->calculate();

    // Memory allocation.
    map          = new double_packet[histogram->getHistogramSegments()];
    equalize_map = new int_packet[histogram->getHistogramSegments()];

    if ( !histogram || !map || !equalize_map )
    {
        delete histogram;
        if (map)
        {
            delete [] map;
        }

        if (equalize_map)
        {
            delete [] equalize_map;
        }

        kWarning() << ("Unable to allocate memory!");
        return;
    }

    // Integrate the histogram to get the equalization map.

    memset(&intensity, 0, sizeof(struct double_packet));
    memset(&high,      0, sizeof(struct double_packet));
    memset(&low,       0, sizeof(struct double_packet));

    for (i = 0 ; runningFlag() && (i < histogram->getHistogramSegments()) ; ++i)
    {
        intensity.red   += histogram->getValue(RedChannel, i);
        intensity.green += histogram->getValue(GreenChannel, i);
        intensity.blue  += histogram->getValue(BlueChannel, i);
        intensity.alpha += histogram->getValue(AlphaChannel, i);
        map[i]           = intensity;
    }

    // Stretch the histogram.

    low  = map[0];
    high = map[histogram->getHistogramSegments()-1];
    memset(equalize_map, 0, histogram->getHistogramSegments()*sizeof(int_packet));

    // TODO magic number 256
    for (i = 0 ; runningFlag() && (i < histogram->getHistogramSegments()) ; ++i)
    {
        if (high.red != low.red)
            equalize_map[i].red = (uint)(((256*histogram->getHistogramSegments() -1) *
                                          (map[i].red-low.red))/(high.red-low.red));

        if (high.green != low.green)
            equalize_map[i].green = (uint)(((256*histogram->getHistogramSegments() -1) *
                                            (map[i].green-low.green))/(high.green-low.green));

        if (high.blue != low.blue)
            equalize_map[i].blue = (uint)(((256*histogram->getHistogramSegments() -1) *
                                           (map[i].blue-low.blue))/(high.blue-low.blue));

        if (high.alpha != low.alpha)
            equalize_map[i].alpha = (uint)(((256*histogram->getHistogramSegments() -1) *
                                            (map[i].alpha-low.alpha))/(high.alpha-low.alpha));
    }

    delete histogram;
    delete [] map;

    uchar* data     = m_orgImage.bits();
    int w           = m_orgImage.width();
    int h           = m_orgImage.height();
    bool sixteenBit = m_orgImage.sixteenBit();
    int size        = w*h;

    // Apply results to image.
    // TODO magic number 257
    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue, alpha;
        uchar* ptr = data;

        for (i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (equalize_map[red].red)/257;
            }

            if (low.green != high.green)
            {
                green = (equalize_map[green].green)/257;
            }

            if (low.blue != high.blue)
            {
                blue = (equalize_map[blue].blue)/257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (equalize_map[alpha].alpha)/257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ( progress%5 == 0 )
            {
                postProgress( progress );
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue, alpha;
        unsigned short* ptr = (unsigned short*)data;

        for (i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (equalize_map[red].red)/257;
            }

            if (low.green != high.green)
            {
                green = (equalize_map[green].green)/257;
            }

            if (low.blue != high.blue)
            {
                blue = (equalize_map[blue].blue)/257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (equalize_map[alpha].alpha)/257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ( progress%5 == 0 )
            {
                postProgress( progress );
            }
        }
    }

    delete [] equalize_map;
}

FilterAction EqualizeFilter::filterAction()
{
    return DefaultFilterAction<EqualizeFilter>();
}

void EqualizeFilter::readParameters(const Digikam::FilterAction& /*action*/)
{
    return; //Digikam::DImgThreadedFilter::readParameters(action);
}

}  // namespace Digikam
