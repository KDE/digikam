/* ============================================================
 * File  : perspectivewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-18
 * Description : 
 * 
 * Copyright 2004 Gilles Caulier
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

class QPixmap;

namespace Digikam
{
class ImageIface;
}

namespace DigikamPerspectiveImagesPlugin
{

class Triangle
{
public:

    Triangle(QPoint A, QPoint B, QPoint C);
    ~Triangle(){};
    
    float angleABC(void);
    float angleACB(void);
    float angleBAC(void);
    
private:  
    
    float  m_a;
    float  m_b;
    float  m_c;
        
    float distanceP2P(QPoint p1, QPoint p2);
};

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
    void   resetSelection(void);
    
    float getAngleTopLeft(void);
    float getAngleTopRight(void);
    float getAngleBottomLeft(void);
    float getAngleBottomRight(void);
    
    void applyPerspectiveAdjusment(void);
    
    Digikam::ImageIface* imageIface();
    
signals:

    void signalPerspectiveChanged( QRect newSize, float topLeftAngle, float topRightAngle,
                                   float bottomLeftAngle, float bottomRightAngle );   

protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );

private:  // Matrix 3x3 perspective transformation implementations.
        
    // TODO : put these methods in a separate class.
    
    struct Matrix3
    {
    double coeff[3][3];
    };

    void   matrix3Identity(Matrix3 *matrix);
    void   matrix3Translate(Matrix3 *matrix, double x, double y);
    void   matrix3Scale(Matrix3 *matrix, double x, double y);
    void   matrix3Mult(const Matrix3 *matrix1, Matrix3 *matrix2);
    void   matrix3TransformPoint(const Matrix3 *matrix, double x, double y, double *newx, double *newy);
    void   matrix3Invert(Matrix3 *matrix);
    double matrix3Determinant(const Matrix3 *matrix);
    
private:  // Widget methods.

    enum ResizingMode
    {
    ResizingNone = 0,
    ResizingTopLeft,
    ResizingTopRight, 
    ResizingBottomLeft,   
    ResizingBottomRight
    };
    
    Digikam::ImageIface *m_iface;
    
    uint       *m_data;
    int         m_w;
    int         m_h;
    int         m_origW;
    int         m_origH;
    
    int         m_xpos;
    int         m_ypos;
    
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
    
    void   updatePixmap(void);
    void   transformAffine(uint *data, uint *newData, const Matrix3 *matrix, int w, int h);
    QPoint buildPerspective(QPoint orignTopLeft, QPoint orignBottomRight,
                            QPoint transTopLeft, QPoint transTopRight,
                            QPoint transBottomLeft, QPoint transBottomRight,
                            uint* data, uint* newData);
};

}  // NameSpace DigikamPerspectiveImagesPlugin

#endif /* PERSPECTIVEWIDGET_H */
