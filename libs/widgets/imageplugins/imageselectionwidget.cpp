/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-09
 * Description : image selection widget used by ratio crop tool.
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#define OPACITY  0.7
#define RCOL     0xAA
#define GCOL     0xAA
#define BCOL     0xAA

#define MINRANGE 50

// Fibanocci irrationel Golden Number.
#define PHI      1.618033988
// 1/PHI
#define INVPHI   0.61803398903633

// C++ includes.

#include <cstdio>
#include <cmath>

// Qt includes.

#include <qregion.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qbrush.h> 
#include <qpixmap.h>
#include <qimage.h>
#include <qpen.h>
#include <qpoint.h> 
#include <qtimer.h> 
#include <qsizepolicy.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Local includes.

#include "imageiface.h"
#include "dimg.h"
#include "imageselectionwidget.h"

namespace Digikam
{

class ImageSelectionWidgetPriv
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

    ImageSelectionWidgetPriv()
    {
        currentResizing = ResizingNone;
        timerH          = 0;
        timerW          = 0;
        iface           = 0;
        pixmap          = 0;
    }

    // Golden guide types.
    bool        drawGoldenSection;
    bool        drawGoldenSpiralSection;
    bool        drawGoldenSpiral;
    bool        drawGoldenTriangle;

    // Golden guide translations.
    bool        flipHorGoldenGuide;
    bool        flipVerGoldenGuide;

    int         guideLinesType;
    int         guideSize;

    int         xpos;
    int         ypos;

    int         currentAspectRatioType;
    int         currentResizing;
    int         currentOrientation;

    float       currentAspectRatioValue;

    QRect       rect;
    QRect       regionSelection;         // Real size image selection.
    QRect       localRegionSelection;    // Local size selection.

    // Draggable local region selection corners.
    QRect       localTopLeftCorner;
    QRect       localBottomLeftCorner;
    QRect       localTopRightCorner;
    QRect       localBottomRightCorner;

    QPixmap    *pixmap;

    QTimer     *timerW;
    QTimer     *timerH;

    QColor      guideColor;

    QImage      preview;
    
    ImageIface *iface;
};

ImageSelectionWidget::ImageSelectionWidget(int w, int h, QWidget *parent, 
                                           float aspectRatioValue, int aspectRatioType, 
                                           int orient, int guideLinesType)
                    : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageSelectionWidgetPriv;
    d->currentAspectRatioType  = aspectRatioType;
    d->currentAspectRatioValue = aspectRatioValue;
    d->currentOrientation      = orient;
    d->guideLinesType          = guideLinesType;
    d->timerW                  = 0L;
    d->timerH                  = 0L;
    d->iface                   = 0L;
    d->pixmap                  = 0L;
    d->guideSize               = 1;

    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    d->iface        = new ImageIface(w, h);
    uchar *data     = d->iface->getPreviewImage();
    int width       = d->iface->previewWidth();
    int height      = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(width, height, sixteenBit, hasAlpha, data).copyQImage();
    delete [] data;
    
    d->pixmap  = new QPixmap(w, h);

    d->rect = QRect(w/2-d->preview.width()/2, h/2-d->preview.height()/2, d->preview.width(), d->preview.height());
    realToLocalRegion();
    updatePixmap();
    setGoldenGuideTypes(true, false, false, false, false, false);
}

ImageSelectionWidget::~ImageSelectionWidget()
{
    if (d->timerW)
       delete d->timerW;

    if (d->timerH)
       delete d->timerH;

    delete d->iface;
    delete d->pixmap;
    delete d;
}

ImageIface* ImageSelectionWidget::imageIface()
{
    return d->iface;
}

void ImageSelectionWidget::resizeEvent(QResizeEvent *e)
{
    delete d->pixmap;

    int w           = e->size().width();
    int h           = e->size().height();
    
    uchar *data     = d->iface->setPreviewImageSize(w, h);
    int width       = d->iface->previewWidth();
    int height      = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(width, height, sixteenBit, hasAlpha, data).copyQImage();
    delete [] data;
    
    d->pixmap = new QPixmap(w, h);

    d->rect = QRect(w/2-d->preview.width()/2, h/2-d->preview.height()/2, d->preview.width(), d->preview.height());
    realToLocalRegion();
    updatePixmap();
}

