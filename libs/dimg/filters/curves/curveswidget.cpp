/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : a widget to draw histogram curves
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "curveswidget.h"

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
#include <QPaintEvent>
#include <QMouseEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "histogrampainter.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class CurvesWidget::Private
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

    explicit Private(CurvesWidget* const q)
        : q(q)
    {
        readOnlyMode     = false;
        leftMost         = 0;
        rightMost        = 0;
        channelType      = LuminosityChannel;
        scaleType        = LinScaleHistogram;
        imageHistogram   = 0;
        histogramPainter = 0;
        curves           = 0;
        grabPoint        = -1;
        last             = 0;
        guideVisible     = false;
        xMouseOver       = -1;
        yMouseOver       = -1;
        clearFlag        = HistogramNone;
        progressCount    = 0;
        progressTimer    = 0;
        progressPix      = DWorkingPixmap();
    }

    bool                           readOnlyMode;
    bool                           guideVisible;

    int                            clearFlag;          // Clear drawing zone with message.
    int                            leftMost;
    int                            rightMost;
    int                            grabPoint;
    int                            last;
    int                            xMouseOver;
    int                            yMouseOver;
    int                            progressCount;      // Position of animation during loading/calculation.
    ChannelType                    channelType;        // Channel type to draw
    HistogramScale                 scaleType;          // Scale to use for drawing
    ImageHistogram*                imageHistogram;     // Full image

    QTimer*                        progressTimer;

    DWorkingPixmap                 progressPix;

    DColor                         colorGuide;

    ImageCurves*                   curves;             // Curves data instance.

    HistogramPainter*              histogramPainter;

    // --- misc methods ---

    int getDelta() const
    {
        // TODO magic number, what is this?
        return imageHistogram->getHistogramSegments() / 16;
    }

    void getHistogramCoordinates(const QPoint& mousePosition, int& x, int& y, int& closestPoint)
    {
        x = CLAMP((int)(mousePosition.x() *
                        ((float)(imageHistogram->getMaxSegmentIndex()) / (float)q->width())),
                  0, imageHistogram->getMaxSegmentIndex());
        y = CLAMP((int)(mousePosition.y() *
                        ((float)(imageHistogram->getMaxSegmentIndex()) / (float)q->height())),
                  0, imageHistogram->getMaxSegmentIndex());

        int distance = NUM_SEGMENTS_16BIT;

        closestPoint = 0;

        for (int i = 0; i < ImageCurves::NUM_POINTS; ++i)
        {
            int xcurvepoint = curves->getCurvePointX(channelType, i);

            if (xcurvepoint != -1)
            {
                if (abs(x - xcurvepoint) < distance)
                {
                    distance     = abs(x - xcurvepoint);
                    closestPoint = i;
                }
            }
        }

        // TODO magic number, what is this?
        if (distance > 8)
        {
            closestPoint = (x + getDelta() / 2) / getDelta();
        }
    }

    // --- rendering ---

    void renderLoadingAnimation()
    {

        QPixmap anim(progressPix.frameAt(progressCount));
        ++progressCount;

        if (progressCount >= progressPix.frameCount())
        {
            progressCount = 0;
        }

        // ... and we render busy text.

        QPainter p1(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width() - 1, q->height() - 1);
        p1.drawPixmap(q->width() / 2 - anim.width() / 2, anim.height(), anim);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));

        if (clearFlag == Private::HistogramDataLoading)
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
    }

    void renderHistogramFailed()
    {
        QPainter p1(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width() - 1, q->height() - 1);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));
        p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                    i18n("Histogram\ncalculation\nfailed."));
        p1.end();
    }

    void renderCurve(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing curves.
        QPainterPath curvePath;
        curvePath.moveTo(0, wHeight);

        for (int x = 0 ; x < wWidth; ++x)
        {

            // TODO duplicate code...
            int i = (x * imageHistogram->getHistogramSegments()) / wWidth;

            int curveVal = curves->getCurveValue(channelType, i);
            curvePath.lineTo(x, wHeight - ((curveVal * wHeight) / imageHistogram->getHistogramSegments()));
        }

        curvePath.lineTo(wWidth, wHeight);

        p1.save();
        p1.setRenderHint(QPainter::Antialiasing);
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Link), 2, Qt::SolidLine));
        p1.drawPath(curvePath);
        p1.restore();


        // Drawing curves points.
        if (!readOnlyMode && curves->getCurveType(channelType) == ImageCurves::CURVE_SMOOTH)
        {

            p1.save();
            p1.setPen(QPen(Qt::red, 3, Qt::SolidLine));
            p1.setRenderHint(QPainter::Antialiasing);

            for (int p = 0 ; p < ImageCurves::NUM_POINTS ; ++p)
            {
                QPoint curvePoint = curves->getCurvePoint(channelType, p);

                if (curvePoint.x() >= 0)
                {
                    p1.drawEllipse(((curvePoint.x() * wWidth) / imageHistogram->getHistogramSegments()) - 2,
                                   wHeight - 2 - ((curvePoint.y() * wHeight) / imageHistogram->getHistogramSegments()),
                                   4, 4);
                }
            }

            p1.restore();
        }
    }

    void renderGrid(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing black/middle/highlight tone grid separators.
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Base), 1, Qt::SolidLine));
        p1.drawLine(wWidth / 4, 0, wWidth / 4, wHeight);
        p1.drawLine(wWidth / 2, 0, wWidth / 2, wHeight);
        p1.drawLine(3 * wWidth / 4, 0, 3 * wWidth / 4, wHeight);
        p1.drawLine(0, wHeight / 4, wWidth, wHeight / 4);
        p1.drawLine(0, wHeight / 2, wWidth, wHeight / 2);
        p1.drawLine(0, 3 * wHeight / 4, wWidth, 3 * wHeight / 4);

    }

    void renderMousePosition(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing X,Y point position dragged by mouse over widget.
        p1.setPen(QPen(Qt::red, 1, Qt::DotLine));

        if (xMouseOver != -1 && yMouseOver != -1)
        {
            QString string = i18n("x:%1\ny:%2", xMouseOver, yMouseOver);
            QFontMetrics fontMt(string);
            QRect rect     = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string);
            rect.moveRight(wWidth);
            rect.moveBottom(wHeight);
            p1.drawText(rect, Qt::AlignLeft | Qt::AlignTop, string);
        }
    }

    void renderFrame(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.setPen(QPen(q->palette().color(QPalette::Active,
                                          QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, pm.width() - 1, pm.height() - 1);
    }

    // --- patterns for storing / restoring state ---

    static QString getChannelTypeOption(const QString& prefix, int channel)
    {
        return QString(prefix + QString::fromLatin1("Channel%1Type")).arg(channel);
    }

    static QString getPointOption(const QString& prefix, int channel, int point)
    {
        return QString(prefix + QString::fromLatin1("Channel%1Point%2")).arg(channel).arg(point);
    }

private:

    CurvesWidget* q;
};

CurvesWidget::CurvesWidget(int w, int h, QWidget* const parent, bool readOnly)
    : QWidget(parent), d(new Private(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setup(w, h, readOnly);
}

CurvesWidget::~CurvesWidget()
{
    d->progressTimer->stop();

    delete d->imageHistogram;
    delete d->curves;
    delete d;
}

void CurvesWidget::setup(int w, int h, bool readOnly)
{
    d->readOnlyMode     = readOnly;
    d->curves           = new ImageCurves(true);
    d->histogramPainter = new HistogramPainter(this);
    d->histogramPainter->setChannelType(LuminosityChannel);
    d->histogramPainter->setRenderXGrid(false);
    d->histogramPainter->setHighlightSelection(false);
    d->histogramPainter->initFrom(this);
    d->channelType      = LuminosityChannel;
    d->scaleType        = LogScaleHistogram;
    d->imageHistogram   = 0;

    setMouseTracking(true);
    setMinimumSize(w, h);

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

void CurvesWidget::saveCurve(KConfigGroup& group, const QString& prefix)
{
    qCDebug(DIGIKAM_DIMG_LOG) << "Storing curves";

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        group.writeEntry(Private::getChannelTypeOption(prefix, channel),
                         (int) curves()->getCurveType(channel));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = curves()->getCurvePoint(channel, point);

            if (!isSixteenBits() && p != ImageCurves::getDisabledValue())
            {
                // Store point as 16 bits depth.
                p.setX(p.x() * ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() * ImageCurves::MULTIPLIER_16BIT);
            }

            group.writeEntry(Private::getPointOption(prefix, channel, point), p);
        }
    }
}

void CurvesWidget::restoreCurve(KConfigGroup& group, const QString& prefix)
{
    qCDebug(DIGIKAM_DIMG_LOG) << "Restoring curves";

    reset();

    qCDebug(DIGIKAM_DIMG_LOG) << "curves " << curves() << " isSixteenBits = " << isSixteenBits();

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        curves()->setCurveType(channel, (ImageCurves::CurveType) group.readEntry(
                                   Private::getChannelTypeOption(
                                       prefix, channel), 0));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = group.readEntry(Private::getPointOption(prefix,
                                                                        channel, point), ImageCurves::getDisabledValue());

            // always load a 16 bit curve and stretch it to 8 bit if necessary
            if (!isSixteenBits() && p != ImageCurves::getDisabledValue())
            {
                p.setX(p.x() / ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() / ImageCurves::MULTIPLIER_16BIT);
            }

            curves()->setCurvePoint(channel, point, p);
        }

        curves()->curvesCalculateCurve(channel);
    }
}

