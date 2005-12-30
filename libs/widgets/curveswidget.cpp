/* ============================================================
 * File  : curveswidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))
  
// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpen.h>
#include <qevent.h>
#include <qtimer.h>
#include <qrect.h> 
#include <qcolor.h>
#include <qfont.h> 
#include <qfontmetrics.h> 

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>

// Digikam includes.

#include "imagehistogram.h"
#include "imagecurves.h"

// Local includes.

#include "curveswidget.h"

namespace Digikam
{

CurvesWidget::CurvesWidget(int w, int h, 
                           uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits,
                           Digikam::ImageCurves *curves, QWidget *parent, 
                           bool readOnly)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_blinkFlag      = false;
    m_clearFlag      = HistogramNone;
    m_curves         = curves;
    m_grab_point     = -1;    
    m_last           = 0;
    m_readOnlyMode   = readOnly;
    m_guideVisible   = false;
    m_sixteenBits    = i_sixteenBits;
    m_xMouseOver     = -1;
    m_yMouseOver     = -1;
   
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
    
    m_blinkTimer = new QTimer( this );
        
    connect( m_blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );
    
    m_imageHistogram = new Digikam::ImageHistogram(i_data, i_w, i_h, i_sixteenBits, this);
}

CurvesWidget::~CurvesWidget()
{
    m_blinkTimer->stop(); 

    if (m_imageHistogram)
       delete m_imageHistogram;
}

void CurvesWidget::reset(void)
{
    m_grab_point   = -1;    
    m_guideVisible = false;
    repaint(false);
}

void CurvesWidget::setCurveGuide(DColor color)
{
    m_guideVisible = true;
    m_colorGuide   = color;
    repaint(false);
}

void CurvesWidget::curveTypeChanged(void)
{
    switch ( m_curves->getCurveType(m_channelType) )
       {
       case Digikam::ImageCurves::CURVE_SMOOTH:
  
          //  pick representative points from the curve and make them control points
          
          for (int i = 0; i <= 8; i++)
          {
             int index = CLAMP(i * m_imageHistogram->getHistogramSegment()/8,
                               0, m_imageHistogram->getHistogramSegment()-1);
             
             m_curves->setCurvePoint( m_channelType,
                                      i * 2, QPoint::QPoint(index, 
                                             m_curves->getCurveValue(m_channelType,
                                             index)) );
          }
          
          m_curves->curvesCalculateCurve(m_channelType);
          break;
         
       case Digikam::ImageCurves::CURVE_FREE:
          break;
       }
                       
    repaint(false);             
    emit signalCurvesChanged();        
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
    
    delete d;
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
       
    int    x, y;
    int    wWidth = width();
    int    wHeight = height();
    double max;
    class Digikam::ImageHistogram *histogram; 
    
    histogram = m_imageHistogram;
    
    x   = 0; 
    y   = 0;
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
    
    int curvePrevVal = 0;
       
    for (x = 0 ; x < wWidth ; x++)
    {
      double value = 0.0; 
      int    i, j;
      int    curveVal;
      
      i = (x * histogram->getHistogramSegment()) / wWidth;
      j = ((x + 1) * histogram->getHistogramSegment()) / wWidth;

      curveVal = m_curves->getCurveValue(m_channelType, i);
             
      do
      {
          double v = 0.0;
          
          switch(m_channelType)
          {
             case CurvesWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(Digikam::ImageHistogram::RedChannel, i++);    
                break;
             
             case CurvesWidget::GreenChannelHistogram:    // Green channel.
                v = histogram->getValue(Digikam::ImageHistogram::GreenChannel, i++);   
                break;
             
             case CurvesWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(Digikam::ImageHistogram::BlueChannel, i++);   
                break;
             
             case CurvesWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(Digikam::ImageHistogram::AlphaChannel, i++);   
                break;

             case CurvesWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(Digikam::ImageHistogram::ValueChannel, i++);   
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

      p1.setPen(QPen::QPen(Qt::lightGray, 1, Qt::SolidLine));
      p1.drawLine(x, wHeight, x, wHeight - y);                 
      p1.setPen(QPen::QPen(Qt::white, 1, Qt::SolidLine));
      p1.drawLine(x, wHeight - y, x, 0);         
      
      // Drawing curves.   
   
      switch(m_channelType)
      {
         case CurvesWidget::RedChannelHistogram:      // Red channel.
            p1.setPen(QPen::QPen(Qt::darkRed, 1, Qt::SolidLine));
            break;
         
         case CurvesWidget::GreenChannelHistogram:    // Green channel.
            p1.setPen(QPen::QPen(Qt::darkGreen, 1, Qt::SolidLine));
            break;
             
         case CurvesWidget::BlueChannelHistogram:     // Blue channel.
            p1.setPen(QPen::QPen(Qt::darkBlue, 1, Qt::SolidLine));
            break;
             
         default:                                     // Luminosity or Alpha.
            p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
            break;
      }            
      
      p1.drawLine(x - 1, wHeight - ((curvePrevVal * wHeight) / histogram->getHistogramSegment()),
                  x,     wHeight - ((curveVal * wHeight) / histogram->getHistogramSegment()));            
      
      curvePrevVal = curveVal;
   }
   
   // Drawing curves points.
   
   if ( !m_readOnlyMode && m_curves->getCurveType(m_channelType) == Digikam::ImageCurves::CURVE_SMOOTH )
   {      
      p1.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
            
      for (int p = 0 ; p < 17 ; p++)
      {
         QPoint curvePoint = m_curves->getCurvePoint(m_channelType, p);
         
         if (curvePoint.x() >= 0)
         {
             p1.drawEllipse( ((curvePoint.x() * wWidth) / histogram->getHistogramSegment()) - 2, 
                             wHeight - 2 - ((curvePoint.y() * wHeight) / histogram->getHistogramSegment()),
                             4, 4 ); 
         }
      }
   }

   // Drawing black/middle/highlight tone grid separators.
      
   p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
   p1.drawLine(wWidth/4, 0, wWidth/4, wHeight);                 
   p1.drawLine(wWidth/2, 0, wWidth/2, wHeight);                 
   p1.drawLine(3*wWidth/4, 0, 3*wWidth/4, wHeight);                 
   p1.drawLine(0, wHeight/4, wWidth, wHeight/4);                 
   p1.drawLine(0, wHeight/2, wWidth, wHeight/2);                 
   p1.drawLine(0, 3*wHeight/4, wWidth, 3*wHeight/4);     
            
   // Drawing X,Y point position dragged by mouse over widget.
   
   p1.setPen(QPen::QPen(Qt::red, 1, Qt::DotLine));      

   if (m_xMouseOver != -1 && m_yMouseOver != -1)
   {
        QString string = i18n("x:%1\ny:%2").arg(m_xMouseOver).arg(m_yMouseOver);
        QFontMetrics fontMt( string );
        QRect rect = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string);
        rect.moveRight(wWidth);
        rect.moveBottom(wHeight);
        p1.drawText(rect, Qt::AlignLeft||Qt::AlignTop, string);
   }

   // Drawing color guide.

   int guidePos;

   if (m_guideVisible)   
   {
      switch(m_channelType)
      {
         case CurvesWidget::RedChannelHistogram:      
            guidePos = m_colorGuide.red();
            break;
         
         case CurvesWidget::GreenChannelHistogram:    
            guidePos = m_colorGuide.green();
            break;
             
         case CurvesWidget::BlueChannelHistogram:     
            guidePos = m_colorGuide.blue();
            break;
             
         case CurvesWidget::ValueHistogram:    
            guidePos = QMAX(QMAX(m_colorGuide.red(), m_colorGuide.green()), m_colorGuide.blue());
            break;

         default:                                     // Alpha.
            guidePos = -1;         
            break;
      }  
      
      if (guidePos != -1)
      {
          int xGuide = (guidePos * wWidth) / histogram->getHistogramSegment();
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
      
   // Drawing frame.

   p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));      
   p1.drawRect(0, 0, width(), height());

   p1.end();
   bitBlt(this, 0, 0, &pm);
}

void CurvesWidget::mousePressEvent ( QMouseEvent * e )
{
   if (m_readOnlyMode) return;
   
   int i;
   int closest_point;
   int distance;

   if (e->button() != Qt::LeftButton || m_clearFlag == HistogramStarted)
      return;

   int x = CLAMP( (int)(e->pos().x() * 
                       ((float)(m_imageHistogram->getHistogramSegment()-1) / (float)width())),
                  0, m_imageHistogram->getHistogramSegment()-1 );
   int y = CLAMP( (int)(e->pos().y() *
                       ((float)(m_imageHistogram->getHistogramSegment()-1) / (float)height())),
                  0, m_imageHistogram->getHistogramSegment()-1 );

   distance = 65536;
   
   for (i = 0, closest_point = 0 ; i < 17 ; i++)
   {
      int xcurvepoint = m_curves->getCurvePointX(m_channelType, i);
      
      if (xcurvepoint != -1)
      {
         if (abs (x - xcurvepoint) < distance)
         {
            distance = abs (x - xcurvepoint);
            closest_point = i;
         }
      }
   }
         
   int delta = m_imageHistogram->getHistogramSegment()/16;
   if (distance > 8)
      closest_point = (x + delta/2) / delta;
   
   setCursor( KCursor::crossCursor() );

   switch( m_curves->getCurveType(m_channelType) )
   {
      case Digikam::ImageCurves::CURVE_SMOOTH:
      {   
         // Determine the leftmost and rightmost points.
         
         m_leftmost = -1;
         
         for (i = closest_point - 1 ; i >= 0 ; i--)
         {
            if (m_curves->getCurvePointX(m_channelType, i) != -1)
            {
               m_leftmost = m_curves->getCurvePointX(m_channelType, i);
               break;
            }
         }
           
         m_rightmost = m_imageHistogram->getHistogramSegment();
         
         for (i = closest_point + 1 ; i < 17 ; i++)
         {
            if (m_curves->getCurvePointX(m_channelType, i) != -1)
            {
               m_rightmost = m_curves->getCurvePointX(m_channelType, i);
               break;
            }
         }
         
         m_grab_point = closest_point;
         m_curves->setCurvePoint(m_channelType, m_grab_point,
                                 QPoint::QPoint(x, m_imageHistogram->getHistogramSegment() - y));
         
         break;
      }

      case Digikam::ImageCurves::CURVE_FREE:
      {

         m_curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegment() - y);
         m_grab_point = x;
         m_last = y;
         break;
      }
   }

   m_curves->curvesCalculateCurve(m_channelType);
   repaint(false);
}

void CurvesWidget::mouseReleaseEvent ( QMouseEvent * e )
{
   if (m_readOnlyMode) return;
   
   if (e->button() != Qt::LeftButton || m_clearFlag == HistogramStarted)
      return;
   
   setCursor( KCursor::arrowCursor() );    
   m_grab_point = -1;
   m_curves->curvesCalculateCurve(m_channelType);
   repaint(false);
   emit signalCurvesChanged();
}

void CurvesWidget::mouseMoveEvent ( QMouseEvent * e )
{
   if (m_readOnlyMode) return;
   
   int i;
   int closest_point;
   int x1, x2, y1, y2;
   int distance;

   if (m_clearFlag == HistogramStarted)
      return;
   
   int x = CLAMP( (int)(e->pos().x()*((float)(m_imageHistogram->getHistogramSegment()-1)/(float)width())),
                  0, m_imageHistogram->getHistogramSegment()-1 );
   int y = CLAMP( (int)(e->pos().y()*((float)(m_imageHistogram->getHistogramSegment()-1)/(float)height())),
                  0, m_imageHistogram->getHistogramSegment()-1 );

   distance = 65536;
   
   for (i = 0, closest_point = 0 ; i < 17 ; i++)
   {
      if (m_curves->getCurvePointX(m_channelType, i) != -1)
      {
         if (abs (x - m_curves->getCurvePointX(m_channelType, i)) < distance)
         {
            distance = abs (x - m_curves->getCurvePointX(m_channelType, i));
            closest_point = i;
         }
      }
   }
         
   int delta = m_imageHistogram->getHistogramSegment()/16;
   if (distance > 8)
      closest_point = (x + delta/2) / delta;
   
   switch ( m_curves->getCurveType(m_channelType) )
   {
      case Digikam::ImageCurves::CURVE_SMOOTH:
      {  
         if (m_grab_point == -1)   // If no point is grabbed... 
         {
            if ( m_curves->getCurvePointX(m_channelType, closest_point) != -1 )
               setCursor( KCursor::arrowCursor() );    
            else
               setCursor( KCursor::crossCursor() );
         }
         else                      // Else, drag the grabbed point
         {
            setCursor( KCursor::crossCursor() );
 
            m_curves->setCurvePointX(m_channelType, m_grab_point, -1);
            
            if (x > m_leftmost && x < m_rightmost)
            {
               closest_point = (x + delta/2) / delta;
               
               if (m_curves->getCurvePointX(m_channelType, closest_point) == -1)
                  m_grab_point = closest_point;

               m_curves->setCurvePoint(m_channelType, m_grab_point, 
                                       QPoint::QPoint(x, m_imageHistogram->getHistogramSegment()-1 - y));
            }

            m_curves->curvesCalculateCurve(m_channelType);
            emit signalCurvesChanged();
         }
         
         break;
      }

      case Digikam::ImageCurves::CURVE_FREE:
      {        
        if (m_grab_point != -1)
        {
           if (m_grab_point > x)
           {
              x1 = x;
              x2 = m_grab_point;
              y1 = y;
              y2 = m_last;
           }
           else
           {
              x1 = m_grab_point;
              x2 = x;
              y1 = m_last;
              y2 = y;
           }

           if (x2 != x1)
           {
              for (i = x1 ; i <= x2 ; i++)
                 m_curves->setCurveValue(m_channelType, i, 
                    m_imageHistogram->getHistogramSegment()-1 - (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
           }
           else
              m_curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegment()-1 - y);

           m_grab_point = x;
           m_last = y;
         }

         emit signalCurvesChanged();
         
         break;
      }
   }

   m_xMouseOver = x;
   m_yMouseOver = m_imageHistogram->getHistogramSegment()-1 - y;
   emit signalMouseMoved(m_xMouseOver, m_yMouseOver);
   repaint(false);
}

void CurvesWidget::leaveEvent( QEvent * )
{
   m_xMouseOver = -1;
   m_yMouseOver = -1;
   emit signalMouseMoved(m_xMouseOver, m_yMouseOver);
   repaint(false);
}

}  // NameSpace Digikam

#include "curveswidget.moc"

