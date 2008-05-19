/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
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

#ifndef SKETCHWIDGET_H
#define SKETCHWIDGET_H

// Qt includes.

#include <QWidget>

namespace Digikam
{

class SketchWidgetPriv;

class SketchWidget : public QWidget
{
    Q_OBJECT

public:

    SketchWidget(QWidget *parent=0);
    ~SketchWidget();

    void   setPenColor(const QColor& newColor);
    QColor penColor() const;

    void   setPenWidth(int newWidth);
    int    penWidth() const;

public slots:

    void clearSketch();

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

private:

    void drawLineTo(const QPoint& endPoint);

private:

    SketchWidgetPriv *d;
};

}  // namespace Digikam

#endif // SKETCHWIDGET_H
