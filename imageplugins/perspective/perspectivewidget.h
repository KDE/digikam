/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date  : 2005-01-18
 * Description : a widget class to edit perspective.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

namespace DigikamPerspectiveImagesPlugin
{

class PerspectiveWidget : public QWidget
{
Q_OBJECT

public:

    PerspectiveWidget(int width, int height, QWidget *parent=0);
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

    Digikam::ImageIface* imageIface();

public Q_SLOTS:

    void slotToggleAntiAliasing(bool a);
    void slotToggleDrawWhileMoving(bool draw);
    void slotToggleDrawGrid(bool grid);

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotInverseTransformationChanged(bool isEnabled);

Q_SIGNALS:

    void signalPerspectiveChanged(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                  float bottomLeftAngle, float bottomRightAngle);

protected:

    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );

private:  // Widget methods.

    void   updatePixmap();

    void   transformAffine(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                           const Matrix& matrix, Digikam::DColor background);

    QPoint buildPerspective(QPoint orignTopLeft, QPoint orignBottomRight,
                            QPoint transTopLeft, QPoint transTopRight,
                            QPoint transBottomLeft, QPoint transBottomRight,
                            Digikam::DImg *orgImage=0, Digikam::DImg *destImage=0,
                            Digikam::DColor background=Digikam::DColor());

private:

    enum ResizingMode
    {
        ResizingNone = 0,
        ResizingTopLeft,
        ResizingTopRight,
        ResizingBottomLeft,
        ResizingBottomRight
    };

    bool                 m_antiAlias;
    bool                 m_drawWhileMoving;
    bool                 m_drawGrid;
    bool                 m_inverseTransformation;

    uint                *m_data;
    int                  m_w;
    int                  m_h;
    int                  m_origW;
    int                  m_origH;

    int                  m_currentResizing;

    int                  m_guideSize;

    QRect                m_rect;

    // Transformed center area for mouse position control.

    QPoint               m_transformedCenter;

    // Draggable local region selection corners.

    QRect                m_topLeftCorner;
    QRect                m_topRightCorner;
    QRect                m_bottomLeftCorner;
    QRect                m_bottomRightCorner;

    QPoint               m_topLeftPoint;
    QPoint               m_topRightPoint;
    QPoint               m_bottomLeftPoint;
    QPoint               m_bottomRightPoint;
    QPoint               m_spot;

    QColor               m_guideColor;

    // 60 points will be stored to compute a grid of 15x15 lines.
    QPolygon             m_grid;

    QPixmap             *m_pixmap;

    Digikam::ImageIface *m_iface;
    Digikam::DImg        m_previewImage;
};

}  // namespace DigikamPerspectiveImagesPlugin

#endif /* PERSPECTIVEWIDGET_H */
