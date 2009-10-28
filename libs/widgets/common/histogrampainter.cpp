/* ============================================================
 *
 * This file is a part of digikam project
 * http://www.digikam.org
 *
 * Date        : 26.10.2009
 * Description : a class that manages painting histogramsw
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "kdebug.h"
#include "klocale.h"

namespace Digikam
{

class HistogramPainterPriv
{

public:

    HistogramPainterPriv(HistogramPainter *q) :
        q(q),
        histogram(0),
        widgetToInitFrom(0),
        scale(LogScaleHistogram),
        channelType(LuminosityChannel),
        mainColorChannel(ColorChannelsRed),
        highlightSelection(false),
        selectionMin(0.0),
        selectionMax(0.0),
        showColorGuide(false),
        showXGrid(true)
        {}

private:

    HistogramPainter *q;

public:

    double calculateMax()
    {
        double max = 0.0;
        switch (channelType)
        {
            case GreenChannel:
            case BlueChannel:
            case RedChannel:
            case AlphaChannel:
            case LuminosityChannel:
                max = histogram->getMaximum(channelType);
                break;
            case ColorChannels:
                max = qMax(qMax(histogram->getMaximum(RedChannel),
                                histogram->getMaximum(GreenChannel)),
                                histogram->getMaximum(BlueChannel));
            default:
                kError() << "Untreated channel type " << channelType
                                << ". Using luminosity as default.";
                max = histogram->getMaximum(LuminosityChannel);
                break;
        }

        switch (scale)
        {
            case LinScaleHistogram:
                break;
            case LogScaleHistogram:
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
                kError() << "Untreated histogram scale " << scale
                                << ". Using linear as default.";
                break;
        }

        return max;

    }

    void calculateSegmentMaxSingleColor(double &maxValue, const int &startSegment, const int &endSegment)
    {

        // sanity checks
        ChannelType channel = channelType;
        switch (channelType)
        {
            case GreenChannel:
            case BlueChannel:
            case RedChannel:
            case AlphaChannel:
            case LuminosityChannel:
                // these cases are known to work with this function
                break;
            default:
                kError() << "Untreated channel type " << channelType
                                << ". Using luminosity as default";
                channel = LuminosityChannel;
        }


        maxValue = 0.0;

        // find the max value for one segment to paint
        int currentSegment = startSegment;
        do
        {
            maxValue = qMax(maxValue, histogram->getValue(channelType, currentSegment));
            ++currentSegment;
        }
        while (currentSegment < endSegment);

    }

    void calculateSegmentMaxMultiColor(double &maxValueR, double &maxValueG,
                    double &maxValueB, const int &startSegment,
                    const int &endSegment)
    {

        // sanity checks
        if (channelType != ColorChannels) {
            kError() << "This function can only be used for color channels "
                     << "and not for type " << channelType
                     << ". Using color channels as fallback.";
        }

        maxValueR = 0.0;
        maxValueG = 0.0;
        maxValueB = 0.0;

        // find the max value for one segment to paint
        int currentSegment = startSegment;
        do
        {
            maxValueR = qMax(maxValueR, histogram->getValue(RedChannel,
                            currentSegment));
            maxValueG = qMax(maxValueG, histogram->getValue(GreenChannel,
                            currentSegment));
            maxValueB = qMax(maxValueB, histogram->getValue(BlueChannel,
                            currentSegment));
            ++currentSegment;
        }
        while (currentSegment < endSegment);

    }

    inline int scaleToPixmapHeight(const double &value,
                    const int &pixmapHeight, const int &max)
    {
        switch (scale)
        {
            case LinScaleHistogram:
                return (int) ((pixmapHeight * value) / max);
            case LogScaleHistogram:
                if (value <= 0.0)
                {
                    kWarning() << "Scaling value < 0: " << value
                                    << ". Falling back to 1.0";
                    return (int) ((pixmapHeight * log(1.0)) / max);
                }
                return (int) ((pixmapHeight * log(value)) / max);
            default:
                kError() << "Unknown scale type " << scale;
                return 0;
        }
    }

    inline void calculateSegementsForIndex(const int &x, const int &drawWidth,
                    int &startSegment, int &endSegment)
    {
        startSegment = (x       * histogram->getHistogramSegments()) / drawWidth;
        endSegment   = ((x + 1) * histogram->getHistogramSegments()) / drawWidth;
    }

    void renderSingleColorLine(QPixmap &bufferPixmap, QPainter &p1,
                    const int &x, const int &max, const int &startSegment,
                    const int &endSegment, const bool &highlight)
    {
        double value = 0.0;
        calculateSegmentMaxSingleColor(value, startSegment, endSegment);

        int lineHeight = scaleToPixmapHeight(value, bufferPixmap.height(), max);

        if (highlight)
        {
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
            p1.drawLine(x, bufferPixmap.height(), x, 0);
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
            p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeight);
        }
        else
        {
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
            p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeight);
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
            p1.drawLine(x, bufferPixmap.height() - lineHeight, x, 0);
        }

    }

    void renderMultiColorLine(QPixmap &bufferPixmap, QPainter &p1,
                    const int &x, const int &max, const int &startSegment,
                    const int &endSegment, const bool &highlight)
    {

        double valueR = 0.0;
        double valueG = 0.0;
        double valueB = 0.0;
        calculateSegmentMaxMultiColor(valueR, valueG, valueB, startSegment,
                        endSegment);

        int lineHeightR = scaleToPixmapHeight(valueR, bufferPixmap.height(), max);
        int lineHeightG = scaleToPixmapHeight(valueG, bufferPixmap.height(), max);
        int lineHeightB = scaleToPixmapHeight(valueB, bufferPixmap.height(), max);

        if (highlight)
        {
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
            p1.drawLine(x, bufferPixmap.height(), x, 0);
            p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));

            // Witch color must be used on the foreground with all colors channel mode?
            switch (mainColorChannel)
            {
                case ColorChannelsRed:
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightR);
                    break;

                case ColorChannelsGreen:
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightG);
                    break;

                default:
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightB);
                    break;
            }
        }
        else
        {
            // Which color must be used on the foreground with all colors channel mode?
            switch (mainColorChannel)
            {
                case ColorChannelsRed:
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightG);
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightB);
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightR);

                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - qMax(qMax(lineHeightR, lineHeightG), lineHeightB), x, 0);
                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - lineHeightG -1, x, bufferPixmap.height() - lineHeightG);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightB -1, x, bufferPixmap.height() - lineHeightB);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightR -1, x, bufferPixmap.height() - lineHeightR);

                break;

                case ColorChannelsGreen:
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightB);
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightR);
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightG);

                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - qMax(qMax(lineHeightR, lineHeightG), lineHeightB), x, 0);
                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - lineHeightB -1, x, bufferPixmap.height() - lineHeightB);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightR -1, x, bufferPixmap.height() - lineHeightR);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightG -1, x, bufferPixmap.height() - lineHeightG);

                break;

                default:
                    p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightR);
                    p1.setPen(QPen(Qt::green, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightG);
                    p1.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height(), x, bufferPixmap.height() - lineHeightB);

                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Background), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - qMax(qMax(lineHeightR, lineHeightG), lineHeightB), x, 0);
                    p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
                    p1.drawLine(x, bufferPixmap.height() - lineHeightR -1, x, bufferPixmap.height() - lineHeightR);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightG -1, x, bufferPixmap.height() - lineHeightG);
                    p1.drawLine(x, bufferPixmap.height() - lineHeightB -1, x, bufferPixmap.height() - lineHeightB);

                break;
            }
        }


    }

    void renderXGrid(QPixmap &bufferPixmap, QPainter &p1)
    {

        for (int x = 0; x < bufferPixmap.width(); x++)
        {
            if ((x == bufferPixmap.width() / 4) || (x == bufferPixmap.width()
                            / 2) || (x == 3 * bufferPixmap.width() / 4))
            {
                p1.setPen(QPen(palette.color(QPalette::Active, QPalette::Base),
                                1, Qt::SolidLine));
                p1.drawLine(x, bufferPixmap.height(), x, 0);
            }
        }

    }

    void renderColorGuide(QPixmap &bufferPixmap, QPainter &p1)
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

        switch(channelType)
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

            case ColorChannels:
            {
                switch(mainColorChannel)
                {
                    case ColorChannelsRed:
                        guidePos = colorGuide.red();
                        break;

                    case ColorChannelsGreen:
                        guidePos = colorGuide.green();
                        break;

                    case ColorChannelsBlue:
                        guidePos = colorGuide.blue();
                        break;

                    default:
                        kWarning() << "Untreated channel " << mainColorChannel;
                        guidePos = colorGuide.red();
                        break;
                }
                break;
            }

            default:
                guidePos = colorGuide.alpha();
                break;
        }

        if (guidePos != -1)
        {

            int xGuide = (int)(((double)(guidePos * bufferPixmap.width())) / ((double)histogram->getHistogramSegments()));

            p1.drawLine(xGuide, 0, xGuide, bufferPixmap.height());

            QString string = i18n("x:%1", guidePos);
            QFontMetrics fontMt( string );
            QRect rect = fontMt.boundingRect(0, 0, bufferPixmap.width(), bufferPixmap.height(), 0, string);
            p1.setPen(QPen(Qt::red, 1, Qt::SolidLine));
            rect.moveTop(1);

            if (xGuide < bufferPixmap.width() / 2)
            {
                rect.moveLeft(xGuide);
                p1.fillRect(rect, QBrush(QColor(250, 250, 255)) );
                p1.drawRect(rect);
                rect.moveLeft(xGuide + 3);
                p1.drawText(rect, Qt::AlignLeft, string);
            }
            else
            {
                rect.moveRight(xGuide);
                p1.fillRect(rect, QBrush(QColor(250, 250, 255)) );
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
    ColorChannelsType mainColorChannel;
    bool              highlightSelection;
    double            selectionMin;
    double            selectionMax;
    bool              showColorGuide;
    bool              showXGrid;
    DColor            colorGuide;
};

HistogramPainter::HistogramPainter(QObject* parent)
                : QObject(parent), d(new HistogramPainterPriv(this))
{
}

HistogramPainter::~HistogramPainter()
{
    delete d;
}

void HistogramPainter::setHistogram(ImageHistogram* histogram)
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

void HistogramPainter::setMainColorChannel(ColorChannelsType mainColorChannel)
{
    d->mainColorChannel = mainColorChannel;
}

void HistogramPainter::setHighlightSelection(bool highlightSelection)
{
   d->highlightSelection = highlightSelection;
}

void HistogramPainter::setSelection(double selectionMin, double selectionMax)
{

    if (selectionMin < 0.0 || selectionMin > 1.0)
    {
        kWarning() << "selectionMin out of range: " << selectionMin
                        << ". Clamping value";
        selectionMin = qMax(0.0, qMin(1.0, selectionMin));
    }
    if (selectionMax < 0.0 || selectionMax > 1.0)
    {
        kWarning() << "selectionMax out of range: " << selectionMax
                        << ". Clamping value";
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
    d->colorGuide = color;
    d->showColorGuide = true;
}

void HistogramPainter::disableHistogramGuide()
{
    d->showColorGuide = false;
}

void HistogramPainter::initFrom(QWidget* widget)
{
    d->widgetToInitFrom = widget;
}

void HistogramPainter::render(QPixmap& bufferPixmap)
{
    if (!d->histogram)
    {
        kError() << "Cannot render because the histogram is missing";
        return;
    }

    int wWidth  = bufferPixmap.width();
    int wHeight = bufferPixmap.height();

    double max = d->calculateMax();

    d->painter.begin(&bufferPixmap);
    if (d->widgetToInitFrom)
    {
        d->painter.initFrom(d->widgetToInitFrom);
        d->palette = d->widgetToInitFrom->palette();
    }

    // clear background
    d->painter.fillRect(0, 0, wWidth, wHeight, d->palette.color(QPalette::Active, QPalette::Background));

    // draw histogram pixel line by pixel line (on x axis)
    for (int x = 0 ; x < wWidth ; ++x)
    {

        // calculate histogram segments included in this single pixel line
        int startSegment = 0;
        int endSegment = 0;
        d->calculateSegementsForIndex(x, bufferPixmap.width(), startSegment,
                        endSegment);

        // decide whether the current line shall be highlighted as being in the selection
        const bool highlight = d->highlightSelection &&
                        (x >= (int) (d->selectionMin * bufferPixmap.width())) &&
                        (x <= (int) (d->selectionMax * bufferPixmap.width()));

        // decide how to render the line
        if (d->channelType == ColorChannels)
        {
            d->renderMultiColorLine(bufferPixmap, d->painter, x, max,
                            startSegment, endSegment, highlight);
        }
        else
        {
            d->renderSingleColorLine(bufferPixmap, d->painter, x, max,
                            startSegment, endSegment, highlight);
        }

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
    d->painter.drawRect(0, 0, wWidth -1, wHeight - 1);
    d->painter.end();
}

} // namespace Digikam
