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

#ifndef HISTOGRAMPAINTER_H
#define HISTOGRAMPAINTER_H

// Qt includes

#include <QObject>
#include <QWidget>

// Local includes

#include "imagehistogram.h"
#include "digikam_globals.h"
#include "dcolor.h"

namespace Digikam
{

/**
 * A class that paints a histogram on a QPixmap.
 *
 * Warning: before first usage of the render method, you must call initFrom()
 * to initialize the painter.
 */
class HistogramPainter : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for Qt's destruction mechanism
     */
    explicit HistogramPainter(QObject* const parent);

    /**
     * Destructor.
     */
    virtual ~HistogramPainter();

    /**
     * Set the histogram to paint with the next call to render.
     *
     * @param histogram an existing pointer to a histogram to paint on next call
     *                  to render. The histogram must still exist at that call.
     */
    void setHistogram(ImageHistogram* const histogram);

    /**
     * Set the scale to paint the histogram with.
     *
     * @param scale scal to paint histogram with
     */
    void setScale(HistogramScale scale);

    /**
     * Set the channel type to render with the next call to render.
     *
     * @param channelType channel type to render
     */
    void setChannelType(ChannelType channelType);

    /**
     * Decide whether to highlight a specified selection in the histogram or
     * not. The selection must be defined with setHighlightSelection.
     *
     * @param highlightSelection if true, a selection will be highlighted
     */
    void setHighlightSelection(bool highlightSelection);

    /**
     * Sets the selection to highlight.
     *
     * @param selectionMin 0 <= value <= 1, percent of the histogram width to
     *                     start highlighting as percent. Ensure that this value
     *                     is smaller then selectionMax.
     * @param selectionMax 0 <= value <= 1, percent of the histogram width to
     *                     end highlighting as percent. Ensure that this value
     *                     is greater then selectionMin.
     */
    void setSelection(double selectionMin, double selectionMax);

    /**
     * Decide whether to render a separation of the histogram in x direction.
     *
     * @param renderXGrid if true, a separation at some significant value in
     *                    x direction is rendered.
     */
    void setRenderXGrid(bool renderXGrid);

    /**
     * Starts rendering a guide that indicates where in the histogram a
     * specified color can be found.
     *
     * @param color the color to highlight in the histogram
     */
    void enableHistogramGuideByColor(const DColor& color);

    /**
     * Disables the rendering of the color guide.
     */
    void disableHistogramGuide();

    /**
     * Stores a widget that is used to initialize the painter used in the next
     * call to render. Therefore you must ensure that this widget will not be
     * destroyed as long as you want to use the render method without a new call
     * to this method!!!
     *
     * @param widget widget to initialize painting from
     */
    void initFrom(QWidget* const widget);

    /**
     * Renders the given histogram on the pixmap. The whole size of the pixmap
     * is used for the histogram.
     *
     * You must ensure that once before using this method a call to initFrom was
     * made and the widget given in that call is still present.
     *
     * @param paintDevice pixmap to paint the histogram on
     */
    void render(QPixmap& paintDevice);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* HISTOGRAMPAINTER_H */
