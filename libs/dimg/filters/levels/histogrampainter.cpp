/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-26
 * Description : a class that manages painting histograms
 *
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "histogrampainter.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

#define HISTOGRAM_CALC_CUTOFF_MIN    0.1
#define HISTOGRAM_CALC_CUTOFF_MAX    0.9
#define HISTOGRAM_CALC_CUTOFF_HEIGHT 0.8

namespace Digikam
{

class HistogramPainter::Private
{

public:

    explicit Private() :
        histogram(0),
        widgetToInitFrom(0),
        scale(LogScaleHistogram),
        channelType(LuminosityChannel),
        highlightSelection(false),
        selectionMin(0.0),
        selectionMax(0.0),
        showColorGuide(false),
        showXGrid(true)
    {
    }

public:

    double calculateMax()
    {
        int segments = histogram->getHistogramSegments();
        int startSeg = (int)(HISTOGRAM_CALC_CUTOFF_MIN * (segments - 1));
        int endSeg   = (int)(HISTOGRAM_CALC_CUTOFF_MAX * (segments - 1));
        double max   = 0.0;

        switch (scale)
        {
            case LinScaleHistogram:

                switch (channelType)
                {
                    case GreenChannel:
                    case BlueChannel:
                    case RedChannel:
                    case AlphaChannel:
                    case LuminosityChannel:
                        max = qMin(histogram->getMaximum(channelType, startSeg, endSeg) / HISTOGRAM_CALC_CUTOFF_HEIGHT,
                                   histogram->getMaximum(channelType, 0, segments - 1));
                        break;

                    case ColorChannels:
                        max = qMin(qMax(qMax(histogram->getMaximum(RedChannel, startSeg, endSeg),
                                             histogram->getMaximum(GreenChannel, startSeg, endSeg)),
                                        histogram->getMaximum(BlueChannel, startSeg, endSeg)) / HISTOGRAM_CALC_CUTOFF_HEIGHT,
                                   qMax(qMax(histogram->getMaximum(RedChannel, 0, segments - 1),
                                             histogram->getMaximum(GreenChannel, 0, segments - 1)),
                                        histogram->getMaximum(BlueChannel, 0, segments - 1)));
                        break;

                    default:
                        qCDebug(DIGIKAM_DIMG_LOG) << "Untreated channel type " << channelType << ". Using luminosity as default.";
                        max = qMin(histogram->getMaximum(LuminosityChannel, startSeg, endSeg) / HISTOGRAM_CALC_CUTOFF_HEIGHT,
                                   histogram->getMaximum(LuminosityChannel, 0, segments - 1));
                        break;
                }

                break;

            case LogScaleHistogram:

                switch (channelType)
                {
                    case GreenChannel:
                    case BlueChannel:
                    case RedChannel:
                    case AlphaChannel:
                    case LuminosityChannel:
                        max = histogram->getMaximum(channelType, 0, segments - 1);
                        break;

                    case ColorChannels:
                        max = qMax(qMax(histogram->getMaximum(RedChannel, 0, segments - 1),
                                        histogram->getMaximum(GreenChannel, 0, segments - 1)),
                                   histogram->getMaximum(BlueChannel, 0, segments - 1));
                        break;

                    default:
                        qCDebug(DIGIKAM_DIMG_LOG) << "Untreated channel type " << channelType << ". Using luminosity as default.";
                        max = histogram->getMaximum(LuminosityChannel, 0, segments - 1);
                        break;
                }

                if (max > 0.0)
                {
                    max = log(max);
                }
                else
                {
                    max = 1.0;
                }

                break;

            default:
                qCDebug(DIGIKAM_DIMG_LOG) << "Untreated histogram scale " << scale << ". Using linear as default.";
                break;
        }

        return max;
    }