void CurvesWidget::updateData(const DImg& img)
{
    qCDebug(DIGIKAM_DIMG_LOG) << "updating data";

    stopHistogramComputation();

    // Remove old histogram data from memory.
    delete d->imageHistogram;
    d->imageHistogram = new ImageHistogram(img);

    connect(d->imageHistogram, SIGNAL(calculationStarted()),
            this, SLOT(slotCalculationStarted()));

    connect(d->imageHistogram, SIGNAL(calculationFinished(bool)),
            this, SLOT(slotCalculationFinished(bool)));

    d->imageHistogram->calculateInThread();

    // keep the old curve
    ImageCurves* const newCurves = new ImageCurves(img.sixteenBit());
    newCurves->setCurveType(ImageCurves::CURVE_SMOOTH);

    if (d->curves)
    {
        newCurves->fillFromOtherCurves(d->curves);
        delete d->curves;
    }

    d->curves = newCurves;

    resetUI();
}

bool CurvesWidget::isSixteenBits() const
{
    return curves()->isSixteenBits();
}

void CurvesWidget::reset()
{
    if (d->curves)
    {
        d->curves->curvesReset();
    }

    resetUI();
}

void CurvesWidget::resetUI()
{
    d->grabPoint    = -1;
    d->guideVisible = false;
    update();
}

