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
               : QStackedWidget(parent)
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
    insertWidget(CanvasMode, d->canvas);
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
        insertWidget(ToolViewMode, d->toolView);
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
        return;

    setCurrentIndex(mode);
}

}  // namespace Digikam
