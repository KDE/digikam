/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "editorstackview.moc"

// Local includes

#include "dzoombar.h"
#include "previewwidget.h"
#include "canvas.h"
#include "thumbnailsize.h"

namespace Digikam
{

class EditorStackView::EditorStackViewPriv
{

public:

    EditorStackViewPriv() :
        toolView(0),
        canvas(0)
    {
    }

    QWidget* toolView;
    Canvas*  canvas;
};

EditorStackView::EditorStackView(QWidget* parent)
    : QStackedWidget(parent), d(new EditorStackViewPriv)
{
}

EditorStackView::~EditorStackView()
{
    delete d;
}

void EditorStackView::setCanvas(Canvas* canvas)
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

void EditorStackView::setToolView(QWidget* view)
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

    PreviewWidget* preview = previewWidget();

    if (preview)
    {
        connect(preview, SIGNAL(signalZoomFactorChanged(double)),
                this, SLOT(slotZoomChanged(double)));

        connect(preview, SIGNAL(signalToggleOffFitToWindow()),
                this, SIGNAL(signalToggleOffFitToWindow()));
    }
}

QWidget* EditorStackView::toolView() const
{
    return d->toolView;
}

int EditorStackView::viewMode()
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
        d->canvas->slotIncreaseZoom();
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            preview->slotIncreaseZoom();
        }
    }
}

void EditorStackView::decreaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->slotDecreaseZoom();
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            preview->slotDecreaseZoom();
        }
    }
}

void EditorStackView::toggleFitToWindow()
{
    // Fit to window action is common place to switch view in this mode.
    // User want to see the same behavors between canvas and tool preview.
    // Both are toggle at the same time.
    d->canvas->toggleFitToWindow();
    PreviewWidget* preview = previewWidget();

    if (preview)
    {
        preview->toggleFitToWindow();
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
        d->canvas->setZoomFactor(1.0);
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            preview->setZoomFactor(1.0);
        }
    }
}

void EditorStackView::setZoomFactor(double zoom)
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->setZoomFactor(zoom);
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            preview->setZoomFactor(zoom);
        }
    }
}

double EditorStackView::zoomMax()
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->zoomMax();
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            return preview->zoomMax();
        }
        else
        {
            return -1.0;
        }
    }
}

double EditorStackView::zoomMin()
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->zoomMin();
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            return preview->zoomMin();
        }
        else
        {
            return -1.0;
        }
    }
}

void EditorStackView::slotZoomSliderChanged(int size)
{
    if (viewMode() == ToolViewMode && !previewWidget())
    {
        return;
    }

    double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());

    if (viewMode() == CanvasMode)
    {
        d->canvas->setZoomFactorSnapped(z);
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            preview->setZoomFactorSnapped(z);
        }
    }
}

void EditorStackView::slotZoomChanged(double zoom)
{
    bool max, min;

    if (viewMode() == CanvasMode)
    {
        max = d->canvas->maxZoom();
        min = d->canvas->minZoom();
        emit signalZoomChanged(max, min, zoom);
    }
    else
    {
        PreviewWidget* preview = previewWidget();

        if (preview)
        {
            max = preview->maxZoom();
            min = preview->minZoom();
            emit signalZoomChanged(max, min, zoom);
        }
    }
}

PreviewWidget* EditorStackView::previewWidget() const
{
    PreviewWidget* preview = dynamic_cast<PreviewWidget*>(d->toolView);

    if (preview)
    {
        return preview;
    }

    return 0;
}

}  // namespace Digikam
