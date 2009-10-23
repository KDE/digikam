/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : a widget to draw histogram curves
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

#include "curveswidget.h"
#include "curveswidget.moc"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QEvent>
#include <QTimer>
#include <QRect>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QCustomEvent>
#include <QPaintEvent>
#include <QMouseEvent>

// KDE includes

#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes

#include "imagehistogram.h"
#include "imagecurves.h"
#include "debug.h"
#include "globals.h"

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

    CurvesWidgetPriv(CurvesWidget *q) : q(q)
    {
        curves        = 0;
        grabPoint     = -1;
        last          = 0;
        guideVisible  = false;
        xMouseOver    = -1;
        yMouseOver    = -1;
        clearFlag     = HistogramNone;
        progressCount = 0;
        progressTimer = 0;
        progressPix   = SmallIcon("process-working", 22);
    }

    bool         sixteenBits;
    bool         readOnlyMode;
    bool         guideVisible;

    int          clearFlag;          // Clear drawing zone with message.
    int          leftMost;
    int          rightMost;
    int          grabPoint;
    int          last;
    int          xMouseOver;
    int          yMouseOver;
    int          progressCount;      // Position of animation during loading/calculation.

    QTimer      *progressTimer;

    QPixmap      progressPix;

    DColor       colorGuide;

    ImageCurves *curves;             // Curves data instance.

    void renderLoadingAnimation()
    {

        QPixmap anim(progressPix.copy(0, progressCount*22, 22, 22));
        progressCount++;
        if (progressCount == 8)
        {
            progressCount = 0;
        }

        // ... and we render busy text.

        QPixmap pm(q->size());
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width()-1, q->height()-1);
        p1.drawPixmap(q->width()/2 - anim.width() /2, anim.height(), anim);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));

        if (clearFlag == CurvesWidgetPriv::HistogramDataLoading)
        {
            p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                        i18n("Loading image..."));
        }
        else
        {
            p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                        i18n("Histogram calculation..."));
        }

        p1.end();
        QPainter p3(q);
        p3.drawPixmap(0, 0, pm);
        p3.end();
    }

    void renderHistogramFailed()
    {
        QPixmap pm(q->size());
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width()-1, q->height()-1);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));
        p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                    i18n("Histogram\ncalculation\nfailed."));
        p1.end();
        QPainter p2(q);
        p2.drawPixmap(0, 0, pm);
        p2.end();
    }

    void renderHistogram(QPixmap & pm)
    {

         int    wWidth  = pm.width();
         int    wHeight = pm.height();
         ImageHistogram *histogram = q->m_imageHistogram;

         int x   = 0;
         int y   = 0;
         double max = 0.0;

         switch(q->m_channelType)
         {
             case GreenChannel:
               max = histogram->getMaximum(GreenChannel);
               break;

            case BlueChannel:
               max = histogram->getMaximum(BlueChannel);
               break;

            case RedChannel:
               max = histogram->getMaximum(RedChannel);
               break;

            case AlphaChannel:
               max = histogram->getMaximum(AlphaChannel);
               break;

            case LuminosityChannel:
               max = histogram->getMaximum(LuminosityChannel);
               break;
         }

         switch (q->m_scaleType)
         {
            case LinScaleHistogram:
               break;

            case LogScaleHistogram:
               if (max > 0.0)
                   max = log (max);
               else
                   max = 1.0;
               break;
         }

         // Drawing selection or all histogram values.
         // A QPixmap is used to enable  double buffering.

         QPainter p1;
         p1.begin(&pm);
         p1.initFrom(q);

         for (x = 0; x < wWidth; ++x)
         {
             double value = 0.0;

             int i = (x       * histogram->getHistogramSegments()) / wWidth;
             int j = ((x + 1) * histogram->getHistogramSegments()) / wWidth;

             do
             {
                 double v = 0.0;

                 switch(q->m_channelType)
                 {
                     case RedChannel:
                         v = histogram->getValue(RedChannel, i++);
                         break;

                     case GreenChannel:
                         v = histogram->getValue(GreenChannel, i++);
                         break;

                     case BlueChannel:
                         v = histogram->getValue(BlueChannel, i++);
                         break;

                     case AlphaChannel:
                         v = histogram->getValue(AlphaChannel, i++);
                         break;

                     case LuminosityChannel:
                         v = histogram->getValue(LuminosityChannel, i++);
                         break;
                 }

                 if (v > value)
                     value = v;
             }
             while (i < j);

             switch (q->m_scaleType)
             {
                 case LinScaleHistogram:
                     y = (int) ((wHeight * value) / max);
                     break;

                 case LogScaleHistogram:
                     if (value <= 0.0) value = 1.0;
                     y = (int) ((wHeight * log (value)) / max);
                     break;

                 default:
                     y = 0;
                     break;
             }

             // Drawing histogram

             p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
             p1.drawLine(x, wHeight, x, wHeight - y);
             p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
             p1.drawLine(x, wHeight - y, x, 0);
         }

    }

    void renderCurve(QPixmap & pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth = pm.width();
        int wHeight = pm.height();

        // Drawing curves.
        int curvePrevVal = 0;
        for (int x = 0 ; x < wWidth; ++x)
        {

            // TODO duplicate code...
            int i = (x * q->m_imageHistogram->getHistogramSegments()) / wWidth;

            int curveVal = curves->getCurveValue(q->m_channelType, i);

            p1.save();
            p1.setRenderHint(QPainter::Antialiasing);
            p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Link), 2, Qt::SolidLine));
            p1.drawLine(x - 1, wHeight - ((curvePrevVal * wHeight) / q->m_imageHistogram->getHistogramSegments()),
                        x,     wHeight - ((curveVal * wHeight) / q->m_imageHistogram->getHistogramSegments()));

            p1.restore();
            curvePrevVal = curveVal;
        }

        // Drawing curves points.
        if (!readOnlyMode && curves->getCurveType(q->m_channelType) == ImageCurves::CURVE_SMOOTH)
        {

            p1.save();
            p1.setPen(QPen(Qt::red, 3, Qt::SolidLine));
            p1.setRenderHint(QPainter::Antialiasing);

            for (int p = 0 ; p < ImageCurves::NUM_POINTS ; ++p)
            {
                QPoint curvePoint = curves->getCurvePoint(q->m_channelType, p);

                if (curvePoint.x() >= 0)
                {
                    p1.drawEllipse( ((curvePoint.x() * wWidth) / q->m_imageHistogram->getHistogramSegments()) - 2,
                                 wHeight - 2 - ((curvePoint.y() * wHeight) / q->m_imageHistogram->getHistogramSegments()),
                                 4, 4 );
                }
            }
            p1.restore();
        }

    }

    void renderGrid(QPixmap & pm)
    {

        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth = pm.width();
        int wHeight = pm.height();

        // Drawing black/middle/highlight tone grid separators.
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Base), 1, Qt::SolidLine));
        p1.drawLine(wWidth/4, 0, wWidth/4, wHeight);
        p1.drawLine(wWidth/2, 0, wWidth/2, wHeight);
        p1.drawLine(3*wWidth/4, 0, 3*wWidth/4, wHeight);
        p1.drawLine(0, wHeight/4, wWidth, wHeight/4);
        p1.drawLine(0, wHeight/2, wWidth, wHeight/2);
        p1.drawLine(0, 3*wHeight/4, wWidth, 3*wHeight/4);

    }

    void renderMousePosition(QPixmap & pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth = pm.width();
        int wHeight = pm.height();

        // Drawing X,Y point position dragged by mouse over widget.
        p1.setPen(QPen(Qt::red, 1, Qt::DotLine));

        if (xMouseOver != -1 && yMouseOver != -1)
        {
            QString string = i18n("x:%1\ny:%2",xMouseOver,yMouseOver);
            QFontMetrics fontMt(string);
            QRect rect = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string);
            rect.moveRight(wWidth);
            rect.moveBottom(wHeight);
            p1.drawText(rect, Qt::AlignLeft||Qt::AlignTop, string);
        }
    }

    void renderColorGuide(QPixmap & pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        // Drawing color guide.
        int wWidth = pm.width();
        int wHeight = pm.height();

        if (guideVisible)
        {
            int guidePos;
            switch(q->m_channelType)
            {
                case RedChannel:
                    guidePos = colorGuide.red();
                    break;

                case GreenChannel:
                    guidePos = colorGuide.green();
                    break;

                case BlueChannel:
                    guidePos = colorGuide.blue();
                    break;

                case LuminosityChannel:
                    guidePos = qMax(qMax(colorGuide.red(), colorGuide.green()), colorGuide.blue());
                    break;

                default:                  // Alpha.
                    guidePos = -1;
                    break;
            }

            if (guidePos != -1)
            {
                int xGuide = (guidePos * wWidth) / q->m_imageHistogram->getHistogramSegments();
                p1.drawLine(xGuide, 0, xGuide, wHeight);

                QString string = i18n("x:%1",guidePos);
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
    }

    void renderFrame(QPixmap & pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.setPen(QPen(q->palette().color(QPalette::Active,
                        QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, pm.width() - 1, pm.height() - 1);
    }

    static QString getChannelPattern(QString prefix)
    {
        return prefix + "Channel%1Type";
    }

    static QString getPointPattern(QString prefix)
    {
        return prefix + "Channel%1Point%2";
    }

private:
    CurvesWidget *q;

};

CurvesWidget::CurvesWidget(int w, int h, QWidget *parent, bool readOnly)
            : QWidget(parent), d(new CurvesWidgetPriv(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setup(w, h, readOnly);
}

CurvesWidget::CurvesWidget(int w, int h,
                           uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits,
                           QWidget *parent, bool readOnly)
            : QWidget(parent), d(new CurvesWidgetPriv(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setup(w, h, readOnly);
    updateData(i_data, i_w, i_h, i_sixteenBits);
}

CurvesWidget::~CurvesWidget()
{
    d->progressTimer->stop();

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
    m_channelType    = LuminosityChannel;
    m_scaleType      = LogScaleHistogram;
    m_imageHistogram = 0;

    setMouseTracking(true);
    setMinimumSize(w, h);

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

void CurvesWidget::saveCurve(KConfigGroup & group, QString prefix)
{

    kDebug(digiKamAreaCode) << "Storing curves";

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        group.writeEntry(CurvesWidgetPriv::getChannelPattern(prefix).arg(
                        channel), (int) curves()->getCurveType(channel));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = curves()->getCurvePoint(channel, point);
            if (!curves()->isSixteenBits() && p
                            != ImageCurves::getDisabledValue())
            {
                // Store point as 16 bits depth.
                p.setX(p.x() * ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() * ImageCurves::MULTIPLIER_16BIT);
            }

            group.writeEntry(CurvesWidgetPriv::getPointPattern(prefix).arg(
                            channel, point), p);
        }

    }

}

void CurvesWidget::restoreCurve(KConfigGroup & group, QString prefix)
{

    kDebug(digiKamAreaCode) << "Restoring curves";

    reset();

    kDebug(digiKamAreaCode) << "curves " << curves() << " isSixteenBits = "
                    << curves()->isSixteenBits();

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        curves()->setCurveType(
                        channel,
                        (ImageCurves::CurveType) group.readEntry(
                                        CurvesWidgetPriv::getChannelPattern(
                                                        prefix).arg(channel), 0));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = group.readEntry(
                            CurvesWidgetPriv::getPointPattern(prefix).arg(
                                            channel, point),
                            ImageCurves::getDisabledValue());

            // always load a 16 bit curve and stretch it to 8 bit if necessary
            if (!curves()->isSixteenBits() && p
                            != ImageCurves::getDisabledValue())
            {
                p.setX(p.x() / ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() / ImageCurves::MULTIPLIER_16BIT);
            }

            curves()->setCurvePoint(channel, point, p);
        }

        curves()->curvesCalculateCurve(channel);

    }

}

void CurvesWidget::updateData(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits)
{

    kDebug(digiKamAreaCode) << "updating data";

    stopHistogramComputation();

    d->sixteenBits = i_sixteenBits;

    // Remove old histogram data from memory.
    if (m_imageHistogram)
        delete m_imageHistogram;

    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits);

    connect(m_imageHistogram, SIGNAL(calculationStarted(const ImageHistogram *)),
            this, SLOT(slotCalculationStarted(const ImageHistogram *)));

    connect(m_imageHistogram, SIGNAL(calculationFinished(const ImageHistogram *, bool)),
            this, SLOT(slotCalculationFinished(const ImageHistogram *, bool)));

    m_imageHistogram->calculateInThread();

    // keep the old curve
    ImageCurves *newCurves = new ImageCurves(i_sixteenBits);
    newCurves->setCurveType(ImageCurves::CURVE_SMOOTH);
    if (d->curves)
    {
        newCurves->fillFromOtherCurvers(d->curves);
        delete d->curves;
    }
    d->curves = newCurves;

    resetUI();
}

void CurvesWidget::reset()
{
    if (d->curves)
        d->curves->curvesReset();

    resetUI();
}

void CurvesWidget::resetUI()
{
    d->grabPoint    = -1;
    d->guideVisible = false;
    repaint();
}

ImageCurves* CurvesWidget::curves() const
{
    return d->curves;
}

void CurvesWidget::setDataLoading()
{
    if (d->clearFlag != CurvesWidgetPriv::HistogramDataLoading)
    {
        setCursor(Qt::WaitCursor);
        d->clearFlag     = CurvesWidgetPriv::HistogramDataLoading;
        d->progressCount = 0;
        d->progressTimer->start(100);
    }
}

void CurvesWidget::setLoadingFailed()
{
    d->clearFlag     = CurvesWidgetPriv::HistogramFailed;
    d->progressCount = 0;
    d->progressTimer->stop();
    repaint();
    setCursor(Qt::ArrowCursor);
}

void CurvesWidget::setCurveGuide(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    repaint();
}

void CurvesWidget::curveTypeChanged()
{
    switch (d->curves->getCurveType(m_channelType))
    {
        case ImageCurves::CURVE_SMOOTH:

            //  pick representative points from the curve and make them control points

            for (int i = 0; i <= 8; ++i)
            {
                int index = CLAMP(i * m_imageHistogram->getHistogramSegments()/8,
                                  0, m_imageHistogram->getHistogramSegments()-1);

                d->curves->setCurvePoint(m_channelType, i * 2,
                                         QPoint(index, d->curves->getCurveValue(m_channelType, index)));
            }

            d->curves->curvesCalculateCurve(m_channelType);
            break;

        case ImageCurves::CURVE_FREE:
            break;
    }

    repaint();
    emit signalCurvesChanged();
}

void CurvesWidget::slotCalculationStarted(const ImageHistogram*)
{
    setCursor(Qt::WaitCursor);
    d->clearFlag = CurvesWidgetPriv::HistogramStarted;
    d->progressTimer->start(200);
    repaint();
}

void CurvesWidget::slotCalculationFinished(const ImageHistogram*, bool success)
{
    if (success)
    {
        // Repaint histogram
        d->clearFlag = CurvesWidgetPriv::HistogramCompleted;
        d->progressTimer->stop();
        repaint();
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        d->clearFlag = CurvesWidgetPriv::HistogramFailed;
        d->progressTimer->stop();
        repaint();
        setCursor(Qt::ArrowCursor);
        emit signalHistogramComputationFailed();
    }
}

void CurvesWidget::stopHistogramComputation()
{
    if (m_imageHistogram)
       m_imageHistogram->stopCalculation();

    d->progressTimer->stop();
    d->progressCount = 0;
}

void CurvesWidget::slotProgressTimerDone()
{
    repaint();
    d->progressTimer->start(200);
}

void CurvesWidget::paintEvent(QPaintEvent*)
{

    // special cases

    if (d->clearFlag == CurvesWidgetPriv::HistogramDataLoading ||
        d->clearFlag == CurvesWidgetPriv::HistogramStarted)
    {
        d->renderLoadingAnimation();
        return;
    }
    else if (d->clearFlag == CurvesWidgetPriv::HistogramFailed)
    {
        d->renderHistogramFailed();
        return;
    }

    // normal case, histogram present

    if (!m_imageHistogram)
    {
        kWarning(digiKamAreaCode)
                        << "Should render a histogram, but did not get one.";
        return;
    }

    // render subelements on a pixmap
    QPixmap pm(size());

    d->renderHistogram(pm);
    d->renderCurve(pm);
    d->renderGrid(pm);
    d->renderMousePosition(pm);
    d->renderColorGuide(pm);
    d->renderFrame(pm);

    // render pixmap on widget
    QPainter p2(this);
    p2.drawPixmap(0, 0, pm);
    p2.end();

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
                       ((float)(m_imageHistogram->getHistogramSegments()-1) / (float)width())),
                    0, m_imageHistogram->getHistogramSegments()-1 );
    int y = CLAMP((int)(e->pos().y() *
                       ((float)(m_imageHistogram->getHistogramSegments()-1) / (float)height())),
                    0, m_imageHistogram->getHistogramSegments()-1 );

    distance = ImageCurves::NUM_VALUES_16BIT;

    for (i = 0, closest_point = 0 ; i < ImageCurves::NUM_POINTS ; ++i)
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

    int delta = m_imageHistogram->getHistogramSegments()/16;
    if (distance > 8)
        closest_point = (x + delta/2) / delta;

    setCursor(Qt::CrossCursor);

    switch(d->curves->getCurveType(m_channelType))
    {
        case ImageCurves::CURVE_SMOOTH:
        {
            // Determine the leftmost and rightmost points.

            d->leftMost = -1;

            for (i = closest_point - 1 ; i >= 0 ; --i)
            {
                if (d->curves->getCurvePointX(m_channelType, i) != -1)
                {
                    d->leftMost = d->curves->getCurvePointX(m_channelType, i);
                    break;
                }
            }

            d->rightMost = m_imageHistogram->getHistogramSegments();

            for (i = closest_point + 1 ; i < ImageCurves::NUM_POINTS ; ++i)
            {
                if (d->curves->getCurvePointX(m_channelType, i) != -1)
                {
                    d->rightMost = d->curves->getCurvePointX(m_channelType, i);
                    break;
                }
            }

            d->grabPoint = closest_point;
            d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                     QPoint(x, m_imageHistogram->getHistogramSegments() - y));

            break;
        }

        case ImageCurves::CURVE_FREE:
        {

            d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegments() - y);
            d->grabPoint = x;
            d->last      = y;
            break;
        }
    }

    d->curves->curvesCalculateCurve(m_channelType);
    repaint();
}

