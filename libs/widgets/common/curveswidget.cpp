/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : a widget to draw histogram curves
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kcursor.h>
#include <klocale.h>

// Digikam includes.

#include "ddebug.h"
#include "imagehistogram.h"
#include "imagecurves.h"

// Local includes.

#include "curveswidget.h"
#include "curveswidget.moc"

namespace Digikam
{

class CurvesWidgetPriv
{
public:

    enum RepaintType
    {
        HistogramDataLoading = 0, // Image Data loading in progress.
        HistogramNone,            // No current histogram values calculation.
        HistogramStarted,         // Histogram values calculation started.
        HistogramCompleted,       // Histogram values calculation completed.
        HistogramFailed           // Histogram values calculation failed.
    };

    CurvesWidgetPriv()
    {
        blinkTimer   = 0;
        curves       = 0;
        grabPoint    = -1;
        last         = 0;
        guideVisible = false;
        xMouseOver   = -1;
        yMouseOver   = -1;
        clearFlag    = HistogramNone;
        pos          = 0;
    }

    int          clearFlag;          // Clear drawing zone with message.
    int          leftMost;
    int          rightMost;
    int          grabPoint;
    int          last;
    int          xMouseOver;
    int          yMouseOver;
    int          pos;                // Position of animation during loading/calculation.

    bool         sixteenBits;
    bool         readOnlyMode;
    bool         guideVisible;

    DColor       colorGuide;

    QTimer      *blinkTimer;

    ImageCurves *curves;             // Curves data instance.

};

CurvesWidget::CurvesWidget(int w, int h, QWidget *parent, bool readOnly)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CurvesWidgetPriv;

    setup(w, h, readOnly);
}

CurvesWidget::CurvesWidget(int w, int h,
                           uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits,
                           QWidget *parent, bool readOnly)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CurvesWidgetPriv;

    setup(w, h, readOnly);
    updateData(i_data, i_w, i_h, i_sixteenBits);
}

CurvesWidget::~CurvesWidget()
{
    d->blinkTimer->stop();

    if (m_imageHistogram)
       delete m_imageHistogram;

    if (d->curves)
       delete d->curves;

    delete d;
}

void CurvesWidget::setup(int w, int h, bool readOnly)
{
    d->readOnlyMode  = readOnly;
    d->curves        = new ImageCurves(true);
    m_channelType    = ValueHistogram;
    m_scaleType      = LogScaleHistogram;
    m_imageHistogram = 0;

    setMouseTracking(true);
    setPaletteBackgroundColor(colorGroup().background());
    setMinimumSize(w, h);

    d->blinkTimer = new QTimer( this );

    connect(d->blinkTimer, SIGNAL(timeout()),
            this, SLOT(slotBlinkTimerDone()));
}

void CurvesWidget::updateData(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits)
{
    stopHistogramComputation();

    d->sixteenBits = i_sixteenBits;

    // Remove old histogram data from memory.
    if (m_imageHistogram)
        delete m_imageHistogram;

    // Calc new histogram data
    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits, this);

    if (d->curves)
        delete d->curves;

    d->curves = new ImageCurves(i_sixteenBits);
    reset();
}

void CurvesWidget::reset()
{
    if (d->curves) 
        d->curves->curvesReset();

    d->grabPoint    = -1;
    d->guideVisible = false;
    repaint(false);
}

ImageCurves* CurvesWidget::curves() const
{
    return d->curves;
}

void CurvesWidget::setDataLoading()
{
    if (d->clearFlag != CurvesWidgetPriv::HistogramDataLoading)
    {
        setCursor(KCursor::waitCursor());
        d->clearFlag = CurvesWidgetPriv::HistogramDataLoading;
        d->pos       = 0;
        d->blinkTimer->start(100);
    }
}

void CurvesWidget::setLoadingFailed()
{
    d->clearFlag = CurvesWidgetPriv::HistogramFailed;
    d->pos       = 0;
    d->blinkTimer->stop();
    repaint(false);
    setCursor(KCursor::arrowCursor());
}

