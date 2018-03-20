/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-18
 * Description : a widget class to edit perspective.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "perspectivewidget.h"

// C++ includes

#include <cstdio>
#include <cstdlib>
#include <cmath>

// Qt includes

#include <QRegion>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QImage>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QPolygon>


// Local includes

#include "digikam_debug.h"
#include "triangle.h"
#include "imageiface.h"
#include "pixelsaliasfilter.h"

namespace Digikam
{

class PerspectiveWidget::Private
{
public:

    enum ResizingMode
    {
        ResizingNone = 0,
        ResizingTopLeft,
        ResizingTopRight,
        ResizingBottomLeft,
        ResizingBottomRight
    };

    Private() :
        antiAliasing(false),
        drawWhileMoving(true),
        drawGrid(false),
        inverseTransformation(false),
        validPerspective(true),
        data(0),
        width(0),
        height(0),
        origW(0),
        origH(0),
        currentResizing(ResizingNone),
        guideSize(1),
        guideColor(Qt::red),
        pixmap(0),
        iface(0)
    {
    }

    bool        antiAliasing;
    bool        drawWhileMoving;
    bool        drawGrid;
    bool        inverseTransformation;
    bool        validPerspective;

    uint*       data;
    int         width;
    int         height;
    int         origW;
    int         origH;

    int         currentResizing;

    int         guideSize;

    QRect       rect;

    // Transformed center area for mouse position control.

    QPoint      transformedCenter;

    // Draggable local region selection corners.

    QRect       topLeftCorner;
    QRect       topRightCorner;
    QRect       bottomLeftCorner;
    QRect       bottomRightCorner;

    QPoint      topLeftPoint;
    QPoint      topRightPoint;
    QPoint      bottomLeftPoint;
    QPoint      bottomRightPoint;
    QPoint      spot;

    QColor      guideColor;
    QColor      bgColor;

    // 60 points will be stored to compute a grid of 15x15 lines.
    QPolygon    grid;

    QPixmap*    pixmap;

