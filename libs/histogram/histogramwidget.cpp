/* ============================================================
 * File  : histogramwidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : a widget for to display an image histogram.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * Some code parts are inspired from from gimp 2.0
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

#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>

#include <cmath>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>

// Local includes.

#include "imagehistogram.h"
#include "histogramwidget.h"

namespace Digikam
{

HistogramWidget::HistogramWidget(int w, int h, uint *i_data,
                                 uint i_w, uint i_h, QWidget *parent)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_inSelected     = false;
    m_xmin           = 0;
    m_xmax           = 0;
    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h);

    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    emit signalMouseReleased(255);
}

HistogramWidget::~HistogramWidget()
{
    if (m_imageHistogram)
       delete m_imageHistogram;
}

// This method is inspired of Gimp2.0 
// app/widgets/gimphistogramview.c::gimp_histogram_view_expose 
void HistogramWidget::paintEvent( QPaintEvent * )
{
    uint   x, y;
    uint   wWidth = width();
    uint   wHeight = height();
    double max;
    
    switch(m_channelType)
       {
       case Digikam::HistogramWidget::GreenChannelHistogram:    // Green channel.
          max = m_imageHistogram->getMaximum(Digikam::ImageHistogram::GreenChannel);  
          break;
             
       case Digikam::HistogramWidget::BlueChannelHistogram:     // Blue channel.
          max = m_imageHistogram->getMaximum(Digikam::ImageHistogram::BlueChannel);    
          break;
             
       case Digikam::HistogramWidget::RedChannelHistogram:      // Red channel.
          max = m_imageHistogram->getMaximum(Digikam::ImageHistogram::RedChannel); 
          break;

       case Digikam::HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
          max = m_imageHistogram->getMaximum(Digikam::ImageHistogram::AlphaChannel);  
          break;
                    
       case Digikam::HistogramWidget::ValueHistogram:           // Luminosity.
          max = m_imageHistogram->getMaximum(Digikam::ImageHistogram::ValueChannel); 
          break;
       }            
             
    switch (m_scaleType)
       {
       case Digikam::HistogramWidget::LinScaleHistogram:
          break;

       case Digikam::HistogramWidget::LogScaleHistogram:
          if (max > 0.0)
              max = log (max);
          else
              max = 1.0;
          break;
       }
    
    // Drawing selection or all histogram values.
    // A QPixmap is used for enable the double buffering.
   
    QPixmap pm(size());
    QPainter p1;
    p1.begin(&pm, this);
       
    for (x = 0 ; x < wWidth ; ++x)
      {
      double value = 0.0;
      int    i, j;
    
      i = (x * 256) / wWidth;
      j = ((x + 1) * 256) / wWidth;

      do
          {
          double v;

          switch(m_channelType)
             {
             case Digikam::HistogramWidget::GreenChannelHistogram:    // Green channel.
                v = m_imageHistogram->getValue(Digikam::ImageHistogram::GreenChannel, ++i);   
                break;
             
             case Digikam::HistogramWidget::BlueChannelHistogram:     // Blue channel.
                v = m_imageHistogram->getValue(Digikam::ImageHistogram::BlueChannel, ++i);   
                break;
             
             case Digikam::HistogramWidget::RedChannelHistogram:      // Red channel.
                v = m_imageHistogram->getValue(Digikam::ImageHistogram::RedChannel, ++i);    
                break;

             case Digikam::HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
                v = m_imageHistogram->getValue(Digikam::ImageHistogram::AlphaChannel, ++i);   
                break;
                                
             case Digikam::HistogramWidget::ValueHistogram:           // Luminosity.
                v = m_imageHistogram->getValue(Digikam::ImageHistogram::ValueChannel, ++i);   
                break;
             }            
            
          if (v > value)
             value = v;
          }
      while (i < j);

      switch (m_scaleType)
        {
        case Digikam::HistogramWidget::LinScaleHistogram:
          y = (int) ((wHeight * value) / max);
          break;

        case Digikam::HistogramWidget::LogScaleHistogram:
          if (value <= 0.0) value = 1.0;
          y = (int) ((wHeight * log (value)) / max);
          break;

        default:
          y = 0;
          break;
        }

      // Drawing the histogram + selection or only the histogram.
       
      if ( x >= (uint)((float)(m_xmin * wWidth) / 256.0) && 
           x <= (uint)((float)(m_xmax * wWidth) / 256.0) )
         {
         p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
         p1.drawLine(x, wHeight, x, 0);
         p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
         p1.drawLine(x, wHeight, x, wHeight - y);                 
         }
      else 
         {
         p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
         p1.drawLine(x, wHeight, x, wHeight - y);                 
         p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
         p1.drawLine(x, wHeight - y, x, 0);                 
         }
      }
      
      p1.end();
      bitBlt(this, 0, 0, &pm);
}

void HistogramWidget::mousePressEvent ( QMouseEvent * e )
{
    if (!m_inSelected) 
       {
       m_inSelected = true;
       m_xmin = 0;
       m_xmax = 0;
       repaint(false);
       }
       
    m_xmin = (int)(e->pos().x()*(256.0/(float)width()));
    m_xminOrg = m_xmin;
    emit signalMousePressed( m_xmin );
}

void HistogramWidget::mouseReleaseEvent ( QMouseEvent * e )
{
    m_inSelected = false;
    int max = (int)(e->pos().x()*(256.0/(float)width()));
    
    if (max < m_xminOrg) 
       {
       m_xmax = m_xminOrg;
       m_xmin = max;
       emit signalMousePressed( m_xmin );
       }
    else 
       {
       m_xmin = m_xminOrg;
       m_xmax = max;
       }
    
    emit signalMouseReleased( m_xmax );
}

void HistogramWidget::mouseMoveEvent ( QMouseEvent * e )
{
    setCursor( KCursor::crossCursor() );
    
    if (m_inSelected)
       {
       int max = (int)(e->pos().x()*(256.0/(float)width()));
    
       if (max < m_xminOrg) 
          {
          m_xmax = m_xminOrg;
          m_xmin = max;
          emit signalMousePressed( m_xmin );
          }
       else 
          {
          m_xmin = m_xminOrg;
          m_xmax = max;
          }

       emit signalMouseReleased( m_xmax );
                 
       repaint(false);
       }
}

void HistogramWidget::slotMinValueChanged( int min )
{
    m_xmin = min;
    repaint(false);    
}

void HistogramWidget::slotMaxValueChanged( int max )
{
    m_xmax = max;
    repaint(false);    
}

}

#include "histogramwidget.moc"