ImageCurves* CurvesWidget::curves() const
{
    return d->curves;
}

void CurvesWidget::setDataLoading()
{
    if (d->clearFlag != Private::HistogramDataLoading)
    {
        setCursor(Qt::WaitCursor);
        d->clearFlag     = Private::HistogramDataLoading;
        d->progressCount = 0;
        d->progressTimer->start(100);
    }
}

void CurvesWidget::setLoadingFailed()
{
    d->clearFlag     = Private::HistogramFailed;
    d->progressCount = 0;
    d->progressTimer->stop();
    update();
    setCursor(Qt::ArrowCursor);
}

void CurvesWidget::setCurveGuide(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    update();
}

void CurvesWidget::curveTypeChanged()
{
    switch (d->curves->getCurveType(d->channelType))
    {
        case ImageCurves::CURVE_SMOOTH:

            //  pick representative points from the curve and make them control points
            int index;

            for (int i = 0; i <= 16; ++i)
            {
                index = CLAMP(i * d->imageHistogram->getHistogramSegments() / 16, 0, d->imageHistogram->getMaxSegmentIndex());
                d->curves->setCurvePoint(d->channelType, i, QPoint(index, d->curves->getCurveValue(d->channelType, index)));
            }

            d->curves->curvesCalculateCurve(d->channelType);
            break;

        case ImageCurves::CURVE_FREE:
            break;
    }

    update();
    emit signalCurvesChanged();
}

void CurvesWidget::slotCalculationStarted()
{
    setCursor(Qt::WaitCursor);
    d->clearFlag = Private::HistogramStarted;
    d->progressTimer->start(200);
    update();
}

void CurvesWidget::slotCalculationFinished(bool success)
{
    if (success)
    {
        // Repaint histogram
        d->clearFlag = Private::HistogramCompleted;
        d->progressTimer->stop();
        update();
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        d->clearFlag = Private::HistogramFailed;
        d->progressTimer->stop();
        update();
        setCursor(Qt::ArrowCursor);
        emit signalHistogramComputationFailed();
    }
}

void CurvesWidget::stopHistogramComputation()
{
    if (d->imageHistogram)
    {
        d->imageHistogram->stopCalculation();
    }

    d->progressTimer->stop();
    d->progressCount = 0;
}

void CurvesWidget::slotProgressTimerDone()
{
    update();
    d->progressTimer->start(200);
}

void CurvesWidget::paintEvent(QPaintEvent*)
{
    // special cases

    if (d->clearFlag == Private::HistogramDataLoading ||
        d->clearFlag == Private::HistogramStarted)
    {
        d->renderLoadingAnimation();
        return;
    }
    else if (d->clearFlag == Private::HistogramFailed)
    {
        d->renderHistogramFailed();
        return;
    }

    // normal case, histogram present

    if (!d->imageHistogram)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Should render a histogram, but did not get one.";
        return;
    }

    // render subelements on a pixmap (double buffering)
    QPixmap pm(size());

    d->histogramPainter->setScale(d->scaleType);
    d->histogramPainter->setHistogram(d->imageHistogram);
    d->histogramPainter->setChannelType(d->channelType);

    if (d->guideVisible)
    {
        d->histogramPainter->enableHistogramGuideByColor(d->colorGuide);
    }
    else
    {
        d->histogramPainter->disableHistogramGuide();
    }

    d->histogramPainter->render(pm);
    d->renderCurve(pm);
    d->renderGrid(pm);
    d->renderMousePosition(pm);
    d->renderFrame(pm);

    // render pixmap on widget
    QPainter p2(this);
    p2.drawPixmap(0, 0, pm);
    p2.end();
}

