/* ============================================================
 * File  : perspectivewidget.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-01-18
 * Description : 
 * 
 * Copyright 2005 Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
 *
 * Matrix3 implementation inspired from gimp 2.0
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

// Qt includes.

#include <qwidget.h>
#include <qpoint.h>
#include <qrect.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes

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

    QRect  getTargetSize(void);
    QPoint getTopLeftCorner(void);
    QPoint getTopRightCorner(void);
    QPoint getBottomLeftCorner(void);
    QPoint getBottomRightCorner(void);
    void   reset(void);

    float getAngleTopLeft(void);
    float getAngleTopRight(void);
    float getAngleBottomLeft(void);
    float getAngleBottomRight(void);

    void  applyPerspectiveAdjustment(void);

    Digikam::ImageIface* imageIface();

public slots:

    void toggleAntiAliasing(bool a);
    void toggleDrawWhileMoving(bool draw);

signals:

    void signalPerspectiveChanged( QRect newSize, float topLeftAngle, float topRightAngle,
                                   float bottomLeftAngle, float bottomRightAngle );   

protected:

    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );

private:

    enum ResizingMode
    {
        ResizingNone = 0,
        ResizingTopLeft,
        ResizingTopRight,
        ResizingBottomLeft,
        ResizingBottomRight
    };

    Digikam::ImageIface *m_iface;

    bool        m_antiAlias;
    bool        m_drawWhileMoving;

    uint       *m_data;
    int         m_w;
    int         m_h;
    int         m_origW;
    int         m_origH;

    int         m_currentResizing;

    QRect       m_rect;

    // Tranformed center area for mouse position control.

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

    QPixmap*    m_pixmap;
    Digikam::DImg m_previewImage;

private:  // Widget methods.

    void   updatePixmap(void);
    void   transformAffine(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                           const Matrix &matrix, Digikam::DColor background);
    QPoint buildPerspective(QPoint orignTopLeft, QPoint orignBottomRight,
                            QPoint transTopLeft, QPoint transTopRight,
                            QPoint transBottomLeft, QPoint transBottomRight,
                            Digikam::DImg *orgImage, Digikam::DImg *destImage,
                            Digikam::DColor background);
};

}  // NameSpace DigikamPerspectiveImagesPlugin

#endif /* PERSPECTIVEWIDGET_H */
