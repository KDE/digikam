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

#ifndef EDITORSTACKVIEW_H
#define EDITORSTACKVIEW_H

// Qt includes

#include <QStackedWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class Canvas;
class GraphicsDImgView;

class DIGIKAM_EXPORT EditorStackView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackViewMode
    {
        CanvasMode = 0,
        ToolViewMode
    };

public:

    explicit EditorStackView(QWidget* const parent = 0);
    ~EditorStackView();

    void     setCanvas(Canvas* const canvas);
    Canvas*  canvas() const;

    void     setToolView(QWidget* const view);
    QWidget* toolView() const;

    void     setViewMode(int mode);
    int      viewMode() const;

    void     increaseZoom();
    void     decreaseZoom();
    void     toggleFitToWindow();
    void     fitToSelect();
    void     zoomTo100Percent();

    double   zoomMax()           const;
    double   zoomMin()           const;
    bool     isZoomablePreview() const;

Q_SIGNALS:

    void signalZoomChanged(bool isMax, bool isMin, double zoom);
    void signalToggleOffFitToWindow();

public Q_SLOTS:

    void setZoomFactor(double);
    void slotZoomSliderChanged(int);

private Q_SLOTS:

    void slotZoomChanged(double);
    void slotToggleOffFitToWindow(bool);

private:

    GraphicsDImgView* previewWidget() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* EDITORSTACKVIEW_H */