void CurvesWidget::setCurveGuide(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    repaint(false);
}

void CurvesWidget::curveTypeChanged()
{
    switch (d->curves->getCurveType(m_channelType))
    {
       case ImageCurves::CURVE_SMOOTH:

          //  pick representative points from the curve and make them control points

          for (int i = 0; i <= 8; i++)
          {
             int index = CLAMP(i * m_imageHistogram->getHistogramSegment()/8,
                               0, m_imageHistogram->getHistogramSegment()-1);

             d->curves->setCurvePoint( m_channelType,
                                       i * 2, QPoint(index, 
                                              d->curves->getCurveValue(m_channelType,
                                              index)) );
          }

          d->curves->curvesCalculateCurve(m_channelType);
          break;

       case ImageCurves::CURVE_FREE:
          break;
    }

    repaint(false);
    emit signalCurvesChanged();
}

void CurvesWidget::customEvent(QCustomEvent *event)
{
    if (!event) return;

    ImageHistogram::EventData *ed = (ImageHistogram::EventData*) event->data();

    if (!ed) return;

    if (ed->starting)
    {
        setCursor(KCursor::waitCursor());
        d->clearFlag = CurvesWidgetPriv::HistogramStarted;
        d->blinkTimer->start(200);
        repaint(false);
    }
    else 
    {
        if (ed->success)
        {
            // Repaint histogram 
            d->clearFlag = CurvesWidgetPriv::HistogramCompleted;
            d->blinkTimer->stop();
            repaint(false);
            setCursor(KCursor::arrowCursor());
        }
        else
        {
            d->clearFlag = CurvesWidgetPriv::HistogramFailed;
            d->blinkTimer->stop();
            repaint(false);
            setCursor(KCursor::arrowCursor());
            emit signalHistogramComputationFailed();
        }
    }

    delete ed;
}

void CurvesWidget::stopHistogramComputation()
{
    if (m_imageHistogram)
       m_imageHistogram->stopCalcHistogramValues();

    d->blinkTimer->stop();
    d->pos = 0;
}

void CurvesWidget::slotBlinkTimerDone()
{
    repaint(false);
    d->blinkTimer->start(200);
}

