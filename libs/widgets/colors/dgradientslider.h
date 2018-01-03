/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : a color gradient slider
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Cyrille Berger <cberger at cberger dot net>
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

#ifndef DGRADIENTSLIDER_H
#define DGRADIENTSLIDER_H

// Qt includes

#include <QWidget>
#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DGradientSlider : public QWidget
{
    Q_OBJECT

public:

    explicit DGradientSlider(QWidget* const parent=0);
    virtual ~DGradientSlider();

    void   showMiddleCursor(bool b);
    double leftValue() const;
    double rightValue() const;
    double middleValue() const;
    int    gradientOffset() const;

    void setColors(const QColor& lcolor, const QColor& rcolor);

public Q_SLOTS:

    void setLeftValue(double);
    void setRightValue(double);
    void setMiddleValue(double);

Q_SIGNALS:

    void leftValueChanged(double);
    void rightValueChanged(double);
    void middleValueChanged(double);

protected:

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void leaveEvent(QEvent*);

private:

    void adjustMiddleValue(double newLeftValue, double newRightValue);
    inline void drawCursorAt(QPainter& painter, double pos, const QColor& brushColor,
                             int width, int height, int gradientWidth);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DGRADIENTSLIDER_H
