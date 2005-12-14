/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : image histogram manipulation methods.
 *
 * Copyright 2004-2005 by Gilles Caulier
 *
 * Some code parts are inspired from gimp 2.0
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

// Qt includes.

#include <qobject.h>
#include <qevent.h>

// KDE includes.

#include <kapplication.h>
#include <kdebug.h>

// Local includes.

#include "dimg.h"
#include "imagehistogram.h"

namespace Digikam
{

ImageHistogram::ImageHistogram(const DImg& image, QObject *parent)
              : QThread()
{
    setup(image.bits(), image.width(), image.height(), image.sixteenBit(), parent);
}

ImageHistogram::ImageHistogram(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits, QObject *parent)
              : QThread()
{
    setup(i_data, i_w, i_h, i_sixteenBits, parent);
}

void ImageHistogram::setup(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits, QObject *parent)
{
    m_imageData     = i_data;
    m_imageWidth    = i_w;
    m_imageHeight   = i_h;
    m_parent        = parent;
    m_histogram     = 0L;
    m_runningFlag   = true;
    m_histoSegments = i_sixteenBits ? 65536 : 256;

    if (m_imageData && m_imageWidth && m_imageHeight)
    {
       if (m_parent)
          start();
       else
          calcHistogramValues();
    }
    else
    {
       if (m_parent)
          postProgress(false, false);
    }
}

ImageHistogram::~ImageHistogram()
{
    stopCalcHistogramValues();

    if (m_histogram)
       delete [] m_histogram;
}

int ImageHistogram::getHistogramSegment(void)
{
    return m_histoSegments;
}

void ImageHistogram::postProgress(bool starting, bool success)
{
    EventData *eventData = new EventData();
    eventData->starting  = starting;
    eventData->success   = success;
    QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, eventData));
}

void ImageHistogram::stopCalcHistogramValues(void)
{
    m_runningFlag = false;
    wait();
}

// List of threaded operations.

void ImageHistogram::run()
{
    calcHistogramValues();
}

void ImageHistogram::calcHistogramValues()
{
    register uint  i;
    int            max;

    if (m_parent)
       postProgress(true, false);

    m_histogram = new double_packet[m_histoSegments];

    if ( !m_histogram )
    {
       kdWarning() << ("HistogramWidget::calcHistogramValues: Unable to allocate memory!") << endl;

       if (m_parent)
          postProgress(false, false);

       return;
    }

    memset(m_histogram, 0, m_histoSegments*sizeof(struct double_packet));

    if (m_histoSegments == 65536)         // 16 bits image.
    {
        unsigned short blue, green, red, alpha;
        unsigned short *data = (unsigned short*)m_imageData;

        for (i = 0 ; (i < m_imageHeight*m_imageWidth*4) && m_runningFlag ; i+=4)
        {
            blue  = data[ i ];
            green = data[i+1];
            red   = data[i+2];
            alpha = data[i+3];

            m_histogram[blue].blue++;
            m_histogram[green].green++;
            m_histogram[red].red++;
            m_histogram[alpha].alpha++;

            max = (blue > green) ? blue : green;

            if (red > max) 
                m_histogram[red].value++;
            else 
                m_histogram[max].value++;
        }
    }
    else                                  // 8 bits images.
    {
        uchar blue, green, red, alpha;
        uchar *data = m_imageData;

        for (i = 0 ; (i < m_imageHeight*m_imageWidth*4) && m_runningFlag ; i+=4)
        {
            blue  = data[ i ];
            green = data[i+1];
            red   = data[i+2];
            alpha = data[i+3];

            m_histogram[blue].blue++;
            m_histogram[green].green++;
            m_histogram[red].red++;
            m_histogram[alpha].alpha++;

            max = (blue > green) ? blue : green;

            if (red > max) 
                m_histogram[red].value++;
            else 
                m_histogram[max].value++;
        }
    }

    if (m_parent && m_runningFlag)
       postProgress(false, true);
}

double ImageHistogram::getCount(int channel, int start, int end)
{
    int    i;
    double count = 0.0;

    if ( !m_histogram || start < 0 || 
        end > m_histoSegments || start > end )
        return 0.0;

    switch(channel)
    {
    case Digikam::ImageHistogram::ValueChannel:
        for (i = start ; i <= end ; i++)
            count += m_histogram[i].value;
        break;

    case Digikam::ImageHistogram::RedChannel:
        for (i = start ; i <= end ; i++)
            count += m_histogram[i].red;
        break;

    case Digikam::ImageHistogram::GreenChannel:
        for (i = start ; i <= end ; i++)
            count += m_histogram[i].green;
        break;

    case Digikam::ImageHistogram::BlueChannel:
        for (i = start ; i <= end ; i++)
            count += m_histogram[i].blue;
        break;

    case Digikam::ImageHistogram::AlphaChannel:
        for (i = start ; i <= end ; i++)
            count += m_histogram[i].alpha;
        break;

    default:
        return 0.0;
        break;
    }

  return count;
}

double ImageHistogram::getPixels()
{
  if ( !m_histogram )
       return 0.0;

  return(m_imageWidth * m_imageHeight);
}

