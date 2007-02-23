/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2004-07-21
 * Description : a widget to display an image histogram.
 *
 * Copyright 2004-2006 by Gilles Caulier
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
#include <qcolor.h>
#include <qbrush.h>
#include <qrect.h> 
#include <qfont.h> 
#include <qfontmetrics.h> 
#include <qtooltip.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "imagehistogram.h"
#include "histogramwidget.h"

namespace Digikam
{

class HistogramWidgetPriv
{
public:

    enum RepaintType
    {
        HistogramNone = 0,        // No current histogram values calculation.
        HistogramDataLoading,     // The image is being loaded
        HistogramStarted,         // Histogram values calculation started.
        HistogramCompleted,       // Histogram values calculation completed.
        HistogramFailed           // Histogram values calculation failed.
    };
    
    HistogramWidgetPriv()
    {
        blinkTimer   = 0;
        sixteenBits  = false;
        inSelected   = false;
        blinkFlag    = false;
        clearFlag    = HistogramNone;
        xmin         = 0.0;
        xmax         = 0.0;
        range        = 255;
        guideVisible = false;
        inInitialRepaintWait = false;
    }

    // Current selection information.
    double  xmin;
    double  xminOrg;
    double  xmax;
    int     range;
    int     clearFlag;          // Clear drawing zone with message.

    bool    sixteenBits;
    bool    guideVisible;       // Display color guide.
    bool    statisticsVisible;  // Display tooltip histogram statistics.
    bool    inSelected;
    bool    selectMode;         // If true, a part of the histogram can be selected !
    bool    blinkFlag;
    bool    blinkComputation;   // If true, a message will be displayed during histogram computation,
                                // else nothing (limit flicker effect in widget especially for small
                                // image/computation time).
    bool    inInitialRepaintWait;

    QTimer *blinkTimer;

    DColor  colorGuide;
};

// Constructor without image data (needed to use updateData() method after instance created).

HistogramWidget::HistogramWidget(int w, int h, 
                                 QWidget *parent, bool selectMode,
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new HistogramWidgetPriv;
    setup(w, h, selectMode, blinkComputation, statisticsVisible);

    m_imageHistogram     = 0L;
    m_selectionHistogram = 0L;
}

// Constructor without image selection.

HistogramWidget::HistogramWidget(int w, int h,
                                 uchar *i_data, uint i_w, uint i_h,
                                 bool i_sixteenBits,
                                 QWidget *parent, bool selectMode,
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new HistogramWidgetPriv;
    d->sixteenBits = i_sixteenBits;
    setup(w, h, selectMode, blinkComputation, statisticsVisible);

    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits, this);
    m_selectionHistogram = 0L;
}

// Constructor with image selection.

HistogramWidget::HistogramWidget(int w, int h, 
                                 uchar *i_data, uint i_w, uint i_h,
                                 uchar *s_data, uint s_w, uint s_h,
                                 bool i_sixteenBits,
                                 QWidget *parent, bool selectMode,
                                 bool blinkComputation, bool statisticsVisible)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new HistogramWidgetPriv;
    d->sixteenBits = i_sixteenBits;
    setup(w, h, selectMode, blinkComputation, statisticsVisible);

    m_imageHistogram     = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits, this);
    m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits, this);
}

HistogramWidget::~HistogramWidget()
{
    d->blinkTimer->stop();

    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;

    delete d;       
}

void HistogramWidget::setup(int w, int h, bool selectMode, bool blinkComputation, bool statisticsVisible)
{
    m_channelType        = ValueHistogram;
    m_scaleType          = LogScaleHistogram;
    m_colorType          = RedColor;
    m_renderingType      = FullImageHistogram;
    d->statisticsVisible = statisticsVisible;
    d->selectMode        = selectMode;
    d->blinkComputation  = blinkComputation;

    setMouseTracking(true);
    setMinimumSize(w, h);

    d->blinkTimer = new QTimer( this );

    connect( d->blinkTimer, SIGNAL(timeout()),
             this, SLOT(slotBlinkTimerDone()) );
}