    inline int scaleToPixmapHeight(const double& value, const int& pixmapHeight, const double& max)
    {
        if (max == 0)
        {
            return 0;
        }

        switch (scale)
        {
            case LinScaleHistogram:
            {
                return qMin((int)((pixmapHeight * value) / max), pixmapHeight);
            }

            case LogScaleHistogram:
            {
                if (value == 0.0)
                {
                    return 0;
                }

                if (value < 0.0)
                {
                    qCWarning(DIGIKAM_DIMG_LOG) << "Scaling value < 0: " << value << ". Assuming 0.";
                    return 0;
                }

                return qMin((int)((pixmapHeight * log(value)) / max), pixmapHeight);
            }

            default:
            {
                qCDebug(DIGIKAM_DIMG_LOG) << "Unknown scale type " << scale;
                return 0;
            }
        }
    }

    inline void calculateSegmentsForIndex(const int& x, const int& drawWidth,
                                          int& startSegment, int& endSegment)
    {
        if (drawWidth == 0)
        {
            startSegment = 0;
            endSegment   = 0;
            return;
        }

        startSegment = (x       * (histogram->getHistogramSegments()) - 1) / drawWidth;
        endSegment   = ((x + 1) * (histogram->getHistogramSegments()) - 1) / drawWidth;
    }

    void renderSingleColorLine(QPixmap& bufferPixmap, QPainter& p1)
    {
        p1.save();

        int wWidth  = bufferPixmap.width();
        int wHeight = bufferPixmap.height();

        QImage bb(wWidth, wHeight, QImage::Format_RGB32);
        QPainter p2;
        p2.begin(&bb);
        p2.initFrom(widgetToInitFrom);

        double max  = 1.05 * calculateMax();

        QPainterPath curvePath;
        curvePath.moveTo(1, wHeight - 1);

        int yPrev = 0;

        for (int x = 1; x < (wWidth - 1); ++x)
        {
            // calculate histogram segments included in this single pixel line
            int startSegment = 0;
            int endSegment   = 0;
            calculateSegmentsForIndex(x - 1, wWidth - 2, startSegment, endSegment);

            double value = histogram->getMaximum(channelType, startSegment, endSegment);
            int y        = scaleToPixmapHeight(value, wHeight - 2, max);

            if (x > 1)
            {
                (y > yPrev) ? curvePath.lineTo(x, wHeight - yPrev) : curvePath.lineTo(x - 1, wHeight - y);
            }

            curvePath.lineTo(x, wHeight - y);
            yPrev = y;
        }

        curvePath.lineTo(wWidth - 2, wHeight - 1);
        curvePath.lineTo(1,          wHeight - 1);
        curvePath.closeSubpath();

        p2.fillRect(0, 0, wWidth, wHeight, palette.color(QPalette::Active, QPalette::Background));

        QColor pColor;
        QColor bColor;

        switch (channelType)
        {
            case GreenChannel:
                pColor = QColor(63, 255, 63);
                bColor = QColor(0, 192, 0);
                break;

            case BlueChannel:
                pColor = QColor(63, 63, 255);
                bColor = QColor(0, 0, 192);
                break;

            case RedChannel:
                pColor = QColor(255, 63, 63);
                bColor = QColor(192, 0, 0);
                break;

            default:
                pColor = palette.color(QPalette::Active,   QPalette::Foreground);
                bColor = palette.color(QPalette::Inactive, QPalette::Foreground);
                break;
        }

        p2.setPen(QPen(pColor, 1, Qt::SolidLine));
        p2.setBrush(QBrush(bColor, Qt::SolidPattern));
        p2.drawPath(curvePath);

        if (highlightSelection)
        {
            p2.setClipRect((int)(selectionMin * wWidth), 0,
                           (int)(selectionMax * wWidth - selectionMin * wWidth), wHeight);
            p2.fillRect((int)(selectionMin * wWidth), 0,
                        (int)(selectionMax * wWidth - selectionMin * wWidth), wHeight,
                        QBrush(palette.color(QPalette::Active, QPalette::Foreground), Qt::SolidPattern));
            p2.fillPath(curvePath, QBrush(palette.color(QPalette::Active, QPalette::Background), Qt::SolidPattern));
        }

        p2.end();
        p1.drawImage(0, 0, bb);

        p1.restore();
    }

