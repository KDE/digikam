/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : a widget for to display an image histogram.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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
 
// C++ includes.

#include <cmath>

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qevent.h>
#include <qtimer.h>
#include <qrect.h> 
#include <qfont.h> 
#include <qfontmetrics.h> 
#include <qtooltip.h>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>

// Digikam includes.

#include <imagehistogram.h>

// Local includes.

#include "histogramwidget.h"

namespace Digikam
{

// Constructor without image data (needed to use updateData() method after instance created).

HistogramWidget::HistogramWidget(int w, int h, 
                                 QWidget *parent, bool selectMode, 
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType       = ValueHistogram;
    m_scaleType         = LogScaleHistogram;
    m_colorType         = RedColor;
    m_renderingType     = FullImageHistogram;
    m_inSelected        = false;
    m_blinkFlag         = false;
    m_clearFlag         = HistogramNone;
    m_selectMode        = selectMode;
    m_xmin              = 0;
    m_xmax              = 0;
    m_blinkComputation  = blinkComputation;
    m_guideVisible      = false;
    m_statisticsVisible = statisticsVisible;
        
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);

    m_blinkTimer = new QTimer( this );
        
    connect( m_blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );
    
    m_imageHistogram     = 0L;
    m_selectionHistogram = 0L;
}

// Constructor without image selection.

HistogramWidget::HistogramWidget(int w, int h, 
                                 uint *i_data, uint i_w, uint i_h, 
                                 QWidget *parent, bool selectMode, 
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType       = ValueHistogram;
    m_scaleType         = LogScaleHistogram;
    m_colorType         = RedColor;
    m_renderingType     = FullImageHistogram;
    m_inSelected        = false;
    m_blinkFlag         = false;
    m_clearFlag         = HistogramNone;
    m_selectMode        = selectMode;
    m_xmin              = 0;
    m_xmax              = 0;
    m_blinkComputation  = blinkComputation;
    m_guideVisible      = false;
    m_statisticsVisible = statisticsVisible;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    
    m_blinkTimer = new QTimer( this );
        
    connect( m_blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );
    
    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h, this);
    m_selectionHistogram = 0L;
}

// Constructor with image selection.

HistogramWidget::HistogramWidget(int w, int h, 
                                 uint *i_data, uint i_w, uint i_h, 
                                 uint *s_data, uint s_w, uint s_h,
                                 QWidget *parent, bool selectMode, 
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType       = ValueHistogram;
    m_scaleType         = LogScaleHistogram;
    m_colorType         = RedColor;
    m_renderingType     = FullImageHistogram;
    m_inSelected        = false;
    m_blinkFlag         = false;
    m_clearFlag         = HistogramNone;
    m_selectMode        = selectMode;
    m_xmin              = 0;
    m_xmax              = 0;
    m_blinkComputation  = blinkComputation;
    m_guideVisible      = false;
    m_statisticsVisible = statisticsVisible;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
        
    m_blinkTimer = new QTimer( this );
        
    connect( m_blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );

    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h, this);
    m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h, this);
}

HistogramWidget::~HistogramWidget()
{
    m_blinkTimer->stop(); 

    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;
}

void HistogramWidget::setHistogramGuide(QColor color)
{
    m_guideVisible = true;
    m_colorGuide   = color;
    repaint(false);
}

void HistogramWidget::reset(void)
{
    m_guideVisible = false;
    repaint(false);
}

void HistogramWidget::customEvent(QCustomEvent *event)
{
    if (!event) return;

    ImageHistogram::EventData *d = (ImageHistogram::EventData*) event->data();

    if (!d) return;

    if (d->starting)
        {
        setCursor( KCursor::waitCursor() );
        m_clearFlag = HistogramStarted;
        m_blinkTimer->start( 200 ); 
        repaint(false);
        }  
    else 
        {
        if (d->success)
            {
            // Repaint histogram 
            m_clearFlag = HistogramCompleted;
            m_blinkTimer->stop(); 
            repaint(false);
            setCursor( KCursor::arrowCursor() );    
            
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
            
            emit signalHistogramComputationDone();
            }
        else
            {
            m_clearFlag = HistogramFailed;
            m_blinkTimer->stop(); 
            repaint(false);
            setCursor( KCursor::arrowCursor() );    
            emit signalHistogramComputationFailed();
            }
        }
}

