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
 
#include "imagehistogram.h"

namespace Digikam
{

ImageHistogram::ImageHistogram(uint *i_data, uint i_w, uint i_h, QObject *parent)
              : QThread()
{ 
    m_imageData   = i_data;
    m_imageWidth  = i_w;
    m_imageHeight = i_h;    
    m_parent      = parent;
    m_histogram   = 0L;
    m_runningFlag = true;   
    
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
          {
          ImageHistogram::EventData *d = new ImageHistogram::EventData;
          d->starting = false;
          d->success = false;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }
       }
}

ImageHistogram::~ImageHistogram()
{ 
    stopCalcHistogramValues();
    
    if (m_histogram)
       delete [] m_histogram;
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

// This method is inspired of Gimp2.0 
// app/base/gimphistogram.c::gimp_histogram_calculate_sub_region 

void ImageHistogram::calcHistogramValues()
{
    register uint  i;                   
    unsigned char  blue, green, red, alpha;
    int            max;
    unsigned int  *p;
    ImageHistogram::EventData *d;
    
    if (m_parent)
       {
       d = new ImageHistogram::EventData;
       d->starting = true;
       d->success = false;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
       }
        
    m_histogram = new double_packet[256];
    
    if ( !m_histogram )
       {
       kdWarning() << ("HistogramWidget::calcHistogramValues: Unable to allocate memory!") << endl;
    
       if (m_parent)
          {
          d = new ImageHistogram::EventData;
          d->starting = false;
          d->success = false;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }
           
       return;
       }
    
    memset(m_histogram, 0, 256*sizeof(struct double_packet));

    // Form histogram (RAW DATA32 ARGB extraction method).

    for (i = 0 ; (i < m_imageHeight*m_imageWidth) && m_runningFlag ; i++)
      {
      p = m_imageData + i;
      
      blue  = (unsigned char)(*p);
      green = (unsigned char)(*p >> 8);
      red   = (unsigned char)(*p >> 16);
      alpha = (unsigned char)(*p >> 24);
         
      m_histogram[blue].blue++;
      m_histogram[green].green++;
      m_histogram[red].red++;
      m_histogram[alpha].alpha++;    
         
      max = (blue > green) ? blue : green;
         
      if (red > max) m_histogram[red].value++;
      else m_histogram[max].value++;
      }

    if (m_parent && m_runningFlag)
       {
       d = new ImageHistogram::EventData;
       d->starting = false;
       d->success = true;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
       }
}

double ImageHistogram::getCount(int channel, int start, int end)
{
  int    i;
  double count = 0.0;

  if ( !m_histogram || start < 0 || 
       end > 256 || start > end )
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
  int   i;
  double mean = 0.0;
  double count;

  if ( !m_histogram || start < 0 || 
       end > 256 || start > end )
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
       end > 256 || start > end )
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
       end > 256 || start > end )
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
  
  if ( !m_histogram || bin < 0 || 
       bin > 256 )
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
  uint   x;

  if ( !m_histogram )
       return 0.0;

  switch(channel)
       {
       case Digikam::ImageHistogram::ValueChannel:
          for (x = 0 ; x < 256 ; x++)
             if (m_histogram[x].value > max)
                max = m_histogram[x].value;
          break;
             
       case Digikam::ImageHistogram::RedChannel:   
          for (x = 0 ; x < 256 ; x++)
             if (m_histogram[x].red > max)
                max = m_histogram[x].red;
          break;
             
       case Digikam::ImageHistogram::GreenChannel:  
          for (x = 0 ; x < 256 ; x++)
             if (m_histogram[x].green > max)
                max = m_histogram[x].green;
          break;

       case Digikam::ImageHistogram::BlueChannel:       
          for (x = 0 ; x < 256 ; x++)
             if (m_histogram[x].blue > max)
                max = m_histogram[x].blue;
          break;
          
       case Digikam::ImageHistogram::AlphaChannel:       
          for (x = 0 ; x < 256 ; x++)
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