int ImageSelectionWidget::getOriginalImageWidth(void)
{
    return d->iface->originalWidth();
}

int ImageSelectionWidget::getOriginalImageHeight(void)
{
    return d->iface->originalHeight();
}

QRect ImageSelectionWidget::getRegionSelection(void)
{
    return d->regionSelection;
}

int ImageSelectionWidget::getMinWidthRange(void)
{
    return( (int)( ((float)MINRANGE - (float)d->rect.x() ) *
                   ( (float)d->iface->originalWidth() / (float)d->preview.width() )) );
}

int ImageSelectionWidget::getMinHeightRange(void)
{
    return( (int)( ((float)MINRANGE - (float)d->rect.y() ) *
                   ( (float)d->iface->originalHeight() / (float)d->preview.height() )) );
}

void ImageSelectionWidget::resetSelection(void)
{
    d->regionSelection.moveTopLeft(QPoint(0, 0));
    d->regionSelection.setWidth((int)(d->iface->originalWidth()/2.0));
    d->regionSelection.setHeight((int)(d->iface->originalHeight()/2.0));
    realToLocalRegion();
    applyAspectRatio(false, false);

    d->localRegionSelection.moveBy(
      d->rect.width()/2 - d->localRegionSelection.width()/2,
      d->rect.height()/2 - d->localRegionSelection.height()/2);

    applyAspectRatio(false, true, false);
    regionSelectionChanged(true);
}

void ImageSelectionWidget::setCenterSelection(int centerType)
{
    switch (centerType)
    {
       case CenterWidth:
          d->regionSelection.moveLeft(0);
          break;

       case CenterHeight:
          d->regionSelection.moveTop(0);
          break;

       case CenterImage:
          d->regionSelection.moveTopLeft(QPoint(0, 0));
          break;
    }

    realToLocalRegion();
    applyAspectRatio(false, false);

    switch (centerType)
    {
       case CenterWidth:
          d->localRegionSelection.moveBy(
            d->rect.width()/2 - d->localRegionSelection.width()/2,
            0);
          break;

       case CenterHeight:
          d->localRegionSelection.moveBy(
            0, 
            d->rect.height()/2 - d->localRegionSelection.height()/2);
          break;

       case CenterImage:
          d->localRegionSelection.moveBy(
            d->rect.width()/2 - d->localRegionSelection.width()/2,
            d->rect.height()/2 - d->localRegionSelection.height()/2);
          break;
    }

    applyAspectRatio(false, true, false);
    regionSelectionChanged(true);
}

void ImageSelectionWidget::maxAspectSelection(void)
{
    d->localRegionSelection.setTopLeft( d->rect.topLeft() );

    if ( !d->currentOrientation )   // Landscape
    {
       d->localRegionSelection.setWidth(d->rect.width());
       applyAspectRatio(false, false);

       if ( d->localRegionSelection.height() > d->rect.height() )
       {
          d->localRegionSelection.setHeight(d->rect.height());
          applyAspectRatio(true, false);
       }
    }
    else                          // Portrait
    {
       d->localRegionSelection.setHeight(d->rect.height());
       applyAspectRatio(true, false);

       if ( d->localRegionSelection.width() > d->rect.width() )
       {
          d->localRegionSelection.setWidth(d->rect.width());
          applyAspectRatio(false, false);
       }
    }

    setCenterSelection(CenterImage);
}

void ImageSelectionWidget::setGoldenGuideTypes(bool drawGoldenSection,  bool drawGoldenSpiralSection,
                                               bool drawGoldenSpiral,   bool drawGoldenTriangle,
                                               bool flipHorGoldenGuide, bool flipVerGoldenGuide)
{
    d->drawGoldenSection       = drawGoldenSection;
    d->drawGoldenSpiralSection = drawGoldenSpiralSection;
    d->drawGoldenSpiral        = drawGoldenSpiral;
    d->drawGoldenTriangle      = drawGoldenTriangle;
    d->flipHorGoldenGuide      = flipHorGoldenGuide;
    d->flipVerGoldenGuide      = flipVerGoldenGuide;
}