double ImageHistogram::getMean(int channel, int start, int end)
{
    int    i;
    double mean = 0.0;
    double count;

    if ( !m_histogram || start < 0 || 
        end > m_histoSegments || start > end )
        return 0.0;

    switch(channel)
    {
        case Digikam::ImageHistogram::ValueChannel:
            for (i = start ; i <= end ; i++)
                mean += i * m_histogram[i].value;
            break;

        case Digikam::ImageHistogram::RedChannel:
            for (i = start ; i <= end ; i++)
                mean += i * m_histogram[i].red;
            break;

        case Digikam::ImageHistogram::GreenChannel:
            for (i = start ; i <= end ; i++)
                mean += i * m_histogram[i].green;
            break;

        case Digikam::ImageHistogram::BlueChannel:
            for (i = start ; i <= end ; i++)
                mean += i * m_histogram[i].blue;
            break;

        case Digikam::ImageHistogram::AlphaChannel:
            for (i = start ; i <= end ; i++)
                mean += i * m_histogram[i].alpha;
            break;

        default:
            return 0.0;
            break;
    }

    count = getCount(channel, start, end);

    if (count > 0.0)
        return mean / count;

    return mean;
}

int ImageHistogram::getMedian(int channel, int start, int end)
{
    int    i;
    double sum = 0.0;
    double count;

    if ( !m_histogram || start < 0 || 
        end > m_histoSegments || start > end )
        return 0;

    count = getCount(channel, start, end);

    switch(channel)
    {
        case Digikam::ImageHistogram::ValueChannel:
            for (i = start ; i <= end ; i++)
            {
                sum += m_histogram[i].value;
                if (sum * 2 > count) return i;
            }
            break;

        case Digikam::ImageHistogram::RedChannel:
            for (i = start ; i <= end ; i++)
            {
                sum += m_histogram[i].red;
                if (sum * 2 > count) return i;
            }
            break;

        case Digikam::ImageHistogram::GreenChannel:
            for (i = start ; i <= end ; i++)
            {
                sum += m_histogram[i].green;
                if (sum * 2 > count) return i;
            }
            break;

        case Digikam::ImageHistogram::BlueChannel:
            for (i = start ; i <= end ; i++)
            {
                sum += m_histogram[i].blue;
                if (sum * 2 > count) return i;
            }
            break;

        case Digikam::ImageHistogram::AlphaChannel:
            for (i = start ; i <= end ; i++) 
            {
                sum += m_histogram[i].alpha;
                if (sum * 2 > count) return i;
            }
            break;

        default:
            return 0;
            break;
    }

    return -1;
}

double ImageHistogram::getStdDev(int channel, int start, int end)
{
    int    i;
    double dev = 0.0;
    double count;
    double mean;

    if ( !m_histogram || start < 0 || 
        end > m_histoSegments || start > end )
        return 0.0;

    mean  = getMean(channel, start, end);
    count = getCount(channel, start, end);

    if (count == 0.0)
        count = 1.0;

    switch(channel)
    {
        case Digikam::ImageHistogram::ValueChannel:
            for (i = start ; i <= end ; i++)
                dev += (i - mean) * (i - mean) * m_histogram[i].value;
            break;

        case Digikam::ImageHistogram::RedChannel:
            for (i = start ; i <= end ; i++)
                dev += (i - mean) * (i - mean) * m_histogram[i].red;
            break;

        case Digikam::ImageHistogram::GreenChannel:
            for (i = start ; i <= end ; i++)
                dev += (i - mean) * (i - mean) * m_histogram[i].green;
            break;

        case Digikam::ImageHistogram::BlueChannel:
            for (i = start ; i <= end ; i++)
                dev += (i - mean) * (i - mean) * m_histogram[i].blue;
            break;

        case Digikam::ImageHistogram::AlphaChannel:
            for (i = start ; i <= end ; i++)
                dev += (i - mean) * (i - mean) * m_histogram[i].alpha;
            break;

        default:
            return 0.0;
            break;
    }

    return sqrt(dev / count);
}

double ImageHistogram::getValue(int channel, int bin)
{
    double value;

    if ( !m_histogram || bin < 0 || bin > m_histoSegments )
        return 0.0;

    switch(channel)
    {
       case Digikam::ImageHistogram::ValueChannel:
          value = m_histogram[bin].value;
          break;

       case Digikam::ImageHistogram::RedChannel:
          value = m_histogram[bin].red;
          break;

       case Digikam::ImageHistogram::GreenChannel:
          value = m_histogram[bin].green;
          break;

       case Digikam::ImageHistogram::BlueChannel:
          value = m_histogram[bin].blue;
          break;

       case Digikam::ImageHistogram::AlphaChannel:
          value = m_histogram[bin].alpha;
          break;

       default:
          return 0.0;
          break;
    }

    return value;
}

double ImageHistogram::getMaximum(int channel)
{
    double max = 0.0;
    int    x;

    if ( !m_histogram )
        return 0.0;

    switch(channel)
    {
       case Digikam::ImageHistogram::ValueChannel:
          for (x = 0 ; x < m_histoSegments ; x++)
             if (m_histogram[x].value > max)
                max = m_histogram[x].value;
          break;

       case Digikam::ImageHistogram::RedChannel:
          for (x = 0 ; x < m_histoSegments ; x++)
             if (m_histogram[x].red > max)
                max = m_histogram[x].red;
          break;

       case Digikam::ImageHistogram::GreenChannel:
          for (x = 0 ; x < m_histoSegments ; x++)
             if (m_histogram[x].green > max)
                max = m_histogram[x].green;
          break;

       case Digikam::ImageHistogram::BlueChannel:
          for (x = 0 ; x < m_histoSegments ; x++)
             if (m_histogram[x].blue > max)
                max = m_histogram[x].blue;
          break;

       case Digikam::ImageHistogram::AlphaChannel:
          for (x = 0 ; x < m_histoSegments ; x++)
             if (m_histogram[x].alpha > max)
                max = m_histogram[x].alpha;
          break;

       default:
          return 0.0;
          break;
    }

    return max;
}

}  // NameSpace Digikam