void HistogramWidget::setHistogramGuideByColor(DColor color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    repaint(false);
}

void HistogramWidget::reset(void)
{
    d->guideVisible = false;
    repaint(false);
}

void HistogramWidget::customEvent(QCustomEvent *event)
{
    if (!event) return;

    ImageHistogram::EventData *ed = (ImageHistogram::EventData*) event->data();

    if (!ed) return;

    if (ed->histogram != m_imageHistogram && ed->histogram != m_selectionHistogram)
        return;

    if (ed->starting)
    {
        setCursor( KCursor::waitCursor() );
        d->clearFlag = HistogramWidgetPriv::HistogramStarted;
        if (!d->inInitialRepaintWait)
        {
            if (d->clearFlag != HistogramWidgetPriv::HistogramDataLoading)
            {
                // enter initial repaint wait, repaint only after waiting
                // a short time so that very fast computation does not create flicker
                d->inInitialRepaintWait = true;
                d->blinkTimer->start( 100 );
            }
            else
            {
                // after the initial repaint, we can repaint immediately
                repaint(false);
                d->blinkTimer->start( 200 );
            }
        }
    }
    else 
    {
        if (ed->success)
        {
            // Repaint histogram 
            d->clearFlag = HistogramWidgetPriv::HistogramCompleted;
            d->blinkTimer->stop();
            d->inInitialRepaintWait = false;
            setCursor( KCursor::arrowCursor() );

            // Send signals to refresh information if necessary.
            // The signals may trigger multiple repaints, avoid this,
            // we repaint once afterwards.
            setUpdatesEnabled(false);

            notifyValuesChanged();
            emit signalHistogramComputationDone(d->sixteenBits);

            setUpdatesEnabled(true);
            repaint(false);
        }
        else
        {
            d->clearFlag = HistogramWidgetPriv::HistogramFailed;
            d->blinkTimer->stop();
            d->inInitialRepaintWait = false;
            repaint(false);
            setCursor( KCursor::arrowCursor() );    
            // Remove old histogram data from memory.
            if (m_imageHistogram)
            {
                delete m_imageHistogram;
                m_imageHistogram = 0;
            }
            if (m_selectionHistogram)
            {
                delete m_selectionHistogram;
                m_selectionHistogram = 0;
            }
            emit signalHistogramComputationFailed();
        }
    }

    delete ed;
}

void HistogramWidget::setDataLoading()
{
    if (d->clearFlag != HistogramWidgetPriv::HistogramDataLoading)
    {
        setCursor( KCursor::waitCursor() );
        d->clearFlag = HistogramWidgetPriv::HistogramDataLoading;
        // enter initial repaint wait, repaint only after waiting
        // a short time so that very fast computation does not create flicker
        d->inInitialRepaintWait = true;
        d->blinkTimer->start( 100 );
        //repaint(false);
    }
}

void HistogramWidget::setLoadingFailed()
{
    d->clearFlag = HistogramWidgetPriv::HistogramFailed;
    d->blinkTimer->stop();
    d->inInitialRepaintWait = false;
    repaint(false);
    setCursor( KCursor::arrowCursor() );
}

void HistogramWidget::stopHistogramComputation(void)
{
    if (m_imageHistogram)
       m_imageHistogram->stopCalcHistogramValues();

    if (m_selectionHistogram)
       m_selectionHistogram->stopCalcHistogramValues();

    d->blinkTimer->stop();
}

void HistogramWidget::updateData(uchar *i_data, uint i_w, uint i_h,
                                 bool i_sixteenBits,
                                 uchar *s_data, uint s_w, uint s_h,
                                 bool blinkComputation)
{
    d->blinkComputation = blinkComputation;
    d->sixteenBits      = i_sixteenBits;

    // We are deleting the histogram data, so we must not use it to draw any more.
    d->clearFlag = HistogramWidgetPriv::HistogramNone;

    // Do not using ImageHistogram::getHistogramSegment()
    // method here because histogram hasn't yet been computed.
    d->range = d->sixteenBits ? 65535 : 255;
    emit signalMaximumValueChanged( d->range );


    // Remove old histogram data from memory.
    if (m_imageHistogram)
       delete m_imageHistogram;

    if (m_selectionHistogram)
       delete m_selectionHistogram;

    // Calc new histogram data
    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits, this);

    if (s_data && s_w && s_h)
        m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits, this);
    else 
        m_selectionHistogram = 0L;
}