    ImageIface* iface;
    DImg        preview;
};

PerspectiveWidget::PerspectiveWidget(int w, int h, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(w, h);
    setMouseTracking(true);

    d->bgColor  = palette().color(QPalette::Background);
    d->iface    = new ImageIface(QSize(w, h));
    d->preview  = d->iface->setPreviewSize(QSize(w, h));
    d->width    = d->iface->previewSize().width();
    d->height   = d->iface->previewSize().height();
    d->origW    = d->iface->originalSize().width();
    d->origH    = d->iface->originalSize().height();
    d->preview.setIccProfile( d->iface->original()->getIccProfile() );

    d->pixmap   = new QPixmap(w, h);
    d->rect     = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    d->grid     = QPolygon(60);

    reset();
}

PerspectiveWidget::~PerspectiveWidget()
{
    delete d->iface;
    delete d->pixmap;
    delete d;
}

void PerspectiveWidget::resizeEvent(QResizeEvent* e)
{
    int old_w     = d->width;
    int old_h     = d->height;

    delete d->pixmap;
    int w         = e->size().width();
    int h         = e->size().height();
    d->preview    = d->iface->setPreviewSize(QSize(w, h));
    d->width      = d->iface->previewSize().width();
    d->height     = d->iface->previewSize().height();
    d->preview.setIccProfile( d->iface->original()->getIccProfile() );

    d->pixmap     = new QPixmap(w, h);
    QRect oldRect = d->rect;
    d->rect       = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

    float xFactor = (float)d->rect.width()/(float)(oldRect.width());
    float yFactor = (float)d->rect.height()/(float)(oldRect.height());

    d->topLeftPoint      = QPoint(lroundf(d->topLeftPoint.x()*xFactor),
                                  lroundf(d->topLeftPoint.y()*yFactor));
    d->topRightPoint     = QPoint(lroundf(d->topRightPoint.x()*xFactor),
                                  lroundf(d->topRightPoint.y()*yFactor));
    d->bottomLeftPoint   = QPoint(lroundf(d->bottomLeftPoint.x()*xFactor),
                                  lroundf(d->bottomLeftPoint.y()*yFactor));
    d->bottomRightPoint  = QPoint(lroundf(d->bottomRightPoint.x()*xFactor),
                                  lroundf(d->bottomRightPoint.y()*yFactor));
    d->transformedCenter = QPoint(lroundf(d->transformedCenter.x()*xFactor),
                                  lroundf(d->transformedCenter.y()*yFactor));

    d->spot.setX((int)((float)d->spot.x() * ( (float)d->width / (float)old_w)));
    d->spot.setY((int)((float)d->spot.y() * ( (float)d->height / (float)old_h)));

    updatePixmap();
}

ImageIface* PerspectiveWidget::imageIface() const
{
    return d->iface;
}

QPoint PerspectiveWidget::getTopLeftCorner() const
{
    return QPoint( lroundf((float)(d->topLeftPoint.x()*d->origW) / (float)d->width),
                   lroundf((float)(d->topLeftPoint.y()*d->origH) / (float)d->height));
}

QPoint PerspectiveWidget::getTopRightCorner() const
{
    return QPoint( lroundf((float)(d->topRightPoint.x()*d->origW) / (float)d->width),
                   lroundf((float)(d->topRightPoint.y()*d->origH) / (float)d->height));
}

QPoint PerspectiveWidget::getBottomLeftCorner() const
{
    return QPoint( lroundf((float)(d->bottomLeftPoint.x()*d->origW) / (float)d->width),
                   lroundf((float)(d->bottomLeftPoint.y()*d->origH) / (float)d->height));
}

QPoint PerspectiveWidget::getBottomRightCorner() const
{
    return QPoint( lroundf((float)(d->bottomRightPoint.x()*d->origW) / (float)d->width),
                   lroundf((float)(d->bottomRightPoint.y()*d->origH) / (float)d->height));
}

QRect PerspectiveWidget::getTargetSize() const
{
    QPolygon perspectiveArea;

    perspectiveArea.putPoints(0, 4,
                              getTopLeftCorner().x(),     getTopLeftCorner().y(),
                              getTopRightCorner().x(),    getTopRightCorner().y(),
                              getBottomRightCorner().x(), getBottomRightCorner().y(),
                              getBottomLeftCorner().x(),  getBottomLeftCorner().y());

    return perspectiveArea.boundingRect();
}

float PerspectiveWidget::getAngleTopLeft() const
{
    Triangle topLeft(getTopLeftCorner(), getTopRightCorner(), getBottomLeftCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleTopRight() const
{
    Triangle topLeft(getTopRightCorner(), getBottomRightCorner(), getTopLeftCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleBottomLeft() const
{
    Triangle topLeft(getBottomLeftCorner(), getTopLeftCorner(), getBottomRightCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleBottomRight() const
{
    Triangle topLeft(getBottomRightCorner(), getBottomLeftCorner(), getTopRightCorner());
    return topLeft.angleBAC();
}

void PerspectiveWidget::reset()
{
    d->topLeftPoint.setX(0);
    d->topLeftPoint.setY(0);

    d->topRightPoint.setX(d->width-1);
    d->topRightPoint.setY(0);

    d->bottomLeftPoint.setX(0);
    d->bottomLeftPoint.setY(d->height-1);

    d->bottomRightPoint.setX(d->width-1);
    d->bottomRightPoint.setY(d->height-1);

    d->spot.setX(d->width / 2);
    d->spot.setY(d->height / 2);

    d->antiAliasing = true;
    updatePixmap();
    update();
}

void PerspectiveWidget::applyPerspectiveAdjustment()
{
    DImg* const orgImage = d->iface->original();

    if (!orgImage) return;

    DImg destImage(orgImage->width(), orgImage->height(), orgImage->sixteenBit(), orgImage->hasAlpha());

    DColor background(0, 0, 0, orgImage->hasAlpha() ? 0 : 255, orgImage->sixteenBit());

    // Perform perspective adjustment.

    buildPerspective(QPoint(0, 0), QPoint(d->origW, d->origH),
                     getTopLeftCorner(), getTopRightCorner(),
                     getBottomLeftCorner(), getBottomRightCorner(),
                     orgImage, &destImage, background);

    // Perform an auto-cropping around the image.

    DImg targetImg = destImage.copy(getTargetSize());

    FilterAction action(QLatin1String("digikam:PerspectiveAdjustment"), 1);
    action.setDisplayableName(i18n("Perspective Adjustment Tool"));

    action.addParameter(QLatin1String("topLeftPointX"),     d->topLeftPoint.x());
    action.addParameter(QLatin1String("topLeftPointY"),     d->topLeftPoint.y());
    action.addParameter(QLatin1String("topRightPointX"),    d->topRightPoint.x());
    action.addParameter(QLatin1String("topRightPointY"),    d->topRightPoint.y());

    action.addParameter(QLatin1String("bottomLeftPointX"),  d->bottomLeftPoint.x());
    action.addParameter(QLatin1String("bottomLeftPointY"),  d->bottomLeftPoint.y());
    action.addParameter(QLatin1String("bottomRightPointX"), d->bottomRightPoint.x());
    action.addParameter(QLatin1String("bottomRightPointY"), d->bottomRightPoint.y());

    action.addParameter(QLatin1String("spotX"),             d->spot.x());
    action.addParameter(QLatin1String("spotY"),             d->spot.y());

    action.addParameter(QLatin1String("antiAliasing"),      d->antiAliasing);
    // Update target image.
    d->iface->setOriginal(i18n("Perspective Adjustment"), action, targetImg);
}

void PerspectiveWidget::slotInverseTransformationChanged(bool isEnabled)
{
    d->inverseTransformation = isEnabled;
    updatePixmap();
    update();
}

void PerspectiveWidget::slotToggleAntiAliasing(bool a)
{
    d->antiAliasing = a;
    updatePixmap();
    update();
}

void PerspectiveWidget::slotToggleDrawWhileMoving(bool draw)
{
    d->drawWhileMoving = draw;
}

void PerspectiveWidget::slotToggleDrawGrid(bool grid)
{
    d->drawGrid = grid;
    updatePixmap();
    update();
}

void PerspectiveWidget::slotChangeGuideColor(const QColor& color)
{
    d->guideColor = color;
    updatePixmap();
    update();
}

void PerspectiveWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePixmap();
    update();
}

void PerspectiveWidget::setBackgroundColor(const QColor& bg)
{
    d->bgColor = bg;
    updatePixmap();
    update();
}

void PerspectiveWidget::updatePixmap()
{
    d->topLeftCorner.setRect(d->topLeftPoint.x() + d->rect.topLeft().x(),
                             d->topLeftPoint.y() + d->rect.topLeft().y(), 8, 8);
    d->topRightCorner.setRect(d->topRightPoint.x() - 7 + d->rect.topLeft().x(),
                              d->topRightPoint.y() + d->rect.topLeft().y(), 8, 8);
    d->bottomLeftCorner.setRect(d->bottomLeftPoint.x() + d->rect.topLeft().x(),
                                d->bottomLeftPoint.y() - 7 + d->rect.topLeft().y(), 8, 8);
    d->bottomRightCorner.setRect(d->bottomRightPoint.x() - 7 + d->rect.topLeft().x(),
                                 d->bottomRightPoint.y() - 7 + d->rect.topLeft().y(), 8, 8);

    // Compute the grid array

    int gXS = d->width  / 15;
    int gYS = d->height / 15;

    for (int i = 0 ; i < 15 ; ++i)
    {
        int j = i*4;

        // Horizontal line.
        d->grid.setPoint(j  , 0,        i*gYS);
        d->grid.setPoint(j+1, d->width, i*gYS);

        // Vertical line.
        d->grid.setPoint(j+2, i*gXS, 0);
        d->grid.setPoint(j+3, i*gXS, d->height);
    }

    // Draw background

    d->pixmap->fill(d->bgColor);

    if (d->inverseTransformation)
    {
        d->transformedCenter = buildPerspective(QPoint(0, 0), QPoint(d->width, d->height),
                                                d->topLeftPoint, d->topRightPoint,
                                                d->bottomLeftPoint, d->bottomRightPoint);

        d->iface->setPreview(d->preview);
        d->iface->paint(d->pixmap, d->rect);
    }
    // if we are resizing with the mouse, compute and draw only if drawWhileMoving is set
    else if ((d->currentResizing == Private::ResizingNone || d->drawWhileMoving) &&
             d->validPerspective)
    {
        // Create preview image

        DImg destImage(d->preview.width(), d->preview.height(),
                       d->preview.sixteenBit(), d->preview.hasAlpha());

        DColor background(d->bgColor);

        d->transformedCenter = buildPerspective(QPoint(0, 0), QPoint(d->width, d->height),
                                                d->topLeftPoint, d->topRightPoint,
                                                d->bottomLeftPoint, d->bottomRightPoint,
                                                &d->preview, &destImage, background);

        d->iface->setPreview(destImage);

        // Draw image

        d->iface->paint(d->pixmap, d->rect);
    }
    else if (d->validPerspective)
    {
        d->transformedCenter = buildPerspective(QPoint(0, 0), QPoint(d->width, d->height),
                                                d->topLeftPoint, d->topRightPoint,
                                                d->bottomLeftPoint, d->bottomRightPoint);
    }

    // Drawing selection borders.

    QPainter p(d->pixmap);
    p.setPen(QPen(QColor(255, 64, 64), 1, Qt::SolidLine));
    p.drawLine(d->topLeftPoint     + d->rect.topLeft(), d->topRightPoint    + d->rect.topLeft());
    p.drawLine(d->topRightPoint    + d->rect.topLeft(), d->bottomRightPoint + d->rect.topLeft());
    p.drawLine(d->bottomRightPoint + d->rect.topLeft(), d->bottomLeftPoint  + d->rect.topLeft());
    p.drawLine(d->bottomLeftPoint  + d->rect.topLeft(), d->topLeftPoint     + d->rect.topLeft());

    // Drawing selection corners.

    QBrush brush(QColor(255, 64, 64));
    p.fillRect(d->topLeftCorner,     brush);
    p.fillRect(d->topRightCorner,    brush);
    p.fillRect(d->bottomLeftCorner,  brush);
    p.fillRect(d->bottomRightCorner, brush);

    // Drawing the grid.

    if (d->drawGrid)
    {
        for (int i = 0 ; i < d->grid.size() ; i += 4)
        {
            // Horizontal line.
            p.drawLine(d->grid.point(i)+d->rect.topLeft(), d->grid.point(i+1)+d->rect.topLeft());

            // Vertical line.
            p.drawLine(d->grid.point(i+2)+d->rect.topLeft(), d->grid.point(i+3)+d->rect.topLeft());
        }
    }

    // Drawing transformed center.

    p.setPen(QPen(QColor(255, 64, 64), 3, Qt::SolidLine));
    p.drawEllipse( d->transformedCenter.x()+d->rect.topLeft().x()-2,
                   d->transformedCenter.y()+d->rect.topLeft().y()-2, 4, 4 );

    // Drawing vertical and horizontal guide lines.

    if (!d->inverseTransformation)
    {
        int xspot = d->spot.x() + d->rect.x();
        int yspot = d->spot.y() + d->rect.y();
        p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
        p.drawLine(xspot, d->rect.top(), xspot, d->rect.bottom());
        p.drawLine(d->rect.left(), yspot, d->rect.right(), yspot);
        p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
        p.drawLine(xspot, d->rect.top(), xspot, d->rect.bottom());
        p.drawLine(d->rect.left(), yspot, d->rect.right(), yspot);
    }

    p.end();

    emit signalPerspectiveChanged(getTargetSize(), getAngleTopLeft(), getAngleTopRight(),
                                  getAngleBottomLeft(), getAngleBottomRight(), d->validPerspective);
}

QPoint PerspectiveWidget::buildPerspective(const QPoint& orignTopLeft, const QPoint& orignBottomRight,
                                           const QPoint& transTopLeft, const QPoint& transTopRight,
                                           const QPoint& transBottomLeft, const QPoint& transBottomRight,
                                           DImg* const orgImage, DImg* const destImage,
                                           const DColor& background)
{
    Matrix matrix, transform;
    double scalex;
    double scaley;

    double x1 = (double)orignTopLeft.x();
    double y1 = (double)orignTopLeft.y();

    double x2 = (double)orignBottomRight.x();
    double y2 = (double)orignBottomRight.y();

    double tx1 = (double)transTopLeft.x();
    double ty1 = (double)transTopLeft.y();

    double tx2 = (double)transTopRight.x();
    double ty2 = (double)transTopRight.y();

    double tx3 = (double)transBottomLeft.x();
    double ty3 = (double)transBottomLeft.y();

    double tx4 = (double)transBottomRight.x();
    double ty4 = (double)transBottomRight.y();

    scalex = scaley = 1.0;

    if ((x2 - x1) > 0)
    {
        scalex = 1.0 / (double) (x2 - x1);
    }

    if ((y2 - y1) > 0)
    {
        scaley = 1.0 / (double) (y2 - y1);
    }

    // Determine the perspective transform that maps from
    // the unit cube to the transformed coordinates

    double dx1, dx2, dx3, dy1, dy2, dy3;

    dx1 = tx2 - tx4;
    dx2 = tx3 - tx4;
    dx3 = tx1 - tx2 + tx4 - tx3;

    dy1 = ty2 - ty4;
    dy2 = ty3 - ty4;
    dy3 = ty1 - ty2 + ty4 - ty3;

    //  Is the mapping affine?

    if ((dx3 == 0.0) && (dy3 == 0.0))
    {
        matrix.coeff[0][0] = tx2 - tx1;
        matrix.coeff[0][1] = tx4 - tx2;
        matrix.coeff[0][2] = tx1;
        matrix.coeff[1][0] = ty2 - ty1;
        matrix.coeff[1][1] = ty4 - ty2;
        matrix.coeff[1][2] = ty1;
        matrix.coeff[2][0] = 0.0;
        matrix.coeff[2][1] = 0.0;
    }
    else
    {
        double det1, det2;

        det1 = dx3 * dy2 - dy3 * dx2;
        det2 = dx1 * dy2 - dy1 * dx2;

        if (det1 == 0.0 && det2 == 0.0)
        {
            matrix.coeff[2][0] = 1.0;
        }
        else
        {
            matrix.coeff[2][0] = det1 / det2;
        }

        det1 = dx1 * dy3 - dy1 * dx3;

        if (det1 == 0.0 && det2 == 0.0)
        {
            matrix.coeff[2][1] = 1.0;
        }
        else
        {
            matrix.coeff[2][1] = det1 / det2;
        }

        matrix.coeff[0][0] = tx2 - tx1 + matrix.coeff[2][0] * tx2;
        matrix.coeff[0][1] = tx3 - tx1 + matrix.coeff[2][1] * tx3;
        matrix.coeff[0][2] = tx1;

        matrix.coeff[1][0] = ty2 - ty1 + matrix.coeff[2][0] * ty2;
        matrix.coeff[1][1] = ty3 - ty1 + matrix.coeff[2][1] * ty3;
        matrix.coeff[1][2] = ty1;
    }

    matrix.coeff[2][2] = 1.0;

    // transform is initialized to the identity matrix
    transform.translate(-x1, -y1);
    transform.scale    (scalex, scaley);
    transform.multiply (matrix);

    if (orgImage && destImage)
    {
        if (d->inverseTransformation)
        {
            Matrix inverseTransform = transform;
            inverseTransform.invert();

            //Transform the matrix so it puts the result into the getTargetSize() rectangle
            Matrix transformIntoBounds;
            transformIntoBounds.scale(double(getTargetSize().width()) / double(orgImage->width()), double(getTargetSize().height()) / double(orgImage->height()));
            transformIntoBounds.translate(getTargetSize().left(), getTargetSize().top());
            inverseTransform.multiply(transformIntoBounds);
            transformAffine(orgImage, destImage, inverseTransform, background);
        }
        else
        {
            // Compute perspective transformation to image if image data containers exist.
            transformAffine(orgImage, destImage, transform, background);
        }
    }

    // Calculate the grid array points.
    double newX, newY;

    for (int i = 0 ; i < d->grid.size() ; ++i)
    {
        transform.transformPoint(d->grid.point(i).x(), d->grid.point(i).y(), &newX, &newY);
        d->grid.setPoint(i, lround(newX), lround(newY));
    }

    // Calculate and return new image center.
    double newCenterX, newCenterY;
    transform.transformPoint(x2/2.0, y2/2.0, &newCenterX, &newCenterY);

    return QPoint(lround(newCenterX), lround(newCenterY));
}

void PerspectiveWidget::transformAffine(DImg* const orgImage, DImg* const destImage,
                                        const Matrix& matrix, const DColor& background)
{
    Matrix m(matrix);

    int    x1,   y1, x2, y2;     // target bounding box
    int    x,    y;              // target coordinates
    int    u1,   v1, u2, v2;     // source bounding box
    double uinc, vinc, winc;     // increments in source coordinates
                                 // per horizontal target coordinate

    double u[5] = {0.0};         // source coordinates,
    double v[5] = {0.0};         //   2
                                 //  / \    0 is sample in the center of pixel
                                 // 1 0 3   1..4 is offset 1 pixel in each
                                 //  \ /    direction (in target space)
                                 //   4

    double tu[5],tv[5],tw[5];    // undivided source coordinates and divisor

    uchar* data = 0, *newData = 0;
    bool   sixteenBit;
    int    coords;
    int    width, height;
    int    bytesDepth;
    int    offset;
    uchar* d2 = 0;
    DColor color;

    bytesDepth  = orgImage->bytesDepth();
    data        = orgImage->bits();
    sixteenBit  = orgImage->sixteenBit();
    width       = orgImage->width();
    height      = orgImage->height();
    newData     = destImage->bits();
    DColor bg   = background;

    if (sixteenBit)
    {
        bg.convertToSixteenBit();
    }

    //destImage->fill(bg);

    PixelsAliasFilter alias;

    // Find the inverse of the transformation matrix
    m.invert();

    u1 = 0;
    v1 = 0;
    u2 = u1 + width;
    v2 = v1 + height;

    x1 = u1;
    y1 = v1;
    x2 = u2;
    y2 = v2;

    QScopedArrayPointer<uchar> dest(new uchar[width * bytesDepth]);

    uinc = m.coeff[0][0];
    vinc = m.coeff[1][0];
    winc = m.coeff[2][0];

    coords = 1;

    // these loops could be rearranged, depending on which bit of code
    // you'd most like to write more than once.

    for (y = y1; y < y2; ++y)
    {
        // set up inverse transform steps

        tu[0] = uinc * (x1 + 0.5) + m.coeff[0][1] * (y + 0.5) + m.coeff[0][2] - 0.5;
        tv[0] = vinc * (x1 + 0.5) + m.coeff[1][1] * (y + 0.5) + m.coeff[1][2] - 0.5;
        tw[0] = winc * (x1 + 0.5) + m.coeff[2][1] * (y + 0.5) + m.coeff[2][2];

        d2 = dest.data();

        for (x = x1; x < x2; ++x)
        {
            int i;     //  normalize homogeneous coords

            for (i = 0; i < coords; ++i)
            {
                if (tw[i] == 1.0)
                {
                    u[i] = tu[i];
                    v[i] = tv[i];
                }
                else if (tw[i] != 0.0)
                {
                    u[i] = tu[i] / tw[i];
                    v[i] = tv[i] / tw[i];
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "homogeneous coordinate = 0...\n";
                }
            }

            //  Set the destination pixels

            int iu = lround( u [0] );
            int iv = lround( v [0] );

            if (iu >= u1 && iu < u2 && iv >= v1 && iv < v2)
            {
                // u, v coordinates into source

                //In inverse transformation we always enable anti-aliasing, because there is always under-sampling
                if (d->antiAliasing || d->inverseTransformation)
                {
                    double finalU = u[0] - u1;
                    double finalV = v[0] - v1;

                    if (sixteenBit)
                    {
                        unsigned short* d16 = reinterpret_cast<unsigned short*>(d2);
                        alias.pixelAntiAliasing16(reinterpret_cast<unsigned short*>(data), width, height, finalU, finalV, d16+3, d16+2, d16+1, d16);
                    }
                    else
                    {
                        alias.pixelAntiAliasing(data, width, height, finalU, finalV, d2+3, d2+2, d2+1, d2);
                    }
                }
                else
                {
                    int u  = iu - u1;
                    int v  = iv - v1;
                    offset = (v * width * bytesDepth) + (u * bytesDepth);
                    color.setColor(data + offset, sixteenBit);
                    color.setPixel(d2);
                }

                d2 += bytesDepth;
            }
            else // not in source range
            {
                // set to background color

                bg.setPixel(d2);
                d2 += bytesDepth;
            }

            for (i = 0; i < coords; ++i)
            {
                tu[i] += uinc;
                tv[i] += vinc;
                tw[i] += winc;
            }
        }

        //  set the pixel region row

        offset = (y - y1) * width * bytesDepth;
        memcpy(newData + offset, dest.data(), width * bytesDepth);
    }
}

void PerspectiveWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);
    p.end();
}

void PerspectiveWidget::mousePressEvent(QMouseEvent* e)
{
    if ( e->button() == Qt::LeftButton && d->rect.contains( e->x(), e->y() ))
    {
        if ( d->topLeftCorner.contains( e->x(), e->y() ) )
        {
            d->currentResizing = Private::ResizingTopLeft;
        }
        else if ( d->bottomRightCorner.contains( e->x(), e->y() ) )
        {
            d->currentResizing = Private::ResizingBottomRight;
        }
        else if ( d->topRightCorner.contains( e->x(), e->y() ) )
        {
            d->currentResizing = Private::ResizingTopRight;
        }
        else if ( d->bottomLeftCorner.contains( e->x(), e->y() ) )
        {
            d->currentResizing = Private::ResizingBottomLeft;
        }
        else
        {
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
        }
    }
}

void PerspectiveWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if ( d->currentResizing != Private::ResizingNone )
    {
        unsetCursor();
        d->currentResizing = Private::ResizingNone;

        // in this case, the pixmap has not been drawn on mouse move
        if (!d->drawWhileMoving)
        {
            updatePixmap();
            update();
        }
    }
    else
    {
        d->spot.setX(e->x()-d->rect.x());
        d->spot.setY(e->y()-d->rect.y());
        updatePixmap();
        update();
    }
}

void PerspectiveWidget::mouseMoveEvent(QMouseEvent* e)
{
    d->validPerspective = true;

    if ( e->buttons() == Qt::LeftButton )
    {
        if ( d->currentResizing != Private::ResizingNone )
        {
            QPolygon unusablePoints;
            QPoint pm(e->x(), e->y());

            if (!d->rect.contains( pm ))
            {
                if (pm.x() > d->rect.right())
                {
                    pm.setX(d->rect.right());
                }
                else if (pm.x() < d->rect.left())
                {
                    pm.setX(d->rect.left());
                }

                if (pm.y() > d->rect.bottom())
                {
                    pm.setY(d->rect.bottom());
                }
                else if (pm.y() < d->rect.top())
                {
                    pm.setY(d->rect.top());
                }
            }

            if ( d->currentResizing == Private::ResizingTopLeft )
            {
                d->topLeftPoint = pm - d->rect.topLeft();
                setCursor( Qt::SizeFDiagCursor );

                unusablePoints.putPoints(0, 7,
                                         d->width-1 + d->rect.x(),              d->height-1 + d->rect.y(),
                                         0 + d->rect.x(),                       d->height-1 + d->rect.y(),
                                         0 + d->rect.x(),                       d->bottomLeftPoint.y()-10 + d->rect.y(),
                                         d->bottomLeftPoint.x() + d->rect.x(),  d->bottomLeftPoint.y()-10 + d->rect.y(),
                                         d->topRightPoint.x()-10 + d->rect.x(), d->topRightPoint.y() + d->rect.y(),
                                         d->topRightPoint.x()-10 + d->rect.x(), 0 + d->rect.y(),
                                         d->width-1 + d->rect.x(),              0 + d->rect.y());
                QRegion unusableArea(unusablePoints);

                if ( unusableArea.contains(pm) && !d->inverseTransformation )
                {
                    d->validPerspective = false;
                }
            }

            else if ( d->currentResizing == Private::ResizingTopRight )
            {
                d->topRightPoint = pm - d->rect.topLeft();
                setCursor( Qt::SizeBDiagCursor );

                unusablePoints.putPoints(0, 7,
                                         0 + d->rect.x(),                       d->height-1 + d->rect.y(),
                                         0 + d->rect.x(),                       0 + d->rect.y(),
                                         d->topLeftPoint.x()+10 + d->rect.x(),  0 + d->rect.y(),
                                         d->topLeftPoint.x()+10 + d->rect.x(),  d->topLeftPoint.y() + d->rect.y(),
                                         d->bottomRightPoint.x() + d->rect.x(), d->bottomRightPoint.y()-10 + d->rect.y(),
                                         d->width-1 + d->rect.x(),              d->bottomRightPoint.y()-10 + d->rect.y(),
                                         d->width-1 + d->rect.x(),              d->height-1 + d->rect.y());
                QRegion unusableArea(unusablePoints);

                if ( unusableArea.contains(pm) && !d->inverseTransformation )
                {
                    d->validPerspective = false;
                }
            }

            else if ( d->currentResizing == Private::ResizingBottomLeft  )
            {
                d->bottomLeftPoint = pm - d->rect.topLeft();
                setCursor( Qt::SizeBDiagCursor );

                unusablePoints.putPoints(0, 7,
                                         d->width-1 + d->rect.x(),                 0 + d->rect.y(),
                                         d->width-1 + d->rect.x(),                 d->height-1 + d->rect.y(),
                                         d->bottomRightPoint.x()-10 + d->rect.x(), d->height-1 + d->rect.y(),
                                         d->bottomRightPoint.x()-10 + d->rect.x(), d->bottomRightPoint.y()+10 + d->rect.y(),
                                         d->topLeftPoint.x() + d->rect.x(),        d->topLeftPoint.y()+10 + d->rect.y(),
                                         0 + d->rect.x(),                          d->topLeftPoint.y() + d->rect.y(),
                                         0 + d->rect.x(),                          0 + d->rect.y());
                QRegion unusableArea(unusablePoints);

                if ( unusableArea.contains(pm) && !d->inverseTransformation )
                {
                    d->validPerspective = false;
                }
            }

            else if ( d->currentResizing == Private::ResizingBottomRight )
            {
                d->bottomRightPoint = pm - d->rect.topLeft();
                setCursor( Qt::SizeFDiagCursor );

                unusablePoints.putPoints(0, 7,
                                         0 + d->rect.x(),                         0 + d->rect.y(),
                                         d->width-1 + d->rect.x(),                0 + d->rect.y(),
                                         d->width-1 + d->rect.x(),                d->topRightPoint.y()+10 + d->rect.y(),
                                         d->topRightPoint.x() + d->rect.x(),      d->topRightPoint.y()+10 + d->rect.y(),
                                         d->bottomLeftPoint.x()+10 + d->rect.x(), d->bottomLeftPoint.y() + d->rect.y(),
                                         d->bottomLeftPoint.x()+10 + d->rect.x(), d->width-1 + d->rect.y(),
                                         0 + d->rect.x(),                         d->width-1 + d->rect.y());
                QRegion unusableArea(unusablePoints);

                if ( unusableArea.contains(pm) && !d->inverseTransformation )
                {
                    d->validPerspective = false;
                }
            }

            else
            {
                d->spot.setX(e->x()-d->rect.x());
                d->spot.setY(e->y()-d->rect.y());
            }

            updatePixmap();
/*
            // NOTE ; To hack unusable region
            QPainter p(d->pixmap);
            QPainterPath pp;
            pp.addPolygon(unusablePoints);
            p.fillPath(pp, QColor(128, 128, 128, 128));
            p.end();
*/
            update();
        }
    }
    else
    {
        if ( d->topLeftCorner.contains( e->x(), e->y() ) ||
             d->bottomRightCorner.contains( e->x(), e->y() ) )
        {
            setCursor( Qt::SizeFDiagCursor );
        }

        else if ( d->topRightCorner.contains( e->x(), e->y() ) ||
                  d->bottomLeftCorner.contains( e->x(), e->y() ) )
        {
            setCursor( Qt::SizeBDiagCursor );
        }
        else
        {
            unsetCursor();
        }
    }
}

}  // namespace Digikam
