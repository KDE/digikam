/* ============================================================
 * File  : curveswidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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
#include <qpoint.h>
#include <qpen.h>
#include <qevent.h>
#include <qtimer.h>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>

// Digikam includes.

#include <digikam/imagehistogram.h>
#include <digikam/imagecurves.h>

// Local includes.

#include "curveswidget.h"

namespace DigikamAdjustCurvesImagesPlugin
{

CurvesWidget::CurvesWidget(int w, int h, 
                           uint *i_data, uint i_w, uint i_h, 
                           Digikam::ImageCurves *curves, QWidget *parent)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_blinkFlag      = false;
    m_clearFlag      = HistogramNone;
    m_curves         = curves;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    
    m_blinkTimer = new QTimer( this );
        
    connect( m_blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );
    
    m_imageHistogram = new Digikam::ImageHistogram(i_data, i_w, i_h, this);
}

CurvesWidget::~CurvesWidget()
{
    m_blinkTimer->stop(); 

    if (m_imageHistogram)
       delete m_imageHistogram;
}

void CurvesWidget::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ImageHistogram::EventData *d = (Digikam::ImageHistogram::EventData*) event->data();

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

void CurvesWidget::stopHistogramComputation(void)
{
    if (m_imageHistogram)
       m_imageHistogram->stopCalcHistogramValues();

    m_blinkTimer->stop(); 
}

void CurvesWidget::slotBlinkTimerDone( void )
{
    m_blinkFlag = !m_blinkFlag;
    repaint(false);
    m_blinkTimer->start( 200 ); 
}

void CurvesWidget::paintEvent( QPaintEvent * )
{
    if (m_clearFlag == HistogramStarted)
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
       
    uint   x, y;
    uint   wWidth = width();
    uint   wHeight = height();
    double max;
    class Digikam::ImageHistogram *histogram; 
    
    histogram = m_imageHistogram;
    
    x  = 0; y  = 0;
    max = 0.0;
    
    switch(m_channelType)
       {
       case CurvesWidget::GreenChannelHistogram:    // Green channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::GreenChannel);  
          break;
             
       case CurvesWidget::BlueChannelHistogram:     // Blue channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::BlueChannel);    
          break;
             
       case CurvesWidget::RedChannelHistogram:      // Red channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::RedChannel); 
          break;

       case CurvesWidget::AlphaChannelHistogram:    // Alpha channel.
          max = histogram->getMaximum(Digikam::ImageHistogram::AlphaChannel);  
          break;
       
       case CurvesWidget::ValueHistogram:           // Luminosity.
          max = histogram->getMaximum(Digikam::ImageHistogram::ValueChannel); 
          break;
       }            
             
    switch (m_scaleType)
       {
       case CurvesWidget::LinScaleHistogram:
          break;

       case CurvesWidget::LogScaleHistogram:
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
      int    curveVal;
      
      i = (x * 256) / wWidth;
      j = ((x + 1) * 256) / wWidth;

      curveVal   = m_curves->getCurveValue(m_channelType, i);
             
      do
          {
          double v;

          v  = 0.0;
          
          switch(m_channelType)
             {
             case CurvesWidget::GreenChannelHistogram:    // Green channel.
                v = histogram->getValue(Digikam::ImageHistogram::GreenChannel, ++i);   
                break;
             
             case CurvesWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(Digikam::ImageHistogram::BlueChannel, ++i);   
                break;
             
             case CurvesWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(Digikam::ImageHistogram::RedChannel, ++i);    
                break;

             case CurvesWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(Digikam::ImageHistogram::AlphaChannel, ++i);   
                break;

             case CurvesWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(Digikam::ImageHistogram::ValueChannel, ++i);   
                break;
             }            
            
          if (v > value)
             value = v;
          }
      while (i < j);

      switch (m_scaleType)
         {
         case CurvesWidget::LinScaleHistogram:
           y = (int) ((wHeight * value) / max);
           break;

         case CurvesWidget::LogScaleHistogram:
           if (value <= 0.0) value = 1.0;
           y = (int) ((wHeight * log (value)) / max);
           break;

         default:
           y = 0;
           break;
         }
 
      // Drawing histogram

      p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
      p1.drawLine(x, wHeight, x, wHeight - y);                 
      p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
      p1.drawLine(x, wHeight - y, x, 0);         
      
      // Drawing curve.   
   
      p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
      p1.drawPoint(x, wHeight - ((curveVal * 256) / wHeight)); 
      }
   
   // Drawing curves points.
      
   p1.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
            
   for (int p = 0 ; p < 17 ; ++p)
      {
      QPoint curvePoint = m_curves->getCurvePoint(m_channelType, p);
      
      p1.drawEllipse( ((curvePoint.x() * wWidth) / 256) - 3, 
                      wHeight - 3 - ((curvePoint.y() * 256) / wHeight),
                      6, 6 ); 
      }
      
   p1.end();
   bitBlt(this, 0, 0, &pm);
}

void CurvesWidget::mousePressEvent ( QMouseEvent * e )
{
}

void CurvesWidget::mouseReleaseEvent ( QMouseEvent * e )
{
}

void CurvesWidget::mouseMoveEvent ( QMouseEvent * e )
{
}

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#include "curveswidget.moc"

