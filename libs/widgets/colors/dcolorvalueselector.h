/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * Copyright (C)      1997 by Martin Jones <mjones at kde dot org>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DCOLORVALUESELECTOR_H
#define DCOLORVALUESELECTOR_H

// Qt includes

#include <QPixmap>
#include <QAbstractSlider>

// Local includes

#include "dcolorchoosermode.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * DSelector is the base class for other widgets which
 * provides the ability to choose from a one-dimensional
 * range of values. An example is the KGradientSelector
 * which allows to choose from a range of colors.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class DIGIKAM_EXPORT DSelector : public QAbstractSlider
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int minValue READ minimum WRITE setMinimum)
    Q_PROPERTY(int maxValue READ maximum WRITE setMaximum)
    Q_PROPERTY(bool indent READ indent WRITE setIndent)
    Q_PROPERTY(Qt::ArrowType arrowDirection READ arrowDirection WRITE setArrowDirection)

public:

    explicit DSelector(QWidget *parent=0);
    explicit DSelector(Qt::Orientation o, QWidget* const parent=0);
    ~DSelector();

    /**
    * @return the rectangle on which subclasses should draw.
    */
    QRect contentsRect() const;

    /**
    * Sets the indent option of the widget to i.
    * This determines whether a shaded frame is drawn.
    */
    void setIndent(bool i);

    /**
    * @return whether the indent option is set.
    */
    bool indent() const;

    /**
    * Sets the arrow direction.
    */
    void setArrowDirection(Qt::ArrowType direction);

    /**
    * @return the current arrow direction
    */
    Qt::ArrowType arrowDirection() const;

protected:

    /**
    * Override this function to draw the contents of the control.
    * The default implementation does nothing.
    *
    * Draw only within contentsRect().
    */
    virtual void drawContents(QPainter*) {};

    /**
    * Override this function to draw the cursor which
    * indicates the current value.
    */
    virtual void drawArrow(QPainter* painter, const QPoint& pos);

    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent*);

private:

    QPoint calcArrowPos(int val);
    void moveArrow(const QPoint& pos);

private:

    class Private;
    friend class Private;
    Private* const d;

    Q_DISABLE_COPY(DSelector)
};

// -----------------------------------------------------------------------------------

class DIGIKAM_EXPORT DColorValueSelector : public DSelector
{
    Q_OBJECT
    Q_PROPERTY( int hue        READ hue        WRITE setHue)
    Q_PROPERTY( int saturation READ saturation WRITE setSaturation)
    Q_PROPERTY( int colorValue READ colorValue WRITE setColorValue)

public:

    explicit DColorValueSelector(QWidget* const parent = 0);
    explicit DColorValueSelector(Qt::Orientation o, QWidget* const parent = 0);
    ~DColorValueSelector();

    /**
     * Updates the widget's contents.
     */
    void updateContents();

    /**
     * Returns the current hue value.
     *
     * @return The hue value (0-359)
     */
    int hue() const;

    /**
     * Sets the hue value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param hue Sets the hue value (0-359)
     */
    void setHue(int hue);

    /**
     * Returns the current saturation value.
     *
     * @return The saturation value (0-255)
     */
    int saturation() const;

    /**
     * Sets the saturation value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param saturation Sets the saturation value (0-255)
     */
    void setSaturation(int saturation);

    /**
     * Returns the current color value.
     *
     * @return The color value (0-255)
     */
    int colorValue() const;

    /**
     * Sets the color value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param colorValue Sets the color value (0-255)
     */
    void setColorValue(int colorValue);

    /**
     * Sets the chooser mode. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param chooserMode Sets the chooser mode (one of the DColorChooserMode constants)
     */
    void setChooserMode(DColorChooserMode chooserMode);

    /**
     * Returns the current chooser mode.
     *
     * @return The chooser mode (one of the DColorChooserMode constants)
     */
    DColorChooserMode chooserMode() const;

protected:

    /**
     * Draws the contents of the widget on a pixmap,
     * which is used for buffering.
     */
    virtual void drawPalette(QPixmap*);
    virtual void resizeEvent(QResizeEvent*);

    /**
     * Reimplemented from DSelector. The drawing is
     * buffered in a pixmap here. As real drawing
     * routine, drawPalette() is used.
     */
    virtual void drawContents(QPainter*);

private:

    class Private;
    friend class Private;
    Private *const d;

    Q_DISABLE_COPY(DColorValueSelector)
};

} // namespace Digikam

#endif // DCOLORVALUESELECTOR_H