    void renderMultiColorLine(QPixmap& bufferPixmap, QPainter& p1)
    {
        p1.save();

        int wWidth  = bufferPixmap.width();
        int wHeight = bufferPixmap.height();

        QImage bb(wWidth, wHeight, QImage::Format_RGB32);
        QPainter p2;
        p2.begin(&bb);
        p2.initFrom(widgetToInitFrom);

        double max  = 1.05 * calculateMax();

        QPainterPath curveRed, curveGreen, curveBlue;
        curveRed.moveTo(1, wHeight - 1);
        curveGreen.moveTo(1, wHeight - 1);
        curveBlue.moveTo(1, wHeight - 1);

        int yrPrev = 0;
        int ygPrev = 0;
        int ybPrev = 0;

        for (int x = 1; x < (wWidth - 1); ++x)
        {
            // calculate histogram segments included in this single pixel line
            int startSegment = 0;
            int endSegment   = 0;
            calculateSegmentsForIndex(x - 1, wWidth - 2, startSegment, endSegment);

            double valueR = histogram->getMaximum(RedChannel,   startSegment, endSegment);
            double valueG = histogram->getMaximum(GreenChannel, startSegment, endSegment);
            double valueB = histogram->getMaximum(BlueChannel,  startSegment, endSegment);

            int yr = scaleToPixmapHeight(valueR, wHeight - 1, max);
            int yg = scaleToPixmapHeight(valueG, wHeight - 1, max);
            int yb = scaleToPixmapHeight(valueB, wHeight - 1, max);

            if (x > 1)
            {
                (yr > yrPrev) ? curveRed.lineTo(x,   wHeight - yrPrev) : curveRed.lineTo(x - 1,   wHeight - yr);
                (yg > ygPrev) ? curveGreen.lineTo(x, wHeight - ygPrev) : curveGreen.lineTo(x - 1, wHeight - yg);
                (yb > ybPrev) ? curveBlue.lineTo(x,  wHeight - ybPrev) : curveBlue.lineTo(x - 1,  wHeight - yb);
            }

            curveRed.lineTo(x,   wHeight - yr);
            curveGreen.lineTo(x, wHeight - yg);
            curveBlue.lineTo(x,  wHeight - yb);

            yrPrev = yr;
            ygPrev = yg;
            ybPrev = yb;
        }

        curveRed.lineTo(wWidth - 2, wHeight - 1);
        curveRed.lineTo(1, wHeight - 1);
        curveRed.closeSubpath();
        curveGreen.lineTo(wWidth - 2, wHeight - 1);
        curveGreen.lineTo(1, wHeight - 1);
        curveGreen.closeSubpath();
        curveBlue.lineTo(wWidth - 2, wHeight - 1);
        curveBlue.lineTo(1, wHeight - 1);
        curveBlue.closeSubpath();

        p2.fillRect(0, 0, wWidth, wHeight, palette.color(QPalette::Active, QPalette::Background));
        p2.fillPath(curveBlue,  QBrush(Qt::black, Qt::SolidPattern));
        p2.fillPath(curveRed,   QBrush(Qt::black, Qt::SolidPattern));
        p2.fillPath(curveGreen, QBrush(Qt::black, Qt::SolidPattern));

        p2.setCompositionMode(QPainter::CompositionMode_Screen);
        p2.setPen(QPen(QColor(63, 63, 255), 1, Qt::SolidLine));
        p2.setBrush(QBrush(QColor(0, 0, 192),  Qt::SolidPattern));
        p2.drawPath(curveBlue);
        p2.setPen(QPen(QColor(255, 63, 63), 1, Qt::SolidLine));
        p2.setBrush(QBrush(QColor(192, 0, 0),  Qt::SolidPattern));
        p2.drawPath(curveRed);
        p2.setPen(QPen(QColor(63, 255, 63), 1, Qt::SolidLine));
        p2.setBrush(QBrush(QColor(0, 192, 0),  Qt::SolidPattern));
        p2.drawPath(curveGreen);

        // Highlight
        if (highlightSelection)
        {
            p2.setClipRect((int)(selectionMin * wWidth), 0,
                           (int)(selectionMax * wWidth - selectionMin * wWidth), wHeight);
            p2.setCompositionMode(QPainter::CompositionMode_Source);
            p2.fillRect((int)(selectionMin * wWidth), 0,
                        (int)(selectionMax * wWidth - selectionMin * wWidth), wHeight,
                        palette.color(QPalette::Active, QPalette::Foreground));
            p2.fillPath(curveBlue, QBrush(Qt::black,  Qt::SolidPattern));
            p2.fillPath(curveRed, QBrush(Qt::black,   Qt::SolidPattern));
            p2.fillPath(curveGreen, QBrush(Qt::black, Qt::SolidPattern));
            p2.setCompositionMode(QPainter::CompositionMode_Screen);
            p2.fillPath(curveBlue, QBrush(QColor(0, 0, 255),  Qt::SolidPattern));
            p2.fillPath(curveRed, QBrush(QColor(255, 0, 0),   Qt::SolidPattern));
            p2.fillPath(curveGreen, QBrush(QColor(0, 255, 0), Qt::SolidPattern));
            p2.setClipRect(0, 0, wWidth, wHeight);
        }

        p2.end();
        p1.drawImage(0, 0, bb);

        p1.restore();
    }