void CurvesWidget::mouseReleaseEvent( QMouseEvent * e )
{
    if (d->readOnlyMode || !m_imageHistogram) return;

    if (e->button() != Qt::LeftButton || d->clearFlag == CurvesWidgetPriv::HistogramStarted)
        return;

    setCursor(Qt::ArrowCursor);
    d->grabPoint = -1;
    d->curves->curvesCalculateCurve(m_channelType);
    repaint();
    emit signalCurvesChanged();
}

void CurvesWidget::mouseMoveEvent(QMouseEvent *e)
{
   if (d->readOnlyMode || !m_imageHistogram) return;

   // FIXME code duplication from mousePressEvent

   int i;
   int closest_point;
   int x1, x2, y1, y2;
   int distance;

   if (d->clearFlag == CurvesWidgetPriv::HistogramStarted)
      return;

   int x = CLAMP( (int)(e->pos().x()*((float)(m_imageHistogram->getHistogramSegments()-1)/(float)width())),
                  0, m_imageHistogram->getHistogramSegments()-1 );
   int y = CLAMP( (int)(e->pos().y()*((float)(m_imageHistogram->getHistogramSegments()-1)/(float)height())),
                  0, m_imageHistogram->getHistogramSegments()-1 );

   distance = ImageCurves::NUM_VALUES_16BIT;

   for (i = 0, closest_point = 0 ; i < ImageCurves::NUM_POINTS ; ++i)
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

   int delta = m_imageHistogram->getHistogramSegments()/16;
   if (distance > 8)
      closest_point = (x + delta/2) / delta;

   switch ( d->curves->getCurveType(m_channelType) )
   {
      case ImageCurves::CURVE_SMOOTH:
      {
         if (d->grabPoint == -1)   // If no point is grabbed...
         {
            if ( d->curves->getCurvePointX(m_channelType, closest_point) != -1 )
               setCursor(Qt::ArrowCursor);
            else
               setCursor(Qt::CrossCursor);
         }
         else                      // Else, drag the grabbed point
         {
            setCursor(Qt::CrossCursor);

            d->curves->setCurvePointX(m_channelType, d->grabPoint, -1);

            if (x > d->leftMost && x < d->rightMost)
            {
               closest_point = (x + delta/2) / delta;

               if (d->curves->getCurvePointX(m_channelType, closest_point) == -1)
                  d->grabPoint = closest_point;

               d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                        QPoint(x, m_imageHistogram->getHistogramSegments()-1 - y));
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
              for (i = x1 ; i <= x2 ; ++i)
                 d->curves->setCurveValue(m_channelType, i,
                    m_imageHistogram->getHistogramSegments()-1 - (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
           }
           else
           {
              d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegments()-1 - y);
           }

           d->grabPoint = x;
           d->last      = y;
         }

         emit signalCurvesChanged();

         break;
      }
   }

   d->xMouseOver = x;
   d->yMouseOver = m_imageHistogram->getHistogramSegments()-1 - y;
   emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
   repaint();
}

void CurvesWidget::leaveEvent(QEvent*)
{
   d->xMouseOver = -1;
   d->yMouseOver = -1;
   emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
   repaint();
}

}  // namespace Digikam
