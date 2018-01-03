/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "editorstackview.h"

// Local includes

#include "dzoombar.h"
#include "canvas.h"
#include "thumbnailsize.h"
#include "graphicsdimgview.h"
#include "previewlayout.h"

namespace Digikam
{

class EditorStackView::Private
{

public:

    Private() :
        toolView(0),
        canvas(0)
    {
    }

    QWidget* toolView;
    Canvas*  canvas;
};

EditorStackView::EditorStackView(QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
}

EditorStackView::~EditorStackView()
{
    delete d;
}

void EditorStackView::setCanvas(Canvas* const canvas)
{
    if (d->canvas)
    {
        return;
    }

    d->canvas = canvas;
    insertWidget(CanvasMode, d->canvas);

    connect(d->canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->canvas, SIGNAL(signalToggleOffFitToWindow()),
            this, SIGNAL(signalToggleOffFitToWindow()));
}

Canvas* EditorStackView::canvas() const
{
    return d->canvas;
}

void EditorStackView::setToolView(QWidget* const view)
{
    if (d->toolView)
    {
        removeWidget(d->toolView);
    }

    d->toolView = view;

    if (d->toolView)
    {
        insertWidget(ToolViewMode, d->toolView);
    }

    GraphicsDImgView* const preview = previewWidget();

    if (preview)
    {
        connect(preview->layout(), SIGNAL(zoomFactorChanged(double)),
                this, SLOT(slotZoomChanged(double)));

        connect(preview->layout(), SIGNAL(fitToWindowToggled(bool)),
                this, SLOT(slotToggleOffFitToWindow(bool)));
    }
}

QWidget* EditorStackView::toolView() const
{
    return d->toolView;
}

int EditorStackView::viewMode() const
{
    return indexOf(currentWidget());
}

void EditorStackView::setViewMode(int mode)
{
    if (mode != CanvasMode && mode != ToolViewMode)
    {
        return;
    }

    setCurrentIndex(mode);
}

void EditorStackView::increaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->increaseZoom();
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->increaseZoom();
        }
    }
}

void EditorStackView::decreaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->decreaseZoom();
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->decreaseZoom();
        }
    }
}

void EditorStackView::toggleFitToWindow()
{
    // Fit to window action is common place to switch view in this mode.
    // User want to see the same behavors between canvas and tool preview.
    // Both are toggle at the same time.
    d->canvas->layout()->toggleFitToWindow();

    GraphicsDImgView* const preview = previewWidget();

    if (preview)
    {
        preview->layout()->toggleFitToWindow();
    }
}

void EditorStackView::fitToSelect()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->fitToSelect();
    }
}

void EditorStackView::zoomTo100Percent()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->toggleFitToWindowOr100();
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->toggleFitToWindowOr100();
        }
    }
}

void EditorStackView::setZoomFactor(double zoom)
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->setZoomFactor(zoom);
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->setZoomFactor(zoom);
        }
    }
}

double EditorStackView::zoomMax() const
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->layout()->maxZoomFactor();
    }
    else
    {
        GraphicsDImgView* const preview  = previewWidget();

        if (preview)
        {
            return preview->layout()->maxZoomFactor();
        }
        else
        {
            return -1.0;
        }
    }
}

double EditorStackView::zoomMin() const
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->layout()->minZoomFactor();
    }
    else
    {
        GraphicsDImgView* const preview  = previewWidget();

        if (preview)
        {
            return preview->layout()->minZoomFactor();
        }
        else
        {
            return -1.0;
        }
    }
}

void EditorStackView::slotZoomSliderChanged(int size)
{
    if (viewMode() == ToolViewMode && !isZoomablePreview())
    {
        return;
    }

    double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());

    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->setZoomFactorSnapped(z);
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            return preview->layout()->setZoomFactorSnapped(z);
        }
    }
}

void EditorStackView::slotZoomChanged(double zoom)
{
    bool max, min;

    if (viewMode() == CanvasMode)
    {
        max = d->canvas->layout()->atMaxZoom();
        min = d->canvas->layout()->atMinZoom();
        emit signalZoomChanged(max, min, zoom);
    }
    else
    {
        GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            max = preview->layout()->atMaxZoom();
            min = preview->layout()->atMinZoom();
            emit signalZoomChanged(max, min, zoom);
        }
    }
}

void EditorStackView::slotToggleOffFitToWindow(bool b)
{
    if (b)
    {
        emit signalToggleOffFitToWindow();
    }
}

GraphicsDImgView* EditorStackView::previewWidget() const
{
    GraphicsDImgView* const preview = dynamic_cast<GraphicsDImgView*>(d->toolView);

    if (preview)
    {
        return preview;
    }

    return 0;
}

bool EditorStackView::isZoomablePreview() const
{
    return previewWidget();
}

}  // namespace Digikam