    void renderXGrid(QPixmap& bufferPixmap, QPainter& p1)
    {
        for (int x = 0; x < bufferPixmap.width(); ++x)
        {
            if ((x == bufferPixmap.width() / 4) || (x == bufferPixmap.width() / 2) ||
                (x == 3 * bufferPixmap.width() / 4))
            {
                p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Base), 1, Qt::SolidLine));
                p1.drawLine(x, bufferPixmap.height(), x, 0);
            }
        }
    }

    void renderColorGuide(QPixmap& bufferPixmap, QPainter& p1)
    {

        if (histogram->isSixteenBit() && !colorGuide.sixteenBit())
        {
            colorGuide.convertToSixteenBit();
        }
        else if (!histogram->isSixteenBit() && colorGuide.sixteenBit())
        {
            colorGuide.convertToEightBit();
        }

        p1.setPen(QPen(Qt::red, 1, Qt::DotLine));

        int guidePos = -1;

        switch (channelType)
        {
            case RedChannel:
            {
                guidePos = colorGuide.red();
                break;
            }

            case GreenChannel:
            {
                guidePos = colorGuide.green();
                break;
            }

            case BlueChannel:
            {
                guidePos = colorGuide.blue();
                break;
            }

            case LuminosityChannel:
            {
                guidePos = qMax(qMax(colorGuide.red(), colorGuide.green()), colorGuide.blue());
                break;
            }

            case ColorChannels:
            {
                guidePos = qMax(qMax(colorGuide.red(), colorGuide.green()), colorGuide.blue());
                break;
            }

            default:
            {
                guidePos = colorGuide.alpha();
                break;
            }
        }

        if (guidePos != -1)
        {
            int xGuide = (int)(((double)(guidePos * bufferPixmap.width())) / ((double)histogram->getHistogramSegments()));

            p1.drawLine(xGuide, 0, xGuide, bufferPixmap.height());

            QString string = i18n("x:%1", guidePos);
            QFontMetrics fontMt(string);
            QRect rect     = fontMt.boundingRect(0, 0, bufferPixmap.width(), bufferPixmap.height(), 0, string);
            p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
            rect.moveTop(1);

            if (xGuide < bufferPixmap.width() / 2)
            {
                rect.moveLeft(xGuide);
                p1.fillRect(rect, QBrush(QColor(250, 250, 255)));
                p1.drawRect(rect);
                rect.moveLeft(xGuide + 3);
                p1.drawText(rect, Qt::AlignLeft, string);
            }
            else
            {
                rect.moveRight(xGuide);
                p1.fillRect(rect, QBrush(QColor(250, 250, 255)));
                p1.drawRect(rect);
                rect.moveRight(xGuide - 3);
                p1.drawText(rect, Qt::AlignRight, string);
            }
        }
    }

