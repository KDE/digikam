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

// Digikam includes.

#include <imagehistogram.h>

// Local includes.

#include "histogramwidget.h"

namespace Digikam
{

// Constructor without image data (needed to use updateData() method after instance created).

HistogramWidget::HistogramWidget(int w, int h, 
                                 QWidget *parent, bool selectMode)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_colorType      = RedColor;
    m_renderingType  = FullImageHistogram;
    m_inSelected     = false;
    m_selectMode     = selectMode;
    m_xmin           = 0;
    m_xmax           = 0;
    
    m_imageHistogram     = 0L;
    m_selectionHistogram = 0L;

    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
}

// Constructor without image selection.

HistogramWidget::HistogramWidget(int w, int h, 
                                 uint *i_data, uint i_w, uint i_h, 
                                 QWidget *parent, bool selectMode)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_colorType      = RedColor;
    m_renderingType  = FullImageHistogram;
    m_inSelected     = false;
    m_selectMode     = selectMode;
    m_xmin           = 0;
    m_xmax           = 0;
    
    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h);
    m_selectionHistogram = 0L;

    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    emit signalMouseReleased(255);
}

// Constructor with image selection.

HistogramWidget::HistogramWidget(int w, int h, 
                                 uint *i_data, uint i_w, uint i_h, 
                                 uint *s_data, uint s_w, uint s_h,
                                 QWidget *parent, bool selectMode)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_colorType      = RedColor;
    m_renderingType  = FullImageHistogram;
    m_inSelected     = false;
    m_selectMode     = selectMode;
    m_xmin           = 0;
    m_xmax           = 0;
    
    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h);
    m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h);

    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    emit signalMouseReleased(255);
}

HistogramWidget::~HistogramWidget()
{
    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;
}

void HistogramWidget::updateData(uint *i_data, uint i_w, uint i_h, 
                                 uint *s_data, uint s_w, uint s_h)
{
    // Remove old histogram data from memory.
    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;
    
    // Calc new histogram data   
    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h);
    m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h);
    
    // Repaint histogram 
    repaint(false);
    
    // Send signal to refresh information if necessary.
    if ( m_xmax == 0 && m_xmin == 0)
       {
       emit signalMouseReleased( 255 );      // No current selection.
       }
    else
       {
       emit signalMousePressed( m_xmin );
       emit signalMouseReleased( m_xmax );   // Current selection available.
       }
    
}


// This method is inspired of Gimp2.0 
// app/widgets/gimphistogramview.c::gimp_histogram_view_expose 

