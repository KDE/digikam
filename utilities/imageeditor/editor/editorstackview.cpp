/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "previewwidget.h"
#include "imageregionwidget.h"
#include "imagepanelwidget.h"
#include "canvas.h"
#include "editorstackview.h"
#include "editorstackview.moc"

namespace Digikam
{

class EditorStackViewPriv
{

public:

    EditorStackViewPriv()
    {
        canvas   = 0;
        toolView = 0;
    }

    QWidget *toolView;
    Canvas  *canvas;
};

EditorStackView::EditorStackView(QWidget *parent)
               : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new EditorStackViewPriv;
}

EditorStackView::~EditorStackView()
{
    delete d;
}

void EditorStackView::setCanvas(Canvas* canvas)
{
    if (d->canvas) return;

    d->canvas = canvas;
    addWidget(d->canvas, CanvasMode);

    connect(d->canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));
}

Canvas* EditorStackView::canvas() const
{
    return d->canvas;
}

void EditorStackView::setToolView(QWidget* view)
{
    if (d->toolView)
        removeWidget(d->toolView);

    d->toolView = view;

    if (d->toolView)
        addWidget(d->toolView, ToolViewMode);

    PreviewWidget *preview = previewWidget();
    if (preview)
    {
        connect(preview, SIGNAL(signalZoomFactorChanged(double)),
                this, SLOT(slotZoomChanged(double)));
    }
}

QWidget* EditorStackView::toolView() const
{
    return d->toolView;
}

int EditorStackView::viewMode()
{
    return id(visibleWidget());
}

void EditorStackView::setViewMode(int mode)
{
    if (mode != CanvasMode && mode != ToolViewMode)
        return;

    raiseWidget(mode);
}

void EditorStackView::increaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->slotIncreaseZoom();
    }
    else
    {
        PreviewWidget *preview = previewWidget();
        if (preview)
            preview->slotIncreaseZoom();
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
        PreviewWidget *preview = previewWidget();
        if (preview)
            preview->slotDecreaseZoom();
    }
}

void EditorStackView::toggleFitToWindow()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->toggleFitToWindow();
    }
    else
    {
        PreviewWidget *preview = previewWidget();
        if (preview)
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

void EditorStackView::zoomTo100Percents()
{
    if (viewMode() == CanvasMode)
    {
        if (d->canvas->zoomFactor() == 1.0)
            d->canvas->toggleFitToWindow();
        else
            d->canvas->setZoomFactor(1.0);
    }
    else
    {
        PreviewWidget *preview = previewWidget();
        if (preview)
        {
            if (preview->zoomFactor() == 1.0)
                preview->toggleFitToWindow();
            else
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
        PreviewWidget *preview = previewWidget();
        if (preview)
            preview->setZoomFactor(zoom);
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
        PreviewWidget *preview = previewWidget();
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
    PreviewWidget *preview = dynamic_cast<PreviewWidget*>(d->toolView);
    if (preview) return preview;

    ImagePanelWidget *panel = dynamic_cast<ImagePanelWidget*>(d->toolView);
    if (panel) return (dynamic_cast<PreviewWidget*>(panel->previewWidget()));

    return 0;
}

}  // namespace Digikam
