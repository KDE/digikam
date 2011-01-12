/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-21
 * Description : a widget to display an image histogram.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "histogramwidget.moc"

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QEvent>
#include <QTimer>
#include <QColor>
#include <QBrush>
#include <QRect>
#include <QFont>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QMouseEvent>

// KDE includes


#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpixmapsequence.h>

// Local includes

#include "ditemtooltip.h"
#include "imagehistogram.h"
#include "globals.h"
#include "histogrampainter.h"

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
        renderingType        = FullImageHistogram;
        sixteenBits          = false;
        inSelected           = false;
        clearFlag            = HistogramNone;
        xmin                 = 0.0;
        xmax                 = 0.0;
        range                = 255;
        guideVisible         = false;
        inInitialRepaintWait = false;
        progressCount        = 0;
        progressTimer        = 0;
        progressPix          = KPixmapSequence("process-working", KIconLoader::SizeSmallMedium);
    }

    bool    inInitialRepaintWait;
    bool    sixteenBits;
    bool    guideVisible;       // Display color guide.
    bool    statisticsVisible;  // Display tooltip histogram statistics.
    bool    inSelected;
    bool    selectMode;         // If true, a part of the histogram can be selected !
    bool    showProgress;       // If true, a message will be displayed during histogram computation,
    // else nothing (limit flicker effect in widget especially for small
    // image/computation time).

    int     renderingType;      // Using full image or image selection for histogram rendering.
    int     range;
    int     clearFlag;          // Clear drawing zone with message.
    int     progressCount;      // Position of animation during loading/calculation.
    ChannelType channelType;    // Channel type to draw
    HistogramScale scaleType;   // Scale to use for drawing
    ImageHistogram* imageHistogram;      // Full image
    ImageHistogram* selectionHistogram;  // Image selection

    // Current selection information.
    double  xmin;
    double  xminOrg;
    double  xmax;

    QTimer* progressTimer;

    KPixmapSequence progressPix;

    DColor  colorGuide;

    HistogramPainter* histogramPainter;

};

// Constructor without image data (needed to use updateData() method after instance created).

HistogramWidget::HistogramWidget(int w, int h,
                                 QWidget* parent, bool selectMode,
                                 bool showProgress, bool statisticsVisible)
    : QWidget(parent), d(new HistogramWidgetPriv)
{
    setup(w, h, selectMode, showProgress, statisticsVisible);
    setAttribute(Qt::WA_DeleteOnClose);

    d->imageHistogram     = 0;
    d->selectionHistogram = 0;
    d->histogramPainter  = new HistogramPainter(this);
}

// Constructor without image selection.

HistogramWidget::HistogramWidget(int w, int h,
                                 uchar* i_data, uint i_w, uint i_h,
                                 bool i_sixteenBits,
                                 QWidget* parent, bool selectMode,
                                 bool showProgress, bool statisticsVisible)
    : QWidget(parent), d(new HistogramWidgetPriv)
{
    d->sixteenBits = i_sixteenBits;
    setup(w, h, selectMode, showProgress, statisticsVisible);
    setAttribute(Qt::WA_DeleteOnClose);

    d->imageHistogram     = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits);
    d->selectionHistogram = 0L;
    d->histogramPainter  = new HistogramPainter(this);

    connectHistogram(d->imageHistogram);

    d->imageHistogram->calculateInThread();
}

// Constructor with image selection.

HistogramWidget::HistogramWidget(int w, int h,
                                 uchar* i_data, uint i_w, uint i_h,
                                 uchar* s_data, uint s_w, uint s_h,
                                 bool i_sixteenBits,
                                 QWidget* parent, bool selectMode,
                                 bool showProgress, bool statisticsVisible)
    : QWidget(parent), d(new HistogramWidgetPriv)
{
    d->sixteenBits = i_sixteenBits;
    setup(w, h, selectMode, showProgress, statisticsVisible);
    setAttribute(Qt::WA_DeleteOnClose);

    d->imageHistogram     = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits);
    d->selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits);
    d->histogramPainter  = new HistogramPainter(this);

    connectHistogram(d->imageHistogram);
    connectHistogram(d->selectionHistogram);

    d->imageHistogram->calculateInThread();
}

HistogramWidget::~HistogramWidget()
{
    d->progressTimer->stop();

    delete d->imageHistogram;
    delete d->selectionHistogram;

    delete d;
}