void CurvesWidget::paintEvent(QPaintEvent*)
{
    if (d->clearFlag == CurvesWidgetPriv::HistogramDataLoading ||
        d->clearFlag == CurvesWidgetPriv::HistogramStarted)
    {
       // In first, we draw an animation.

       int asize = 24;
       QPixmap anim(asize, asize);
       QPainter p2;
       p2.begin(&anim, this);
       p2.fillRect(0, 0, asize, asize, palette().active().background());
       p2.translate(asize/2, asize/2);

       d->pos = (d->pos + 10) % 360;
       p2.setPen(QPen(palette().active().text()));
       p2.rotate(d->pos);
       for ( int i=0 ; i<12 ; i++ )
       {
           p2.drawLine(asize/2-5, 0, asize/2-2, 0);
           p2.rotate(30);
       }
       p2.end();

       // ... and we render busy text.

       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, width(), height(), palette().active().background());
       p1.setPen(QPen(palette().active().foreground(), 1, Qt::SolidLine));
       p1.drawRect(0, 0, width(), height());
       p1.drawPixmap(width()/2 - asize /2, asize, anim);
       p1.setPen(QPen(palette().active().text()));

       if (d->clearFlag == CurvesWidgetPriv::HistogramDataLoading)
           p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                       i18n("Loading image..."));
       else
           p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                       i18n("Histogram calculation..."));

       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
    }

    if (d->clearFlag == CurvesWidgetPriv::HistogramFailed)
    {
       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, width(), height(), palette().active().background());
       p1.setPen(QPen(palette().active().foreground(), 1, Qt::SolidLine));
       p1.drawRect(0, 0, width(), height());
       p1.setPen(QPen(palette().active().text()));
       p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                   i18n("Histogram\ncalculation\nfailed."));
       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
    }

    if (!m_imageHistogram) return;

    int    x, y;
    int    wWidth  = width();
    int    wHeight = height();
    double max;
    class ImageHistogram *histogram = m_imageHistogram;

    x   = 0;
    y   = 0;
    max = 0.0;

    switch(m_channelType)
    {
       case CurvesWidget::GreenChannelHistogram:    // Green channel.
          max = histogram->getMaximum(ImageHistogram::GreenChannel);
          break;

       case CurvesWidget::BlueChannelHistogram:     // Blue channel.
          max = histogram->getMaximum(ImageHistogram::BlueChannel);
          break;

       case CurvesWidget::RedChannelHistogram:      // Red channel.
          max = histogram->getMaximum(ImageHistogram::RedChannel);
          break;

       case CurvesWidget::AlphaChannelHistogram:    // Alpha channel.
          max = histogram->getMaximum(ImageHistogram::AlphaChannel);
          break;

       case CurvesWidget::ValueHistogram:           // Luminosity.
          max = histogram->getMaximum(ImageHistogram::ValueChannel);
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

      curveVal = d->curves->getCurveValue(m_channelType, i);

      do
      {
          double v = 0.0;

          switch(m_channelType)
          {
             case CurvesWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(ImageHistogram::RedChannel, i++);
                break;

             case CurvesWidget::GreenChannelHistogram:    // Green channel.
                v = histogram->getValue(ImageHistogram::GreenChannel, i++);
                break;

             case CurvesWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(ImageHistogram::BlueChannel, i++);
                break;

             case CurvesWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(ImageHistogram::AlphaChannel, i++);
                break;

             case CurvesWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(ImageHistogram::ValueChannel, i++);
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

      p1.setPen(QPen(palette().active().foreground(), 1, Qt::SolidLine));
      p1.drawLine(x, wHeight, x, wHeight - y);
      p1.setPen(QPen(palette().active().background(), 1, Qt::SolidLine));
      p1.drawLine(x, wHeight - y, x, 0);

      // Drawing curves.

      p1.setPen(QPen(palette().active().link(), 2, Qt::SolidLine));
      p1.drawLine(x - 1, wHeight - ((curvePrevVal * wHeight) / histogram->getHistogramSegment()),
                  x,     wHeight - ((curveVal * wHeight) / histogram->getHistogramSegment()));

      curvePrevVal = curveVal;
   }

   // Drawing curves points.

   if (!d->readOnlyMode && d->curves->getCurveType(m_channelType) == ImageCurves::CURVE_SMOOTH)
   {
      p1.setPen(QPen(Qt::red, 3, Qt::SolidLine));

      for (int p = 0 ; p < 17 ; p++)
      {
         QPoint curvePoint = d->curves->getCurvePoint(m_channelType, p);

         if (curvePoint.x() >= 0)
         {
             p1.drawEllipse( ((curvePoint.x() * wWidth) / histogram->getHistogramSegment()) - 2, 
                             wHeight - 2 - ((curvePoint.y() * wHeight) / histogram->getHistogramSegment()),
                             4, 4 );
         }
      }
   }

   // Drawing black/middle/highlight tone grid separators.

   p1.setPen(QPen(palette().active().base(), 1, Qt::SolidLine));
   p1.drawLine(wWidth/4, 0, wWidth/4, wHeight);
   p1.drawLine(wWidth/2, 0, wWidth/2, wHeight);
   p1.drawLine(3*wWidth/4, 0, 3*wWidth/4, wHeight);
   p1.drawLine(0, wHeight/4, wWidth, wHeight/4);
   p1.drawLine(0, wHeight/2, wWidth, wHeight/2);
   p1.drawLine(0, 3*wHeight/4, wWidth, 3*wHeight/4);

   // Drawing X,Y point position dragged by mouse over widget.

   p1.setPen(QPen(Qt::red, 1, Qt::DotLine));

   if (d->xMouseOver != -1 && d->yMouseOver != -1)
   {
        QString string = i18n("x:%1\ny:%2").arg(d->xMouseOver).arg(d->yMouseOver);
        QFontMetrics fontMt(string);
        QRect rect = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string);
        rect.moveRight(wWidth);
        rect.moveBottom(wHeight);
        p1.drawText(rect, Qt::AlignLeft||Qt::AlignTop, string);
   }

   // Drawing color guide.

   int guidePos;

   if (d->guideVisible)
   {
      switch(m_channelType)
      {
         case CurvesWidget::RedChannelHistogram:
            guidePos = d->colorGuide.red();
            break;

         case CurvesWidget::GreenChannelHistogram:
            guidePos = d->colorGuide.green();
            break;

         case CurvesWidget::BlueChannelHistogram:
            guidePos = d->colorGuide.blue();
            break;

         case CurvesWidget::ValueHistogram:
            guidePos = QMAX(QMAX(d->colorGuide.red(), d->colorGuide.green()), d->colorGuide.blue());
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
          p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
          rect.moveTop(1);

          if (xGuide < wWidth/2)
          {
             rect.moveLeft(xGuide);
             p1.fillRect(rect, QBrush(QColor(250, 250, 255)));
             p1.drawRect(rect);
             rect.moveLeft(xGuide+3);
             p1.drawText(rect, Qt::AlignLeft, string);
          }
          else
          {
             rect.moveRight(xGuide);
             p1.fillRect(rect, QBrush(QColor(250, 250, 255)));
             p1.drawRect(rect);
             rect.moveRight(xGuide-3);
             p1.drawText(rect, Qt::AlignRight, string);
          }
      }
   }

   // Drawing frame.

   p1.setPen(QPen(palette().active().foreground(), 1, Qt::SolidLine));
   p1.drawRect(0, 0, width(), height());

   p1.end();
   bitBlt(this, 0, 0, &pm);
}