public:

    ImageHistogram*   histogram;
    QPainter          painter;
    QWidget*          widgetToInitFrom;
    QPalette          palette;

    // rendering settings
    HistogramScale    scale;
    ChannelType       channelType;
    bool              highlightSelection;
    double            selectionMin;
    double            selectionMax;
    bool              showColorGuide;
    bool              showXGrid;
    DColor            colorGuide;
};

HistogramPainter::HistogramPainter(QObject* const parent)
    : QObject(parent), d(new Private())
{
}

HistogramPainter::~HistogramPainter()
{
    delete d;
}

void HistogramPainter::setHistogram(ImageHistogram* const histogram)
{
    d->histogram = histogram;
}

void HistogramPainter::setScale(HistogramScale scale)
{
    d->scale = scale;
}

void HistogramPainter::setChannelType(ChannelType channelType)
{
    d->channelType = channelType;
}

void HistogramPainter::setHighlightSelection(bool highlightSelection)
{
    d->highlightSelection = highlightSelection;
}

void HistogramPainter::setSelection(double selectionMin, double selectionMax)
{
    if (selectionMin < 0.0 || selectionMin > 1.0)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "selectionMin out of range: " << selectionMin << ". Clamping value";
        selectionMin = qMax(0.0, qMin(1.0, selectionMin));
    }

    if (selectionMax < 0.0 || selectionMax > 1.0)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "selectionMax out of range: " << selectionMax << ". Clamping value";
        selectionMax = qMax(0.0, qMin(1.0, selectionMax));
    }

    d->selectionMin = selectionMin;
    d->selectionMax = selectionMax;
}

void HistogramPainter::setRenderXGrid(bool renderXGrid)
{
    d->showXGrid = renderXGrid;
}

void HistogramPainter::enableHistogramGuideByColor(const DColor& color)
{
    d->colorGuide     = color;
    d->showColorGuide = true;
}

void HistogramPainter::disableHistogramGuide()
{
    d->showColorGuide = false;
}

void HistogramPainter::initFrom(QWidget* const widget)
{
    d->widgetToInitFrom = widget;
}

void HistogramPainter::render(QPixmap& bufferPixmap)
{
    if (!d->histogram)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Cannot render because the histogram is missing";
        return;
    }

    int wWidth  = bufferPixmap.width();
    int wHeight = bufferPixmap.height();

    d->painter.begin(&bufferPixmap);

    if (d->widgetToInitFrom)
    {
        d->painter.initFrom(d->widgetToInitFrom);
        d->palette = d->widgetToInitFrom->palette();
    }

    // clear background
    d->painter.fillRect(0, 0, wWidth, wHeight, d->palette.color(QPalette::Active, QPalette::Background));

    // decide how to render the line
    if (d->channelType == ColorChannels)
    {
        d->renderMultiColorLine(bufferPixmap, d->painter);
    }
    else
    {
        d->renderSingleColorLine(bufferPixmap, d->painter);
    }

    if (d->showXGrid)
    {
        d->renderXGrid(bufferPixmap, d->painter);
    }

    if (d->showColorGuide)
    {
        d->renderColorGuide(bufferPixmap, d->painter);
    }

    // draw a final border around everything
    d->painter.setPen(QPen(d->palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
    d->painter.drawRect(0, 0, wWidth - 1, wHeight - 1);
    d->painter.end();
}

} // namespace Digikam