void HistogramWidget::setup(int w, int h, bool selectMode, bool showProgress, bool statisticsVisible)
{
    d->channelType       = LuminosityChannel;
    d->scaleType         = LogScaleHistogram;
    d->statisticsVisible = statisticsVisible;
    d->selectMode        = selectMode;
    d->showProgress      = showProgress;

    setMouseTracking(true);
    setMinimumSize(w, h);

    d->progressTimer = new QTimer( this );

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

void HistogramWidget::connectHistogram(const ImageHistogram* histogram)
{
    connect(histogram, SIGNAL(calculationStarted(const ImageHistogram*)),
            this, SLOT(slotCalculationStarted(const ImageHistogram*)));

    connect(histogram, SIGNAL(calculationFinished(const ImageHistogram*, bool)),
            this, SLOT(slotCalculationFinished(const ImageHistogram*, bool)));
}

void HistogramWidget::setHistogramGuideByColor(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    repaint();
}

void HistogramWidget::setRenderingType(HistogramRenderingType type)
{
    if (type != d->renderingType)
    {
        d->renderingType = type;

        ImageHistogram* nowUsedHistogram;

        if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
        {
            nowUsedHistogram = d->selectionHistogram;
        }
        else
        {
            nowUsedHistogram = d->imageHistogram;
        }

        // already calculated?
        if (!nowUsedHistogram->isValid())
        {
            // still computing, or need to start it?
            if (nowUsedHistogram->isCalculating())
            {
                slotCalculationStarted(nowUsedHistogram);
            }
            else
            {
                nowUsedHistogram->calculateInThread();
            }
        }
        else
        {
            update();
        }
    }
}

ImageHistogram* HistogramWidget::currentHistogram()
{
    if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
    {
        return d->selectionHistogram;
    }
    else
    {
        return d->imageHistogram;
    }
}

void HistogramWidget::reset()
{
    d->histogramPainter->disableHistogramGuide();
    repaint();
}

void HistogramWidget::slotCalculationStarted(const ImageHistogram* histogram)
{
    if (histogram != d->imageHistogram && histogram != d->selectionHistogram)
    {
        return;
    }

    // only react to the histogram that the user is currently waiting for
    if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
    {
        if (histogram == d->imageHistogram)
        {
            return;
        }
    }
    else
    {
        if (histogram == d->selectionHistogram)
        {
            return;
        }
    }

    setCursor( Qt::WaitCursor );
    d->clearFlag = HistogramWidgetPriv::HistogramStarted;

    if (!d->inInitialRepaintWait)
    {
        if (d->clearFlag != HistogramWidgetPriv::HistogramDataLoading)
        {
            // enter initial repaint wait, repaint only after waiting
            // a short time so that very fast computation does not create flicker
            d->inInitialRepaintWait = true;
            d->progressTimer->start( 100 );
        }
        else
        {
            // For data loading, the initial wait has been set in setDataLoading.
            // After the initial repaint, we can repaint immediately.
            repaint();
            d->progressTimer->start( 200 );
        }
    }
}

void HistogramWidget::slotCalculationFinished(const ImageHistogram* histogram, bool success)
{
    if (histogram != d->imageHistogram && histogram != d->selectionHistogram)
    {
        return;
    }

    if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
    {
        if (histogram == d->imageHistogram)
        {
            return;
        }
    }
    else
    {
        if (histogram == d->selectionHistogram)
        {
            return;
        }
    }

    if (success)
    {
        // Repaint histogram
        d->clearFlag = HistogramWidgetPriv::HistogramCompleted;
        d->progressTimer->stop();
        d->inInitialRepaintWait = false;
        setCursor( Qt::ArrowCursor );

        // Send signals to refresh information if necessary.
        // The signals may trigger multiple repaints, avoid this,
        // we repaint once afterwards.
        setUpdatesEnabled(false);

        notifyValuesChanged();
        emit signalHistogramComputationDone(d->sixteenBits);

        setUpdatesEnabled(true);
        repaint();
    }
    else
    {
        d->clearFlag = HistogramWidgetPriv::HistogramFailed;
        d->progressTimer->stop();
        d->inInitialRepaintWait = false;
        repaint();
        setCursor( Qt::ArrowCursor );

        // Remove old histogram data from memory.
        delete d->imageHistogram;
        d->imageHistogram = 0;
        delete d->selectionHistogram;
        d->selectionHistogram = 0;
        emit signalHistogramComputationFailed();
    }
}

void HistogramWidget::setDataLoading()
{
    if (d->clearFlag != HistogramWidgetPriv::HistogramDataLoading)
    {
        setCursor( Qt::WaitCursor );
        d->clearFlag = HistogramWidgetPriv::HistogramDataLoading;
        // enter initial repaint wait, repaint only after waiting
        // a short time so that very fast computation does not create flicker
        d->inInitialRepaintWait = true;
        d->progressCount        = 0;
        d->progressTimer->start( 100 );
    }
}

void HistogramWidget::setLoadingFailed()
{
    d->clearFlag     = HistogramWidgetPriv::HistogramFailed;
    d->progressCount = 0;
    d->progressTimer->stop();
    d->inInitialRepaintWait = false;
    repaint();
    setCursor( Qt::ArrowCursor );
}

void HistogramWidget::stopHistogramComputation()
{
    if (d->imageHistogram)
    {
        d->imageHistogram->stopCalculation();
    }

    if (d->selectionHistogram)
    {
        d->selectionHistogram->stopCalculation();
    }

    d->progressTimer->stop();
    d->progressCount = 0;
}

void HistogramWidget::updateData(uchar* i_data, uint i_w, uint i_h,
                                 bool i_sixteenBits,
                                 uchar* s_data, uint s_w, uint s_h,
                                 bool showProgress)
{
    d->showProgress = showProgress;
    d->sixteenBits  = i_sixteenBits;

    // We are deleting the histogram data, so we must not use it to draw any more.
    d->clearFlag = HistogramWidgetPriv::HistogramNone;

    // Do not using ImageHistogram::getHistogramSegments()
    // method here because histogram hasn't yet been computed.
    d->range = d->sixteenBits ? MAX_SEGMENT_16BIT : MAX_SEGMENT_8BIT;
    emit signalMaximumValueChanged( d->range );


    // Remove old histogram data from memory.
    if (d->imageHistogram)
    {
        delete d->imageHistogram;
    }

    if (d->selectionHistogram)
    {
        delete d->selectionHistogram;
    }

    // Calc new histogram data
    d->imageHistogram = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits);
    connectHistogram(d->imageHistogram);

    if (s_data && s_w && s_h)
    {
        d->selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits);
        connectHistogram(d->selectionHistogram);
    }
    else
    {
        d->selectionHistogram = 0L;
    }

    if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
    {
        d->selectionHistogram->calculateInThread();
    }
    else
    {
        d->imageHistogram->calculateInThread();
    }
}

