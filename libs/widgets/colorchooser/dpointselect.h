/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : 2D value selector
 *
 * Copyright (C) 1997 by Martin Jones <mjones at kde dot org>
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DPOINT_SELECT_H
#define DPOINT_SELECT_H

// Qt includes

#include <QWidget>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPointSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int xValue READ xValue WRITE setXValue)
    Q_PROPERTY(int yValue READ yValue WRITE setYValue)

public:

    /**
     * Constructs a two-dimensional selector widget which
     * has a value range of [0..100] in both directions.
     */
    explicit DPointSelect(QWidget* const parent=0);
    ~DPointSelect();

    /**
     * Sets the current values in horizontal and
     * vertical direction.
     * @param xPos the horizontal value
     * @param yPos the vertical value
     */
    void setValues(int xPos, int yPos);

    /**
     * Sets the current horizontal value
     * @param xPos the horizontal value
     */
    void setXValue(int xPos);

    /**
     * Sets the current vertical value
     * @param yPos the vertical value
     */
    void setYValue(int yPos);

    /**
     * Sets the range of possible values.
     */
    void setRange(int minX, int minY, int maxX, int maxY);

    /**
     * Sets the color used to draw the marker
     * @param col the color
     */
    void setMarkerColor(const QColor& col);

    /**
     * @return the current value in horizontal direction.
     */
    int xValue() const;

    /**
     * @return the current value in vertical direction.
     */
    int yValue() const;

    /**
     * @return the rectangle on which subclasses should draw.
     */
    QRect contentsRect() const;

    /**
     * Reimplemented to give the widget a minimum size
     */
    virtual QSize minimumSizeHint() const;

Q_SIGNALS:

    /**
     * This signal is emitted whenever the user chooses a value,
     * e.g. by clicking with the mouse on the widget.
     */
    void valueChanged(int x, int y);

protected:

    /**
     * Override this function to draw the contents of the widget.
     * The default implementation does nothing.
     *
     * Draw within contentsRect() only.
     */
    virtual void drawContents(QPainter*) {};

    /**
     * Override this function to draw the marker which
     * indicates the currently selected value pair.
     */
    virtual void drawMarker(QPainter* p, int xp, int yp);

    virtual void paintEvent(QPaintEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent*);

    /**
     * Converts a pixel position to its corresponding values.
     */
    void valuesFromPosition(int x, int y, int& xVal, int& yVal) const;

private:
    
    void setPosition(int xp, int yp);

private:
    
    class Private;
    friend class Private;
    Private* const d;
    
    Q_DISABLE_COPY(DPointSelect)
};

}  // namespace Digikam

#endif // DPOINT_SELECT_H