void HistogramWidget::stopHistogramComputation(void)
{
    if (m_imageHistogram)
       m_imageHistogram->stopCalcHistogramValues();

    if (m_selectionHistogram)
       m_selectionHistogram->stopCalcHistogramValues();
    
    m_blinkTimer->stop(); 
}

void HistogramWidget::updateData(uint *i_data, uint i_w, uint i_h, 
                                 uint *s_data, uint s_w, uint s_h,
                                 bool blinkComputation)
{
    m_blinkComputation = blinkComputation;
    
    // Remove old histogram data from memory.
    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;
           
    // Calc new histogram data   
    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h, this);
    
    if (s_data && s_w && s_h)
        m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h, this);
    else 
        m_selectionHistogram = 0L;
}

void HistogramWidget::slotBlinkTimerDone( void )
{
    m_blinkFlag = !m_blinkFlag;
    repaint(false);
    m_blinkTimer->start( 200 ); 
}

// This method is inspired of Gimp2.0 
// app/widgets/gimphistogramview.c::gimp_histogram_view_expose 

void HistogramWidget::paintEvent( QPaintEvent * )
{
    if (m_clearFlag == HistogramStarted && m_blinkComputation)
       {
       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, size().width(), size().height(), Qt::white);
       
       if (m_blinkFlag)
           p1.setPen(Qt::green);
       else 
           p1.setPen(Qt::darkGreen);
       
       p1.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                  i18n("Histogram\ncalculation\nin progress..."));
       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
       }
             
    if (m_clearFlag == HistogramFailed)
       {
       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, size().width(), size().height(), Qt::white);
       p1.setPen(Qt::red);
       p1.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                  i18n("Histogram\ncalculation\nfailed."));
       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
       }
       
    int    x, y;
    int    yr, yg, yb;             // For all color channels.
    int    wWidth = width();
    int    wHeight = height();
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
    
    // A QPixmap is used for enable the double buffering.
   
    QPixmap pm(size());
    QPainter p1;
    p1.begin(&pm, this);
    
    // Drawing selection or all histogram values.
           
    for (x = 0 ; x < wWidth ; x++)
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
                v = histogram->getValue(Digikam::ImageHistogram::GreenChannel, i++);   
                break;
             
             case Digikam::HistogramWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(Digikam::ImageHistogram::BlueChannel, i++);   
                break;
             
             case Digikam::HistogramWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(Digikam::ImageHistogram::RedChannel, i++);    
                break;

             case Digikam::HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(Digikam::ImageHistogram::AlphaChannel, i++);   
                break;

             case Digikam::HistogramWidget::ColorChannelsHistogram:   // All color channels.
                vr = histogram->getValue(Digikam::ImageHistogram::RedChannel, i++);   
                vg = histogram->getValue(Digikam::ImageHistogram::GreenChannel, i);   
                vb = histogram->getValue(Digikam::ImageHistogram::BlueChannel, i);   
                break;
                                                
             case Digikam::HistogramWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(Digikam::ImageHistogram::ValueChannel, i++);   
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
            if ( x >= (int)((float)(m_xmin * wWidth) / 256.0) && 
                x <= (int)((float)(m_xmax * wWidth) / 256.0) )
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
               
               if ( x == wWidth/3 || x == 2*wWidth/3 )
                  p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
               else
                  p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
               
               p1.drawLine(x, wHeight - y, x, 0);                 
               }
            }
         else 
            {
            p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
            p1.drawLine(x, wHeight, x, wHeight - y);                 
            
            if ( x == wWidth/3 || x == 2*wWidth/3 )
               p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
            else
               p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
            
            p1.drawLine(x, wHeight - y, x, 0);                 
            }
         }
      else
         {
         if ( m_selectMode == true )   // Histogram selection mode enable ?
            {
            if ( x >= (int)((float)(m_xmin * wWidth) / 256.0) && 
                x <= (int)((float)(m_xmax * wWidth) / 256.0) )
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
               // Which color must be used on the foreground with all colors channel mode?
               switch (m_colorType) 
                  {
                  case Digikam::HistogramWidget::RedColor:
                    p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);                 
                    p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);                 
                    p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);                 
                    
                    if ( x == wWidth/3 || x == 2*wWidth/3 )
                       p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                    else
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
                    
                    if ( x == wWidth/3 || x == 2*wWidth/3 )
                       p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                    else
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
                    
                    if ( x == wWidth/3 || x == 2*wWidth/3 )
                       p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                    else
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
            // Which color must be used on the foreground with all colors channel mode?
            switch (m_colorType) 
               {
               case Digikam::HistogramWidget::RedColor:
                 p1.setPen(QPen::QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);                 
                 p1.setPen(QPen::QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);                 
                 p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);                 
                 
                 if ( x == wWidth/3 || x == 2*wWidth/3 )
                    p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                 else
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
                 
                 if ( x == wWidth/3 || x == 2*wWidth/3 )
                    p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                 else
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
                 
                 if ( x == wWidth/3 || x == 2*wWidth/3 )
                    p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
                 else
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
      
   // Drawing color guide.

    p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::DotLine));      
    int guidePos;

    if (m_guideVisible)   
       {
       switch(m_channelType)
          {
          case HistogramWidget::RedChannelHistogram:      
             guidePos = m_colorGuide.red();
             break;
         
          case HistogramWidget::GreenChannelHistogram:    
             guidePos = m_colorGuide.green();
             break;
             
          case HistogramWidget::BlueChannelHistogram:     
             guidePos = m_colorGuide.blue();
             break;
             
          case HistogramWidget::ValueHistogram:    
             guidePos = QMAX(QMAX(m_colorGuide.red(), m_colorGuide.green()), m_colorGuide.blue());
             break;

          default:                                     // Alpha.
             guidePos = -1;
             break;
          }  
      
       if (guidePos != -1)
          {
          int xGuide = (guidePos * wWidth) / 256;
          p1.drawLine(xGuide, 0, xGuide, wHeight);  

          QString string = i18n("x:%1").arg(guidePos);
          QFontMetrics fontMt( string );       
          QRect rect = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string); 
          rect.setBottom(wHeight - 10);
      
          if (guidePos < wWidth/2)
             {
             rect.moveLeft(xGuide + 3);
             p1.drawText(rect, Qt::AlignLeft, string);
             }
          else
             {
             rect.moveRight(xGuide - 3);
             p1.drawText(rect, Qt::AlignRight, string);
             }
          }
       }

    if (m_statisticsVisible)   
       {
       QString tipText, value;
       QString cellBeg("<tr><td><nobr><font size=-1>");
       QString cellMid("</font></nobr></td><td><nobr><font size=-1>");
       QString cellEnd("</font></nobr></td></tr>");
       tipText  = "<table cellspacing=0 cellpadding=0>";
       
       tipText += cellBeg + i18n("Mean:") + cellMid;
       double mean = histogram->getMean(m_channelType, 0, 255);
       tipText += value.setNum(mean, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Pixels:") + cellMid;
       double pixels = histogram->getPixels();
       tipText += value.setNum((float)pixels, 'f', 0) + cellEnd;

       tipText += cellBeg + i18n("Std dev.:") + cellMid;
       double stddev = histogram->getStdDev(m_channelType, 0, 255);
       tipText += value.setNum(stddev, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Count:") + cellMid;
       double counts = histogram->getCount(m_channelType, 0, 255);
       tipText += value.setNum((float)counts, 'f', 0) + cellEnd;
       
       tipText += cellBeg + i18n("Median:") + cellMid;
       double median = histogram->getMedian(m_channelType, 0, 255);
       tipText += value.setNum(median, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Percent:") + cellMid;
       double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
       tipText += value.setNum(percentile, 'f', 1) + cellEnd;
                     
       tipText += "</table>";
    
       QToolTip::add( this, tipText);
       }      
       
    p1.end();
    bitBlt(this, 0, 0, &pm);
}

void HistogramWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( m_selectMode == true && m_clearFlag == HistogramCompleted ) // Selection mode enable ?
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
    if ( m_selectMode == true  && m_clearFlag == HistogramCompleted ) // Selection mode enable ?
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
    if ( m_selectMode == true && m_clearFlag == HistogramCompleted ) // Selection mode enable ?
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
    if ( m_selectMode == true && m_clearFlag == HistogramCompleted ) // Selection mode enable ?
       {
       m_xmin = min;
       repaint(false);    
       }
}

void HistogramWidget::slotMaxValueChanged( int max )
{
    if ( m_selectMode == true && m_clearFlag == HistogramCompleted ) // Selection mode enable ?
       {
       m_xmax = max;
       repaint(false);    
       }
}

}

#include "histogramwidget.moc"