void HistogramWidget::updateSelectionData(uchar *s_data, uint s_w, uint s_h,
                                          bool i_sixteenBits,
                                          bool blinkComputation)
{
    d->blinkComputation = blinkComputation;

    // Remove old histogram data from memory.

    if (m_selectionHistogram)
       delete m_selectionHistogram;

    // Calc new histogram data
    m_selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits, this);
}

void HistogramWidget::slotBlinkTimerDone( void )
{
    d->blinkFlag = !d->blinkFlag;
    d->inInitialRepaintWait = false;
    repaint(false);
    d->blinkTimer->start( 200 );
}

void HistogramWidget::paintEvent( QPaintEvent * )
{
    // Widget is disabled, not initialized, 
    // or loading, but no message shall be drawn:
    // Drawing grayed frame.
    if (  !isEnabled() ||
           d->clearFlag == HistogramWidgetPriv::HistogramNone ||
         (!d->blinkComputation && (d->clearFlag == HistogramWidgetPriv::HistogramStarted ||
                                   d->clearFlag == HistogramWidgetPriv::HistogramDataLoading))
       )
    {
       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, size().width(), size().height(),  palette().disabled().background());
       p1.setPen(QPen(palette().disabled().foreground(), 1, Qt::SolidLine));
       p1.drawRect(0, 0, width(), height());
       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
    }
    // Image data is loading or histogram is being computed:
    // Draw message.
    else if (  d->blinkComputation &&
              (d->clearFlag == HistogramWidgetPriv::HistogramStarted ||
               d->clearFlag == HistogramWidgetPriv::HistogramDataLoading)
            )
    {
       QPixmap pm(size());
       QPainter p1;
       p1.begin(&pm, this);
       p1.fillRect(0, 0, size().width(), size().height(), Qt::white);

       if (d->blinkFlag)
           p1.setPen(Qt::green);
       else 
           p1.setPen(Qt::darkGreen);

       if (d->clearFlag == HistogramWidgetPriv::HistogramDataLoading)
           p1.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                       i18n("Loading image..."));
       else 
           p1.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                  i18n("Histogram\ncalculation\nin progress..."));
       p1.end();
       bitBlt(this, 0, 0, &pm);
       return;
    }
    // Histogram computation failed:
    // Draw message.
    else if (d->clearFlag == HistogramWidgetPriv::HistogramFailed)
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
    class  ImageHistogram *histogram; 

    if (m_renderingType == ImageSelectionHistogram && m_selectionHistogram)
       histogram = m_selectionHistogram;
    else 
       histogram = m_imageHistogram;

    if (!histogram)
        return;

    x   = 0; y  = 0;
    yr  = 0; yg = 0; yb = 0;
    max = 0.0;

    switch(m_channelType)
    {
       case HistogramWidget::GreenChannelHistogram:    // Green channel.
          max = histogram->getMaximum(ImageHistogram::GreenChannel);  
          break;

       case HistogramWidget::BlueChannelHistogram:     // Blue channel.
          max = histogram->getMaximum(ImageHistogram::BlueChannel);    
          break;

       case HistogramWidget::RedChannelHistogram:      // Red channel.
          max = histogram->getMaximum(ImageHistogram::RedChannel); 
          break;

       case HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
          max = histogram->getMaximum(ImageHistogram::AlphaChannel);  
          break;

       case HistogramWidget::ColorChannelsHistogram:   // All color channels.
          max = QMAX (QMAX (histogram->getMaximum(ImageHistogram::RedChannel),
                            histogram->getMaximum(ImageHistogram::GreenChannel)),
                      histogram->getMaximum(ImageHistogram::BlueChannel));  
          break;

       case HistogramWidget::ValueHistogram:           // Luminosity.
          max = histogram->getMaximum(ImageHistogram::ValueChannel); 
          break;
    }

    switch (m_scaleType)
    {
       case HistogramWidget::LinScaleHistogram:
          break;

       case HistogramWidget::LogScaleHistogram:
          if (max > 0.0)
              max = log (max);
          else
              max = 1.0;
          break;
    }

    // A QPixmap is used to enable the double buffering.

    QPixmap pm(size());
    QPainter p1;
    p1.begin(&pm, this);

    // Drawing selection or all histogram values.

    for (x = 0 ; x < wWidth ; x++)
    {
      double value = 0.0; 
      double value_r = 0.0, value_g = 0.0, value_b = 0.0; // For all color channels.
      int    i, j;
    
      i = (x * histogram->getHistogramSegment()) / wWidth;
      j = ((x + 1) * histogram->getHistogramSegment()) / wWidth;

      do
      {
          double v;
          double vr, vg, vb;                              // For all color channels.

          v  = 0.0;
          vr = 0.0; vg = 0.0; vb = 0.0;

          switch(m_channelType)
          {
             case HistogramWidget::GreenChannelHistogram:    // Green channel.
                v = histogram->getValue(ImageHistogram::GreenChannel, i++);   
                break;

             case HistogramWidget::BlueChannelHistogram:     // Blue channel.
                v = histogram->getValue(ImageHistogram::BlueChannel, i++);   
                break;

             case HistogramWidget::RedChannelHistogram:      // Red channel.
                v = histogram->getValue(ImageHistogram::RedChannel, i++);    
                break;

             case HistogramWidget::AlphaChannelHistogram:    // Alpha channel.
                v = histogram->getValue(ImageHistogram::AlphaChannel, i++);   
                break;

             case HistogramWidget::ColorChannelsHistogram:   // All color channels.
                vr = histogram->getValue(ImageHistogram::RedChannel, i++);   
                vg = histogram->getValue(ImageHistogram::GreenChannel, i);   
                vb = histogram->getValue(ImageHistogram::BlueChannel, i);   
                break;

             case HistogramWidget::ValueHistogram:           // Luminosity.
                v = histogram->getValue(ImageHistogram::ValueChannel, i++);   
                break;
          }

          if ( m_channelType != HistogramWidget::ColorChannelsHistogram )
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

      if ( m_channelType != HistogramWidget::ColorChannelsHistogram )
      {
         switch (m_scaleType)
         {
            case HistogramWidget::LinScaleHistogram:
              y = (int) ((wHeight * value) / max);
              break;

            case HistogramWidget::LogScaleHistogram:
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
            case HistogramWidget::LinScaleHistogram:
              yr = (int) ((wHeight * value_r) / max);
              yg = (int) ((wHeight * value_g) / max);
              yb = (int) ((wHeight * value_b) / max);
              break;

            case HistogramWidget::LogScaleHistogram:
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

      if ( m_channelType != HistogramWidget::ColorChannelsHistogram )
      {
         if ( d->selectMode == true )   // Selection mode enable ?
         {
            if ( x >= (int)(d->xmin * wWidth) && x <= (int)(d->xmax * wWidth) )
            {
               p1.setPen(QPen(Qt::black, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, 0);
               p1.setPen(QPen(Qt::lightGray, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, wHeight - y);
            }
            else 
            {
               p1.setPen(QPen(Qt::black, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, wHeight - y);
               p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight - y, x, 0);

               if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
               {
                  p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                  p1.drawLine(x, wHeight, x, 0);
               }
            }
         }
         else
         {
            p1.setPen(QPen(Qt::black, 1, Qt::SolidLine));
            p1.drawLine(x, wHeight, x, wHeight - y);
            p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
            p1.drawLine(x, wHeight - y, x, 0);

            if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
            {
               p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, 0);
            }
         }
      }
      else
      {
         if ( d->selectMode == true )   // Histogram selection mode enable ?
         {
             if ( x >= (int)(d->xmin * wWidth) && x <= (int)(d->xmax * wWidth) )
            {
               p1.setPen(QPen(Qt::black, 1, Qt::SolidLine));
               p1.drawLine(x, wHeight, x, 0);
               p1.setPen(QPen(Qt::lightGray, 1, Qt::SolidLine));

               // Witch color must be used on the foreground with all colors channel mode?
               switch (m_colorType) 
               {
                  case HistogramWidget::RedColor:
                    p1.drawLine(x, wHeight, x, wHeight - yr);
                    break;

                  case HistogramWidget::GreenColor:
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
                  case HistogramWidget::RedColor:
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);

                    p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);
                    p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);

                    if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                    {
                       p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                       p1.drawLine(x, wHeight, x, 0);
                    }

                    break;

                  case HistogramWidget::GreenColor:
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);

                    p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);
                    p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);

                    if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                    {
                       p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                       p1.drawLine(x, wHeight, x, 0);
                    }

                    break;

                  default:
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yr);
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yg);
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight, x, wHeight - yb);

                    p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);
                    p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                    p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);
                    p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);
                    p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);

                    if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                    {
                       p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                       p1.drawLine(x, wHeight, x, 0);
                    }

                    break;
               }
            }
         }
         else 
         {
            // Which color must be used on the foreground with all colors channel mode?
            switch (m_colorType) 
            {
               case HistogramWidget::RedColor:
                 p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);
                 p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);
                 p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);

                 p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);
                 p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);

                 if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                 {
                     p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                     p1.drawLine(x, wHeight, x, 0);
                 }

                 break;

               case HistogramWidget::GreenColor:
                 p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);
                 p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);
                 p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);

                 p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                 p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 

                 if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                 {
                     p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                     p1.drawLine(x, wHeight, x, 0);
                 }

                 break;

               default:
                 p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yr);                 
                 p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yg);                 
                 p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight, x, wHeight - yb);                 

                 p1.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - QMAX(QMAX(yr, yg), yb), x, 0);                 
                 p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                 p1.drawLine(x, wHeight - yr -1, x, wHeight - yr);                 
                 p1.drawLine(x, wHeight - yg -1, x, wHeight - yg);                 
                 p1.drawLine(x, wHeight - yb -1, x, wHeight - yb);                 

                 if ( x == wWidth/4 || x == wWidth/2 || x == 3*wWidth/4 )
                 {
                     p1.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
                     p1.drawLine(x, wHeight, x, 0);
                 }

                 break;
             }
          }
       }
    }

    // Drawing color guide.

    p1.setPen(QPen(Qt::red, 1, Qt::DotLine));
    int guidePos;

    if (d->guideVisible)
    {
       switch(m_channelType)
       {
          case HistogramWidget::RedChannelHistogram:
             guidePos = d->colorGuide.red();
             break;

          case HistogramWidget::GreenChannelHistogram:    
             guidePos = d->colorGuide.green();
             break;

          case HistogramWidget::BlueChannelHistogram:     
             guidePos = d->colorGuide.blue();
             break;

          case HistogramWidget::ValueHistogram:    
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
             p1.fillRect(rect, QBrush(QColor(250, 250, 255)) );
             p1.drawRect(rect);
             rect.moveLeft(xGuide+3);
             p1.drawText(rect, Qt::AlignLeft, string);
          }
          else
          {
             rect.moveRight(xGuide);
             p1.fillRect(rect, QBrush(QColor(250, 250, 255)) );
             p1.drawRect(rect);
             rect.moveRight(xGuide-3);
             p1.drawText(rect, Qt::AlignRight, string);
          }
       }
    }

    if (d->statisticsVisible)
    {
       QString tipText, value;
       QString cellBeg("<tr><td><nobr><font size=-1>");
       QString cellMid("</font></nobr></td><td><nobr><font size=-1>");
       QString cellEnd("</font></nobr></td></tr>");
       tipText  = "<table cellspacing=0 cellpadding=0>";

       tipText += cellBeg + i18n("Mean:") + cellMid;
       double mean = histogram->getMean(m_channelType, 0, histogram->getHistogramSegment()-1);
       tipText += value.setNum(mean, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Pixels:") + cellMid;
       double pixels = histogram->getPixels();
       tipText += value.setNum((float)pixels, 'f', 0) + cellEnd;

       tipText += cellBeg + i18n("Std dev.:") + cellMid;
       double stddev = histogram->getStdDev(m_channelType, 0, histogram->getHistogramSegment()-1);
       tipText += value.setNum(stddev, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Count:") + cellMid;
       double counts = histogram->getCount(m_channelType, 0, histogram->getHistogramSegment()-1);
       tipText += value.setNum((float)counts, 'f', 0) + cellEnd;

       tipText += cellBeg + i18n("Median:") + cellMid;
       double median = histogram->getMedian(m_channelType, 0, histogram->getHistogramSegment()-1);
       tipText += value.setNum(median, 'f', 1) + cellEnd;

       tipText += cellBeg + i18n("Percent:") + cellMid;
       double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
       tipText += value.setNum(percentile, 'f', 1) + cellEnd;

       tipText += "</table>";

       QToolTip::add( this, tipText);
    }

    // Drawing frame.

    p1.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    p1.drawRect(0, 0, width(), height());
    p1.end();
    bitBlt(this, 0, 0, &pm);
}

void HistogramWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted )
    {
       if (!d->inSelected)
       {
          d->inSelected = true;
          repaint(false);
       }

       d->xmin = ((double)e->pos().x()) / ((double)width());
       d->xminOrg = d->xmin;
       notifyValuesChanged();
       //emit signalValuesChanged( (int)(d->xmin * d->range),  );
       d->xmax = 0.0;
    }
}

void HistogramWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->selectMode == true  && d->clearFlag == HistogramWidgetPriv::HistogramCompleted ) 
    {
        d->inSelected = false;
        // Only single click without mouse move? Remove selection.
        if (d->xmax == 0.0)
        {
            d->xmin = 0.0;
            //emit signalMinValueChanged( 0 );
            //emit signalMaxValueChanged( d->range );
            notifyValuesChanged();
            repaint(false);
        }
    }
}

void HistogramWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted ) 
    {
       setCursor( KCursor::crossCursor() );

       if (d->inSelected)
       {
          double max = ((double)e->pos().x()) / ((double)width());
          //int max = (int)(e->pos().x()*((float)m_imageHistogram->getHistogramSegment()/(float)width()));

          if (max < d->xminOrg)
          {
             d->xmax = d->xminOrg;
             d->xmin = max;
             //emit signalMinValueChanged( (int)(d->xmin * d->range) );
          }
          else
          {
             d->xmin = d->xminOrg;
             d->xmax = max;
          }

          notifyValuesChanged();
          //emit signalMaxValueChanged( d->xmax == 0.0 ? d->range : (int)(d->xmax * d->range) );

          repaint(false);
       }
    }
}

void HistogramWidget::notifyValuesChanged()
{
    emit signalIntervalChanged( (int)(d->xmin * d->range), d->xmax == 0.0 ? d->range : (int)(d->xmax * d->range) );
}

void HistogramWidget::slotMinValueChanged( int min )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted )
    {
        if (min == 0 && d->xmax == 1.0)
        {
            // everything is selected means no selection
            d->xmin = 0.0;
            d->xmax = 0.0;
        }
        if (min >= 0 && min < d->range)
        {
           d->xmin = ((double)min)/d->range;
        }
        repaint(false);
    }
}

void HistogramWidget::slotMaxValueChanged( int max )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted ) 
    {
        if (d->xmin == 0.0 && max == d->range)
        {
            // everything is selected means no selection
            d->xmin = 0.0;
            d->xmax = 0.0;
        }
        else if (max > 0 && max <= d->range)
        {
            d->xmax = ((double)max)/d->range;
        }
        repaint(false);
    }
}

}  // namespace Digikam

#include "histogramwidget.moc"