void HistogramWidget::updateSelectionData(uchar* s_data, uint s_w, uint s_h,
        bool i_sixteenBits,
        bool showProgress)
{
    d->showProgress = showProgress;

    // Remove old histogram data from memory.

    delete d->selectionHistogram;
    // Calc new histogram data
    d->selectionHistogram = new ImageHistogram(s_data, s_w, s_h, i_sixteenBits);
    connectHistogram(d->selectionHistogram);

    if (d->renderingType == ImageSelectionHistogram)
    {
        d->selectionHistogram->calculateInThread();
    }
}

void HistogramWidget::slotProgressTimerDone()
{
    d->inInitialRepaintWait = false;
    repaint();
    d->progressTimer->start( 200 );
}

void HistogramWidget::paintEvent(QPaintEvent*)
{
    // Widget is disabled, not initialized,
    // or loading, but no message shall be drawn:
    // Drawing grayed frame.
    if ( !isEnabled() ||
         d->clearFlag == HistogramWidgetPriv::HistogramNone ||
         (!d->showProgress && (d->clearFlag == HistogramWidgetPriv::HistogramStarted ||
                               d->clearFlag == HistogramWidgetPriv::HistogramDataLoading))
       )
    {
        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Disabled, QPalette::Background));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width()-1, height()-1);
        QPen pen(palette().color(QPalette::Disabled, QPalette::Foreground));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);

        p1.setPen(pen);
        p1.drawRect(0, 0, width(), height());
        p1.end();

        return;
    }
    // Image data is loading or histogram is being computed:
    // Draw message.
    else if (  d->showProgress &&
               (d->clearFlag == HistogramWidgetPriv::HistogramStarted ||
                d->clearFlag == HistogramWidgetPriv::HistogramDataLoading)
            )
    {
        // In first, we draw an animation.

        QPixmap anim(d->progressPix.frameAt(d->progressCount));
        d->progressCount++;

        if (d->progressCount >= d->progressPix.frameCount())
        {
            d->progressCount = 0;
        }

        // ... and we render busy text.

        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width()-1, height()-1);
        p1.drawPixmap(width()/2 - anim.width() /2, anim.height(), anim);
        p1.setPen(palette().color(QPalette::Active, QPalette::Text));

        if (d->clearFlag == HistogramWidgetPriv::HistogramDataLoading)
            p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                        i18n("Loading image..."));
        else
            p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                        i18n("Histogram calculation..."));

        p1.end();

        return;
    }
    // Histogram computation failed:
    // Draw message.
    else if (d->clearFlag == HistogramWidgetPriv::HistogramFailed)
    {
        QPainter p1(this);
        p1.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, width()-1, height()-1);
        p1.setPen(palette().color(QPalette::Active, QPalette::Text));
        p1.drawText(0, 0, width(), height(), Qt::AlignCenter,
                    i18n("Histogram\ncalculation\nfailed."));
        p1.end();

        return;
    }

    // render histogram in normal case
    ImageHistogram* histogram = 0;

    if (d->renderingType == ImageSelectionHistogram && d->selectionHistogram)
    {
        histogram = d->selectionHistogram;
    }
    else
    {
        histogram = d->imageHistogram;
    }

    if (!histogram)
    {
        return;
    }

    d->histogramPainter->setHistogram(histogram);

    d->histogramPainter->setChannelType(d->channelType);
    d->histogramPainter->setScale(d->scaleType);
    d->histogramPainter->setSelection(d->xmin, d->xmax);
    d->histogramPainter->setHighlightSelection(d->selectMode);

    if (d->guideVisible == true)
    {
        d->histogramPainter->enableHistogramGuideByColor(d->colorGuide);
    }
    else
    {
        d->histogramPainter->disableHistogramGuide();
    }

    // A QPixmap is used to enable the double buffering.
    QPixmap bufferPixmap(size());

    d->histogramPainter->initFrom(this);
    d->histogramPainter->render(bufferPixmap);

    // render the pixmap on the widget
    QPainter p2(this);
    p2.drawPixmap(0, 0, bufferPixmap);
    p2.end();

    // render statistics if needed
    if (d->statisticsVisible)
    {
        DToolTipStyleSheet cnt;
        QString            tipText, value;
        tipText = "<qt><table cellspacing=0 cellpadding=0>";

        tipText += cnt.cellBeg + i18n("Mean:") + cnt.cellMid;
        double mean = histogram->getMean(d->channelType, 0, histogram->getHistogramSegments()-1);
        tipText += value.setNum(mean, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18n("Pixels:") + cnt.cellMid;
        double pixels = histogram->getPixels();
        tipText += value.setNum((float)pixels, 'f', 0) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18n("Std dev.:") + cnt.cellMid;
        double stddev = histogram->getStdDev(d->channelType, 0, histogram->getHistogramSegments()-1);
        tipText += value.setNum(stddev, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18n("Count:") + cnt.cellMid;
        double counts = histogram->getCount(d->channelType, 0, histogram->getHistogramSegments()-1);
        tipText += value.setNum((float)counts, 'f', 0) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18n("Median:") + cnt.cellMid;
        double median = histogram->getMedian(d->channelType, 0, histogram->getHistogramSegments()-1);
        tipText += value.setNum(median, 'f', 1) + cnt.cellEnd;

        tipText += cnt.cellBeg + i18n("Percent:") + cnt.cellMid;
        double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
        tipText += value.setNum(percentile, 'f', 1) + cnt.cellEnd;

        tipText += "</table></qt>";

        this->setToolTip(tipText);
    }

}

void HistogramWidget::mousePressEvent ( QMouseEvent* e )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted )
    {
        if (!d->inSelected)
        {
            d->inSelected = true;
            repaint();
        }

        d->xmin = ((double)e->pos().x()) / ((double)width());
        d->xminOrg = d->xmin;
        notifyValuesChanged();
        //emit signalValuesChanged( (int)(d->xmin * d->range),  );
        d->xmax = 0.0;
    }
}

void HistogramWidget::mouseReleaseEvent ( QMouseEvent* )
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
            repaint();
        }
    }
}

void HistogramWidget::mouseMoveEvent ( QMouseEvent* e )
{
    if ( d->selectMode == true && d->clearFlag == HistogramWidgetPriv::HistogramCompleted )
    {
        setCursor( Qt::CrossCursor );

        if (d->inSelected)
        {
            double max = ((double)e->pos().x()) / ((double)width());
            //int max = (int)(e->pos().x()*((float)m_imageHistogram->getHistogramSegments()/(float)width()));

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

            repaint();
        }
    }
}

void HistogramWidget::notifyValuesChanged()
{
    emit signalIntervalChanged( (int)(d->xmin * d->range), d->xmax == 0.0 ? d->range : (int)(d->xmax * d->range) );
}

void HistogramWidget::slotMinValueChanged(int min)
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

        repaint();
    }
}

void HistogramWidget::slotMaxValueChanged(int max)
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

        repaint();
    }
}

void HistogramWidget::setChannelType(ChannelType channel)
{
    d->channelType = channel;
    update();
}

void HistogramWidget::setScaleType(HistogramScale scale)
{
    d->scaleType = scale;
    update();
}

}  // namespace Digikam
