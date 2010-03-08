/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date  : 2005-01-18
 * Description : a widget class to edit perspective.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef PERSPECTIVEWIDGET_H
#define PERSPECTIVEWIDGET_H

// Qt includes

#include <QWidget>
#include <QPoint>
#include <QPolygon>
#include <QColor>
#include <QRect>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>

// Local includes

#include "dimg.h"
#include "matrix.h"

class QPixmap;

namespace Digikam
{
class ImageIface;
}

using namespace Digikam;

namespace DigikamTransformImagePlugin
{

class PerspectiveWidget : public QWidget
{
Q_OBJECT

public:

    PerspectiveWidget(int width, int height, QWidget* parent=0);
    ~PerspectiveWidget();

    QRect  getTargetSize();
    QPoint getTopLeftCorner();
    QPoint getTopRightCorner();
    QPoint getBottomLeftCorner();
    QPoint getBottomRightCorner();
    void   reset();

    float getAngleTopLeft();
    float getAngleTopRight();
    float getAngleBottomLeft();
    float getAngleBottomRight();

    void  applyPerspectiveAdjustment();

    ImageIface* imageIface();

public Q_SLOTS:

    void slotToggleAntiAliasing(bool a);
    void slotToggleDrawWhileMoving(bool draw);
    void slotToggleDrawGrid(bool grid);

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotInverseTransformationChanged(bool isEnabled);

Q_SIGNALS:

    void signalPerspectiveChanged(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                  float bottomLeftAngle, float bottomRightAngle, bool valid);

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:  // Widget methods.

    void   updatePixmap();

    void   transformAffine(DImg* orgImage, DImg* destImage,
                           const Matrix& matrix, DColor background);

    QPoint buildPerspective(const QPoint& orignTopLeft, const QPoint& orignBottomRight,
                            const QPoint& transTopLeft, const QPoint& transTopRight,
                            const QPoint& transBottomLeft, const QPoint& transBottomRight,
                            DImg* orgImage=0, DImg* destImage=0,
                            DColor background=DColor());

private:

    enum ResizingMode
    {
        ResizingNone = 0,
        ResizingTopLeft,
        ResizingTopRight,
        ResizingBottomLeft,
        ResizingBottomRight
    };

    bool        m_antiAlias;
    bool        m_drawWhileMoving;
    bool        m_drawGrid;
    bool        m_inverseTransformation;
    bool        m_validPerspective;

    uint*       m_data;
    int         m_w;
    int         m_h;
    int         m_origW;
    int         m_origH;

    int         m_currentResizing;

    int         m_guideSize;

    QRect       m_rect;

    // Transformed center area for mouse position control.

    QPoint      m_transformedCenter;

    // Draggable local region selection corners.

    QRect       m_topLeftCorner;
    QRect       m_topRightCorner;
    QRect       m_bottomLeftCorner;
    QRect       m_bottomRightCorner;

    QPoint      m_topLeftPoint;
    QPoint      m_topRightPoint;
    QPoint      m_bottomLeftPoint;
    QPoint      m_bottomRightPoint;
    QPoint      m_spot;

    QColor      m_guideColor;

    // 60 points will be stored to compute a grid of 15x15 lines.
    QPolygon    m_grid;

    QPixmap*    m_pixmap;

    ImageIface* m_iface;
    DImg        m_previewImage;
};

}  // namespace DigikamTransformImagePlugin

#endif /* PERSPECTIVEWIDGET_H */