void CurvesWidget::mousePressEvent(QMouseEvent *e)
{
   if (d->readOnlyMode || !m_imageHistogram) return;

   int i;
   int closest_point;
   int distance;

   if (e->button() != Qt::LeftButton || d->clearFlag == CurvesWidgetPriv::HistogramStarted)
      return;

   int x = CLAMP((int)(e->pos().x() *
                      ((float)(m_imageHistogram->getHistogramSegment()-1) / (float)width())),
                  0, m_imageHistogram->getHistogramSegment()-1 );
   int y = CLAMP((int)(e->pos().y() *
                      ((float)(m_imageHistogram->getHistogramSegment()-1) / (float)height())),
                  0, m_imageHistogram->getHistogramSegment()-1 );

   distance = 65536;

   for (i = 0, closest_point = 0 ; i < 17 ; i++)
   {
      int xcurvepoint = d->curves->getCurvePointX(m_channelType, i);

      if (xcurvepoint != -1)
      {
         if (abs (x - xcurvepoint) < distance)
         {
            distance      = abs (x - xcurvepoint);
            closest_point = i;
         }
      }
   }

   int delta = m_imageHistogram->getHistogramSegment()/16;
   if (distance > 8)
      closest_point = (x + delta/2) / delta;

   setCursor(KCursor::crossCursor());

   switch(d->curves->getCurveType(m_channelType))
   {
      case ImageCurves::CURVE_SMOOTH:
      {
         // Determine the leftmost and rightmost points.

         d->leftMost = -1;

         for (i = closest_point - 1 ; i >= 0 ; i--)
         {
            if (d->curves->getCurvePointX(m_channelType, i) != -1)
            {
               d->leftMost = d->curves->getCurvePointX(m_channelType, i);
               break;
            }
         }

         d->rightMost = m_imageHistogram->getHistogramSegment();

         for (i = closest_point + 1 ; i < 17 ; i++)
         {
            if (d->curves->getCurvePointX(m_channelType, i) != -1)
            {
               d->rightMost = d->curves->getCurvePointX(m_channelType, i);
               break;
            }
         }

         d->grabPoint = closest_point;
         d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                  QPoint(x, m_imageHistogram->getHistogramSegment() - y));

         break;
      }

      case ImageCurves::CURVE_FREE:
      {

         d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegment() - y);
         d->grabPoint = x;
         d->last      = y;
         break;
      }
   }

   d->curves->curvesCalculateCurve(m_channelType);
   repaint(false);
}