void CurvesWidget::mousePressEvent(QMouseEvent* e)
{
    if (d->readOnlyMode || !d->imageHistogram)
    {
        return;
    }

    if (e->button() != Qt::LeftButton || d->clearFlag == Private::HistogramStarted)
    {
        return;
    }

    int x, y, closest_point;
    d->getHistogramCoordinates(e->pos(), x, y, closest_point);

    setCursor(Qt::CrossCursor);

    switch (d->curves->getCurveType(d->channelType))
    {
        case ImageCurves::CURVE_SMOOTH:
        {
            // Determine the leftmost and rightmost points.

            d->leftMost = -1;

            for (int i = closest_point - 1; i >= 0; --i)
            {
                if (d->curves->getCurvePointX(d->channelType, i) != -1)
                {
                    d->leftMost = d->curves->getCurvePointX(d->channelType, i);
                    break;
                }
            }

            d->rightMost = d->imageHistogram->getHistogramSegments();

            for (int i = closest_point + 1; i < ImageCurves::NUM_POINTS; ++i)
            {
                if (d->curves->getCurvePointX(d->channelType, i) != -1)
                {
                    d->rightMost = d->curves->getCurvePointX(d->channelType, i);
                    break;
                }
            }

            d->grabPoint = closest_point;
            d->curves->setCurvePoint(d->channelType, d->grabPoint,
                                     QPoint(x, d->imageHistogram->getHistogramSegments() - y));

            break;
        }

        case ImageCurves::CURVE_FREE:
        {
            d->curves->setCurveValue(d->channelType, x, d->imageHistogram->getHistogramSegments() - y);
            d->grabPoint = x;
            d->last      = y;
            break;
        }
    }

    d->curves->curvesCalculateCurve(d->channelType);
    emit signalCurvesChanged();
    update();
}

void CurvesWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d->readOnlyMode || !d->imageHistogram)
    {
        return;
    }

    if (e->button() != Qt::LeftButton || d->clearFlag == Private::HistogramStarted)
    {
        return;
    }

    setCursor(Qt::ArrowCursor);
    d->grabPoint = -1;
}

void CurvesWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (d->readOnlyMode || !d->imageHistogram)
    {
        return;
    }

    if (d->clearFlag == Private::HistogramStarted)
    {
        return;
    }

    int x, y, closest_point;
    d->getHistogramCoordinates(e->pos(), x, y, closest_point);

    switch (d->curves->getCurveType(d->channelType))
    {
        case ImageCurves::CURVE_SMOOTH:
        {
            if (d->grabPoint == -1)   // If no point is grabbed...
            {
                if (d->curves->getCurvePointX(d->channelType, closest_point) != -1)
                {
                    setCursor(Qt::ArrowCursor);
                }
                else
                {
                    setCursor(Qt::CrossCursor);
                }
            }
            else                      // Else, drag the grabbed point
            {
                setCursor(Qt::CrossCursor);

                d->curves->setCurvePointX(d->channelType, d->grabPoint, -1);

                if (x > d->leftMost && x < d->rightMost)
                {
                    closest_point = (x + d->getDelta() / 2) / d->getDelta();

                    if (d->curves->getCurvePointX(d->channelType, closest_point) == -1)
                    {
                        d->grabPoint = closest_point;
                    }

                    d->curves->setCurvePoint(d->channelType, d->grabPoint,
                                             QPoint(x, d->imageHistogram->getMaxSegmentIndex() - y));
                }

                d->curves->curvesCalculateCurve(d->channelType);
                emit signalCurvesChanged();
            }

            break;
        }

        case ImageCurves::CURVE_FREE:
        {
            if (d->grabPoint != -1)
            {
                int x1, x2, y1, y2;

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
                    for (int i = x1 ; i <= x2 ; ++i)
                        d->curves->setCurveValue(d->channelType, i, d->imageHistogram->getMaxSegmentIndex() -
                                                 (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
                }
                else
                {
                    d->curves->setCurveValue(d->channelType, x, d->imageHistogram->getMaxSegmentIndex() - y);
                }

                d->grabPoint = x;
                d->last      = y;

                emit signalCurvesChanged();
            }

            break;
        }
    }

    d->xMouseOver = x;
    d->yMouseOver = d->imageHistogram->getMaxSegmentIndex() - y;
    emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
    update();
}

void CurvesWidget::leaveEvent(QEvent*)
{
    d->xMouseOver = -1;
    d->yMouseOver = -1;
    emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
    update();
}

void CurvesWidget::setChannelType(ChannelType channel)
{
    d->channelType = channel;
    update();
}

void CurvesWidget::setScaleType(HistogramScale scale)
{
    d->scaleType = scale;
    update();
}

}  // namespace Digikam