void ImageSelectionWidget::slotGuideLines(int guideLinesType)
{
    d->guideLinesType = guideLinesType;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::slotChangeGuideColor(const QColor &color)
{
    d->guideColor = color;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionOrientation(int orient)
{
    d->currentOrientation = orient;
    applyAspectRatio(true);
}

void ImageSelectionWidget::setSelectionAspectRatioType(int aspectRatioType)
{
    d->currentAspectRatioType = aspectRatioType;

    switch(aspectRatioType)
    {
       case RATIO01X01:  
          d->currentAspectRatioValue = 1.0;
          break;

       case RATIO03X04:  
          d->currentAspectRatioValue = 0.75;
          break;

       case RATIO02x03:
          d->currentAspectRatioValue = 0.66666666666667;
          break;

       case RATIO05x07:
          d->currentAspectRatioValue = 0.71428571428571;
          break;

       case RATIO07x10:
          d->currentAspectRatioValue = 0.7;
          break;

       case RATIO04X05:
          d->currentAspectRatioValue = 0.8;
          break;

       case RATIOGOLDEN:
          d->currentAspectRatioValue = INVPHI;
          break;
    }

    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionAspectRatioValue(float aspectRatioValue)
{
    d->currentAspectRatioValue = aspectRatioValue;
    d->currentAspectRatioType  = RATIOCUSTOM;
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionX(int x)
{
    d->regionSelection.moveLeft(x);
    realToLocalRegion();
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionY(int y)
{
    d->regionSelection.moveTop(y);
    realToLocalRegion();
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionWidth(int w)
{
    d->regionSelection.setWidth(w);
    realToLocalRegion(true);    
    applyAspectRatio(false, true, false);

    if (d->currentAspectRatioType == RATIONONE)
    {
       emit signalSelectionChanged( d->regionSelection );
       return;
    }

    localToRealRegion();
    emit signalSelectionHeightChanged(d->regionSelection.height());

    if (d->timerW)
    {
       d->timerW->stop();
       delete d->timerW;
    }

    d->timerW = new QTimer( this );
    connect( d->timerW, SIGNAL(timeout()),
             this, SLOT(slotTimerDone()) );
    d->timerW->start(500, true);
}

void ImageSelectionWidget::setSelectionHeight(int h)
{
    d->regionSelection.setHeight(h);
    realToLocalRegion(true);
    applyAspectRatio(true, true, false);

    if (d->currentAspectRatioType == RATIONONE)
    {
       emit signalSelectionChanged( d->regionSelection );
       return;
    }

    localToRealRegion();
    emit signalSelectionWidthChanged(d->regionSelection.width());

    if (d->timerH)
    {
       d->timerH->stop();
       delete d->timerH;
    }

    d->timerH = new QTimer( this );
    connect( d->timerH, SIGNAL(timeout()),
             this, SLOT(slotTimerDone()) );
    d->timerH->start(500, true);
}

void ImageSelectionWidget::slotTimerDone(void)
{
    regionSelectionChanged(true);
}

void ImageSelectionWidget::realToLocalRegion(bool updateSizeOnly)
{
    if (!updateSizeOnly)
    {
       if (d->regionSelection.x() == 0 )
          d->localRegionSelection.setX(d->rect.x());
       else
          d->localRegionSelection.setX( 1 + d->rect.x() + (int)((float)d->regionSelection.x() *
                                      ( (float)d->preview.width() / (float)d->iface->originalWidth() )) );

       if (d->regionSelection.y() == 0 )
          d->localRegionSelection.setY(d->rect.y());
       else
          d->localRegionSelection.setY( 1 + d->rect.y() + (int)((float)d->regionSelection.y() *
                                      ( (float)d->preview.height() / (float)d->iface->originalHeight() )) );
    }

    d->localRegionSelection.setWidth( (int)((float)d->regionSelection.width() *
                                          ( (float)d->preview.width() / (float)d->iface->originalWidth() )) );

    d->localRegionSelection.setHeight( (int)((float)d->regionSelection.height() *
                                           ( (float)d->preview.height() / (float)d->iface->originalHeight() )) );
}

void ImageSelectionWidget::localToRealRegion(void)
{
    int x = (int)( ((float)d->localRegionSelection.x() - (float)d->rect.x() ) *
                   ( (float)d->iface->originalWidth() / (float)d->preview.width() ));

    int y = (int)( ((float)d->localRegionSelection.y() - (float)d->rect.y() ) *
                   ( (float)d->iface->originalHeight() / (float)d->preview.height() ));

    int w = (int)((float)d->localRegionSelection.width() *
                 ( (float)d->iface->originalWidth() / (float)d->preview.width() ));

    int h = (int)((float)d->localRegionSelection.height() *
                 ( (float)d->iface->originalHeight() / (float)d->preview.height() ));

    d->regionSelection.setRect(x, y, w, h);
}

void ImageSelectionWidget::applyAspectRatio(bool WOrH, bool repaintWidget, bool updateChange)
{
    // Save local selection area for re-adjustment after changing width and height.
    QRect oldLocalRegionSelection = d->localRegionSelection;

    if ( !WOrH )  // Width changed.
    {    
       int w = d->localRegionSelection.width();

       switch(d->currentAspectRatioType)
       {
          case RATIONONE:  
             break;

          default:
             if ( d->currentOrientation )
                d->localRegionSelection.setHeight((int)(w / d->currentAspectRatioValue));  // Landscape
             else                       
                d->localRegionSelection.setHeight((int)(w * d->currentAspectRatioValue));  // Portrait
             break;
       }
    }  
    else      // Height changed.
    {
       int h = d->localRegionSelection.height();

       switch(d->currentAspectRatioType)
       {
          case RATIONONE:  
             break;

          default:
             if ( d->currentOrientation )
                d->localRegionSelection.setWidth((int)(h * d->currentAspectRatioValue));   // Portrait
             else
                d->localRegionSelection.setWidth((int)(h / d->currentAspectRatioValue));   // Landscape
             break;
       }
    }

    // If we change local selection size by a corner, re-adjust the oposite corner position.
    // It unecessary to do that for Bottom Left corner because it's do by setWidth and setHeight
    // methods.

    switch(d->currentResizing)
    {
       case ImageSelectionWidgetPriv::ResizingTopLeft:    
          d->localRegionSelection.moveBottomRight( oldLocalRegionSelection.bottomRight() );
          break;

       case ImageSelectionWidgetPriv::ResizingTopRight:      
          d->localRegionSelection.moveBottomLeft( oldLocalRegionSelection.bottomLeft() );
          break;

       case ImageSelectionWidgetPriv::ResizingBottomLeft:      
          d->localRegionSelection.moveTopRight( oldLocalRegionSelection.topRight() );
          break;
    }       
       
    // Recalculate the real selection values.
    
    if (updateChange) 
       regionSelectionChanged(false);
    
    if (repaintWidget)
    {
       updatePixmap();
       repaint(false);
    }
}

void ImageSelectionWidget::regionSelectionMoved( bool targetDone )
{
    if (targetDone)
    {
       if (d->localRegionSelection.left() < d->rect.left())
           d->localRegionSelection.moveLeft(d->rect.left());
       if (d->localRegionSelection.top() < d->rect.top())
           d->localRegionSelection.moveTop(d->rect.top());
       if (d->localRegionSelection.right() > d->rect.right())
          d->localRegionSelection.moveRight(d->rect.right());
       if (d->localRegionSelection.bottom() > d->rect.bottom())
          d->localRegionSelection.moveBottom(d->rect.bottom());
       
       updatePixmap();
       repaint(false);
    }

    localToRealRegion();
       
    if (targetDone)    
       emit signalSelectionMoved( d->regionSelection );
}

void ImageSelectionWidget::regionSelectionChanged(bool targetDone)
{
    if (targetDone)
    {
       if (d->localRegionSelection.left() < d->rect.left())
       {
          d->localRegionSelection.setLeft(d->rect.left());
          applyAspectRatio(false);
       }
       if (d->localRegionSelection.top() < d->rect.top())
       {
          d->localRegionSelection.setTop(d->rect.top());
          applyAspectRatio(true);
       }
       if (d->localRegionSelection.right() > d->rect.right())
       {
          d->localRegionSelection.setRight(d->rect.right());
          applyAspectRatio(false);
       }
       if (d->localRegionSelection.bottom() > d->rect.bottom())
       {
          d->localRegionSelection.setBottom(d->rect.bottom());
          applyAspectRatio(true);
       }
    }

    localToRealRegion();

    if (targetDone)
       emit signalSelectionChanged( d->regionSelection );
}

void ImageSelectionWidget::updatePixmap(void)
{
    // Updated draging corners region.

    d->localTopLeftCorner.setRect(d->localRegionSelection.left(),
                                 d->localRegionSelection.top(), 8, 8);
    d->localBottomLeftCorner.setRect(d->localRegionSelection.left(),
                                    d->localRegionSelection.bottom() - 7, 8, 8);
    d->localTopRightCorner.setRect(d->localRegionSelection.right() - 7,
                                  d->localRegionSelection.top(), 8, 8);
    d->localBottomRightCorner.setRect(d->localRegionSelection.right() - 7,
                                     d->localRegionSelection.bottom() - 7, 8, 8);

    // Drawing background and image.

    d->pixmap->fill(colorGroup().background());

    if (d->preview.isNull())
        return;

    // Drawing region outside selection grayed.

    int lx = d->localRegionSelection.left()   - d->rect.left();
    int rx = d->localRegionSelection.right()  - d->rect.left();
    int ty = d->localRegionSelection.top()    - d->rect.top();
    int by = d->localRegionSelection.bottom() - d->rect.top();

    QImage image = d->preview.copy();

    uchar* ptr = image.bits();
    uchar  r, g, b;

    for (uint j=0 ; j < (uint)d->preview.height() ; j++)
    {
        for (uint i=0 ; i < (uint)d->preview.width() ; i++)
        {
            if (i < (uint)lx || i >= (uint)rx || j < (uint)ty || j >= (uint)by)
            {
                b = ptr[0];
                g = ptr[1];
                r = ptr[2];

                r += (uchar)((RCOL - r) * OPACITY);
                g += (uchar)((GCOL - g) * OPACITY);
                b += (uchar)((BCOL - b) * OPACITY);

                ptr[0] = b;
                ptr[1] = g;
                ptr[2] = r;
            }

            ptr+=4;
        }
    }

    QPixmap pix(image);
    bitBlt(d->pixmap, d->rect.x(), d->rect.y(), &pix);
    QPainter p(d->pixmap);

    // Drawing selection borders.

    p.setPen(QPen(QColor(250, 250, 255), 1, Qt::SolidLine));
    p.drawRect(d->localRegionSelection);

    // Drawing selection corners.

    p.drawRect(d->localTopLeftCorner);
    p.drawRect(d->localBottomLeftCorner);
    p.drawRect(d->localTopRightCorner);
    p.drawRect(d->localBottomRightCorner);

    // Drawing guide lines.

    // Constraint drawing only on local selection region.
    // This is needed because arcs and incurved lines can draw
    // outside a little of local selection region.
    p.setClipping(true);
    p.setClipRect(d->localRegionSelection);

    switch (d->guideLinesType)
    {
       case RulesOfThirds:
       {
            int xThird = d->localRegionSelection.width()  / 3;
            int yThird = d->localRegionSelection.height() / 3;

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
            p.drawLine( d->localRegionSelection.left() + xThird,   d->localRegionSelection.top(),
                        d->localRegionSelection.left() + xThird,   d->localRegionSelection.bottom() );
            p.drawLine( d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.top(),
                        d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.bottom() );

            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + yThird );
            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + 2*yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + 2*yThird );

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
            p.drawLine( d->localRegionSelection.left() + xThird,   d->localRegionSelection.top(),
                        d->localRegionSelection.left() + xThird,   d->localRegionSelection.bottom() );
            p.drawLine( d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.top(),
                        d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.bottom() );

            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + yThird );
            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + 2*yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + 2*yThird );
            break;
       }

       case HarmoniousTriangles:
       {
            // Move coordinates to local center selection.
            p.translate(d->localRegionSelection.center().x(), d->localRegionSelection.center().y());

            // Flip horizontal.
            if (d->flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip verical.
            if (d->flipVerGoldenGuide)
                p.scale(1, -1);

            float w = (float)d->localRegionSelection.width();
            float h = (float)d->localRegionSelection.height();
            int dst = (int)((h*cos(atan(w/h)) / (cos(atan(h/w)))));

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
            p.drawLine( -d->localRegionSelection.width()/2, -d->localRegionSelection.height()/2,
                         d->localRegionSelection.width()/2,  d->localRegionSelection.height()/2);

            p.drawLine( -d->localRegionSelection.width()/2 + dst, -d->localRegionSelection.height()/2,
                        -d->localRegionSelection.width()/2,        d->localRegionSelection.height()/2);

            p.drawLine( d->localRegionSelection.width()/2,       -d->localRegionSelection.height()/2,
                        d->localRegionSelection.width()/2 - dst,  d->localRegionSelection.height()/2);

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
            p.drawLine( -d->localRegionSelection.width()/2, -d->localRegionSelection.height()/2,
                         d->localRegionSelection.width()/2,  d->localRegionSelection.height()/2);

            p.drawLine( -d->localRegionSelection.width()/2 + dst, -d->localRegionSelection.height()/2,
                        -d->localRegionSelection.width()/2,        d->localRegionSelection.height()/2);

            p.drawLine( d->localRegionSelection.width()/2,       -d->localRegionSelection.height()/2,
                        d->localRegionSelection.width()/2 - dst,  d->localRegionSelection.height()/2);
            break;
       }

       case GoldenMean:
       {
            // Move coordinates to local center selection.
            p.translate(d->localRegionSelection.center().x(), d->localRegionSelection.center().y());

            // Flip horizontal.
            if (d->flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip verical.
            if (d->flipVerGoldenGuide)
                p.scale(1, -1);

            int w = d->localRegionSelection.width();
            int h = d->localRegionSelection.height();

            QRect R1(-w/2, -h/2, 
                     (int)(w/PHI), h);
            QRect R2((int)(w*(INVPHI - 0.5)), (int)(h*(0.5 - INVPHI)), 
                     (int)(w*(1 - INVPHI)), (int)(h/PHI)); 
            QRect R3((int)(w/2 - R2.width()/PHI), -h/2, 
                     (int)(R2.width()/PHI), h - R2.height());
            QRect R4(R2.x(), R1.y(), R3.x() - R2.x(), 
                     (int)(R3.height()/PHI));
            QRect R5(R4.x(), R4.bottom(), (int)(R4.width()/PHI), 
                     R3.height() - R4.height());
            QRect R6(R5.x() + R5.width(), R5.bottom() - (int)(R5.height()/PHI), 
                     R3.x() - R5.right(), (int)(R5.height()/PHI));
            QRect R7(R6.right() - (int)(R6.width()/PHI), R4.bottom(), 
                     (int)(R6.width()/PHI), R5.height() - R6.height());

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));

            // Drawing Golden sections.
            if (d->drawGoldenSection)
            {
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               p.drawLine( R2.right() - R1.width(), R1.top(),
                           R2.right() - R1.width(), R1.bottom() );

               p.drawLine( R1.topRight(), R1.bottomRight() );
            }

            // Drawing Golden triangle guides.
            if (d->drawGoldenTriangle)
            {
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );

               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }

            // Drawing Golden spiral sections.
            if (d->drawGoldenSpiralSection)
            {
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }

            // Drawing Golden Spiral.
            if (d->drawGoldenSpiral)
            {
               p.drawArc ( R1.left(), 
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(), 
                           180*16, 90*16);

               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);

               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);

               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);

               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);

               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);

               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);
            }

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));

            // Drawing Golden sections.
            if (d->drawGoldenSection)
            {
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               p.drawLine( R2.right() - R1.width(), R1.top(),
                           R2.right() - R1.width(), R1.bottom() );

               p.drawLine( R1.topRight(), R1.bottomRight() );
            }

            // Drawing Golden triangle guides.
            if (d->drawGoldenTriangle)
            {            
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );
                
               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }
                
            // Drawing Golden spiral sections.
            if (d->drawGoldenSpiralSection)
            {            
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }
                                        
            // Drawing Golden Spiral.
            if (d->drawGoldenSpiral)
            {
               p.drawArc ( R1.left(), 
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(), 
                           180*16, 90*16);                       
               
               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);                       
                
               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);                       
                
               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);                       
                
               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);                       
                
               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);                       
                
               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);                       
            }
                
            break;
       }
    }    
    
    p.setClipping(false);
              
    p.end();
}

void ImageSelectionWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, d->pixmap);
}

void ImageSelectionWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
       if ( d->localTopLeftCorner.contains( e->x(), e->y() ) )
          d->currentResizing = ImageSelectionWidgetPriv::ResizingTopLeft;
       else if ( d->localBottomRightCorner.contains( e->x(), e->y() ) )
          d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomRight;
       else if ( d->localTopRightCorner.contains( e->x(), e->y() ) )
          d->currentResizing = ImageSelectionWidgetPriv::ResizingTopRight;
       else if ( d->localBottomLeftCorner.contains( e->x(), e->y() ) )
          d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomLeft;
       else if ( d->localRegionSelection.contains( e->x(), e->y() ) )
       {
          d->xpos = e->x();
          d->ypos = e->y();
          setCursor( KCursor::sizeAllCursor() );
       }
    }
}

void ImageSelectionWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->currentResizing != ImageSelectionWidgetPriv::ResizingNone )
    {
       setCursor( KCursor::arrowCursor() );
       regionSelectionChanged(true);
       d->currentResizing = ImageSelectionWidgetPriv::ResizingNone;
    } 
    else if ( d->localRegionSelection.contains( d->xpos, d->ypos ) )
    {    
       setCursor( KCursor::arrowCursor() );
       regionSelectionMoved(true);
    }      
}

void ImageSelectionWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
    {
       if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingNone )
       {
          setCursor( KCursor::sizeAllCursor() );
          int newxpos = e->x();
          int newypos = e->y();
               
          d->localRegionSelection.moveBy (newxpos - d->xpos, newypos - d->ypos);

          d->xpos = newxpos;
          d->ypos = newypos;
          
          // Perform normalization of selection area.
          
          if (d->localRegionSelection.left() < d->rect.left())
             d->localRegionSelection.moveLeft(d->rect.left());
             
          if (d->localRegionSelection.top() < d->rect.top())
             d->localRegionSelection.moveTop(d->rect.top());
             
          if (d->localRegionSelection.right() > d->rect.right())
             d->localRegionSelection.moveRight(d->rect.right());
             
          if (d->localRegionSelection.bottom() > d->rect.bottom())
             d->localRegionSelection.moveBottom(d->rect.bottom());
          
          updatePixmap();
          repaint(false);
          regionSelectionMoved(false);
          return;
       }    
       else if ( d->rect.contains(e->x(), e->y()) )
       {
          QPoint pm(e->x(), e->y());
          
          if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingTopLeft &&
               pm.x() < d->localRegionSelection.right() - MINRANGE &&
               pm.y() < d->localRegionSelection.bottom() - MINRANGE )
              d->localRegionSelection.setTopLeft(pm);
             
          else if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingTopRight  &&
               pm.x() > d->localRegionSelection.left() + MINRANGE &&
               pm.y() < d->localRegionSelection.bottom() - MINRANGE )
             d->localRegionSelection.setTopRight(pm);
          
          else if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingBottomLeft  &&
               pm.x() < d->localRegionSelection.right() - MINRANGE &&
               pm.y() > d->localRegionSelection.top() + MINRANGE )
             d->localRegionSelection.setBottomLeft(pm);
             
          else if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingBottomRight  &&
               pm.x() > d->localRegionSelection.left() + MINRANGE &&
               pm.y() > d->localRegionSelection.top() + MINRANGE )
             d->localRegionSelection.setBottomRight(pm);
          else 
             return;
             
          applyAspectRatio(false, false);
          applyAspectRatio(true);
          
          return;
       }
    }
    else
    {
       if ( d->localTopLeftCorner.contains( e->x(), e->y() ) ||
            d->localBottomRightCorner.contains( e->x(), e->y() ) )
           setCursor( KCursor::sizeFDiagCursor() );
       else if ( d->localTopRightCorner.contains( e->x(), e->y() ) ||
                 d->localBottomLeftCorner.contains( e->x(), e->y() ) )
           setCursor( KCursor::sizeBDiagCursor() );
       else if ( d->localRegionSelection.contains( e->x(), e->y() ) )
           setCursor( KCursor::handCursor() );
       else
           setCursor( KCursor::arrowCursor() );
    }
}

}  // NameSpace Digikam

#include "imageselectionwidget.moc"