void CurvesWidget::mouseReleaseEvent(QMouseEvent *e)
{
   if (d->readOnlyMode || !m_imageHistogram) return;

   if (e->button() != Qt::LeftButton || d->clearFlag == CurvesWidgetPriv::HistogramStarted)
      return;

   setCursor(KCursor::arrowCursor());
   d->grabPoint = -1;
   d->curves->curvesCalculateCurve(m_channelType);
   repaint(false);
   emit signalCurvesChanged();
}

void CurvesWidget::mouseMoveEvent(QMouseEvent *e)
{
   if (d->readOnlyMode || !m_imageHistogram) return;

   int i;
   int closest_point;
   int x1, x2, y1, y2;
   int distance;

   if (d->clearFlag == CurvesWidgetPriv::HistogramStarted)
      return;

   int x = CLAMP( (int)(e->pos().x()*((float)(m_imageHistogram->getHistogramSegment()-1)/(float)width())),
                  0, m_imageHistogram->getHistogramSegment()-1 );
   int y = CLAMP( (int)(e->pos().y()*((float)(m_imageHistogram->getHistogramSegment()-1)/(float)height())),
                  0, m_imageHistogram->getHistogramSegment()-1 );

   distance = 65536;

   for (i = 0, closest_point = 0 ; i < 17 ; i++)
   {
      if (d->curves->getCurvePointX(m_channelType, i) != -1)
      {
         if (abs (x - d->curves->getCurvePointX(m_channelType, i)) < distance)
         {
            distance      = abs (x - d->curves->getCurvePointX(m_channelType, i));
            closest_point = i;
         }
      }
   }

   int delta = m_imageHistogram->getHistogramSegment()/16;
   if (distance > 8)
      closest_point = (x + delta/2) / delta;

   switch ( d->curves->getCurveType(m_channelType) )
   {
      case ImageCurves::CURVE_SMOOTH:
      {
         if (d->grabPoint == -1)   // If no point is grabbed...
         {
            if ( d->curves->getCurvePointX(m_channelType, closest_point) != -1 )
               setCursor(KCursor::arrowCursor());
            else
               setCursor(KCursor::crossCursor());
         }
         else                      // Else, drag the grabbed point
         {
            setCursor(KCursor::crossCursor());

            d->curves->setCurvePointX(m_channelType, d->grabPoint, -1);

            if (x > d->leftMost && x < d->rightMost)
            {
               closest_point = (x + delta/2) / delta;

               if (d->curves->getCurvePointX(m_channelType, closest_point) == -1)
                  d->grabPoint = closest_point;

               d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                        QPoint(x, m_imageHistogram->getHistogramSegment()-1 - y));
            }

            d->curves->curvesCalculateCurve(m_channelType);
            emit signalCurvesChanged();
         }

         break;
      }

      case ImageCurves::CURVE_FREE:
      {
        if (d->grabPoint != -1)
        {
           if (d->grabPoint > x)
           {
              x1 = x;
              x2 = d->grabPoint;
              y1 = y;
              y2 = d->last;
           }
           else
           {
              x1 = d->grabPoint;
              x2 = x;
              y1 = d->last;
              y2 = y;
           }

           if (x2 != x1)
           {
              for (i = x1 ; i <= x2 ; i++)
                 d->curves->setCurveValue(m_channelType, i,
                    m_imageHistogram->getHistogramSegment()-1 - (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
           }
           else
           {
              d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegment()-1 - y);
           }

           d->grabPoint = x;
           d->last      = y;
         }

         emit signalCurvesChanged();

         break;
      }
   }

   d->xMouseOver = x;
   d->yMouseOver = m_imageHistogram->getHistogramSegment()-1 - y;
   emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
   repaint(false);
}

void CurvesWidget::leaveEvent(QEvent*)
{
   d->xMouseOver = -1;
   d->yMouseOver = -1;
   emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
   repaint(false);
}

}  // NameSpace Digikam