void HistogramWidget::paintEvent( QPaintEvent * )
{
    uint   x, y;
    uint   yr, yg, yb;             // For all color channels.
    uint   wWidth = width();
    uint   wHeight = height();
    double max;
    class ImageHistogram *histogram; 
    
    if (m_renderingType == ImageSelectionHistogram && m_selectionHistogram)
       histogram = m_selectionHistogram;
    else 
       histogram = m_imageHistogram;
    
    x  = 0; y  = 0;
    yr = 0; yg = 0; yb = 0;
    max = 0.0;
    
    switch(m_channelType)
       {
       case Digikam::HistogramWidget::GreenChannelHistogram:    // Green channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::GreenChannel);  
          break;
             
       case Digikam::HistogramWidget::BlueChannelHistogram:     // Blue channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::BlueChannel);    
          break;
             
       case Digikam::HistogramWidget::RedChannelHistogram:      // Red channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::RedChannel); 
          break;

       case Digikam::HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::AlphaChannel);  
          break;
       
       case Digikam::HistogramWidget::ColorChannelsHistogram:   // All color channels.
          max = QMAX (QMAX (histogram->getMaximum(Digikam::ImageHistogram::RedChannel),
                            histogram->getMaximum(Digikam::ImageHistogram::GreenChannel)),
                      histogram->getMaximum(Digikam::ImageHistogram::BlueChannel));  
          break;
                    
       case Digikam::HistogramWidget::ValueHistogram:           // Luminosity.
          max = histogram->getMaximum(Digikam::ImageHistogram::ValueChannel); 
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
      double value_r = 0.0, value_g = 0.0, value_b = 0.0; // For all color channels.
      int    i, j;
    
      i = (x * 256) / wWidth;
      j = ((x + 1) * 256) / wWidth;

      do
          {
          double v;
          double vr, vg, vb;                              // For all color channels.

          v  = 0.0;
          vr = 0.0; vg = 0.0; vb = 0.0;
          
          switch(m_channelType)
             {
             case Digikam::HistogramWidget::GreenChannelHistogram:    // Green channel.
                v = histogram->getValue(Digikam::ImageHistogram::GreenChannel, ++i);   
                break;
             
             case Digikam::HistogramWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(Digikam::ImageHistogram::BlueChannel, ++i);   
                break;
             
             case Digikam::HistogramWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(Digikam::ImageHistogram::RedChannel, ++i);    
                break;

             case Digikam::HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(Digikam::ImageHistogram::AlphaChannel, ++i);   
                break;

             case Digikam::HistogramWidget::ColorChannelsHistogram:   // All color channels.
                vr = histogram->getValue(Digikam::ImageHistogram::RedChannel, ++i);   
                vg = histogram->getValue(Digikam::ImageHistogram::GreenChannel, i);   
                vb = histogram->getValue(Digikam::ImageHistogram::BlueChannel, i);   
                break;
                                                
             case Digikam::HistogramWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(Digikam::ImageHistogram::ValueChannel, ++i);   
                break;
             }            
            
          if ( m_channelType != Digikam::HistogramWidget::ColorChannelsHistogram )
             {
             if (v > value)
                value = v;
             }
          else 
             {
             if (vr > value_r)
                value_r = vr;
             if (vg > value_g)
                value_g = vg;
             if (vb > value_b)
                value_b = vb;
             }
          }
      while (i < j);

      if ( m_channelType != Digikam::HistogramWidget::ColorChannelsHistogram )
         {
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
         }
      else
         {
         switch (m_scaleType)
            {
            case Digikam::HistogramWidget::LinScaleHistogram:
              yr = (int) ((wHeight * value_r) / max);
              yg = (int) ((wHeight * value_g) / max);
              yb = (int) ((wHeight * value_b) / max);
              break;

            case Digikam::HistogramWidget::LogScaleHistogram:
              if (value_r <= 0.0) value_r = 1.0;
              if (value_g <= 0.0) value_g = 1.0;
              if (value_b <= 0.0) value_b = 1.0;
              yr = (int) ((wHeight * log (value_r)) / max);
              yg = (int) ((wHeight * log (value_g)) / max);
              yb = (int) ((wHeight * log (value_b)) / max);
              break;

            default:
              yr = 0;
              yg = 0;
              yb = 0;
              break;
            }
         }

      // Drawing the histogram + selection or only the histogram.

      if ( m_channelType != Digikam::HistogramWidget::ColorChannelsHistogram )
         {
         if ( m_selectMode == true )   // Selection mode enable ?
            {
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
         else 
            {
            p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
            p1.drawLine(x, wHeight, x, wHeight - y);                 
            p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
            p1.drawLine(x, wHeight - y, x, 0);                 
            }
         }
      else
         {
         if ( m_selectMode == true )   // Histogram selection mode enable ?
            {
            if ( x >= (uint)((float)(m_xmin * wWidth) / 256.0) && 
                x <= (uint)((float)(m_xmax * wWidth) / 256.0) )
               {
               p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, 0);
               p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
               
               // Witch color must be used on the foreground with all colors channel mode?
               switch (m_colorType) 
                  {
                  case Digikam::HistogramWidget::RedColor:
                    p1.drawLine(x, wHeight, x, wHeight - yr);     
                    break;

                  case Digikam::HistogramWidget::GreenColor:
                    p1.drawLine(x, wHeight, x, wHeight - yg);     
                    break;

                  default:
                    p1.drawLine(x, wHeight, x, wHeight - yb);     
                    break;
                  }
               }
            else 
               {
               // Witch color must be used on the foreground with all colors channel mode?
               switch (m_colorType) 
                  {
                  case Digikam::HistogramWidget::RedColor:
                    p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);                 
                    p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);                 
                    p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);                 
                    p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                    p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                    break;

                  case Digikam::HistogramWidget::GreenColor:
                    p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);                 
                    p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);                 
                    p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);                 
                    p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                    p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                    break;
  
                  default:
                    p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);                 
                    p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);                 
                    p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);                 
                    p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                    p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                    break;
                  }
               }
            }
         else 
            {
            // Witch color must be used on the foreground with all colors channel mode?
            switch (m_colorType) 
               {
               case Digikam::HistogramWidget::RedColor:
                 p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);                 
                 p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);                 
                 p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);                 
                 p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                 p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                 break;

               case Digikam::HistogramWidget::GreenColor:
                 p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);                 
                 p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);                 
                 p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);                 
                 p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                 p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                 break;

               default:
                 p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);                 
                 p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);                 
                 p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);                 
                 p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                 p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                 break;
               }
            }
         }
      }
      
      p1.end();
      bitBlt(this, 0, 0, &pm);
}

void HistogramWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( m_selectMode == true ) // Selection mode enable ?
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
}

void HistogramWidget::mouseReleaseEvent ( QMouseEvent * e )
{
    if ( m_selectMode == true ) // Selection mode enable ?
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
}

void HistogramWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( m_selectMode == true ) // Selection mode enable ?
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

