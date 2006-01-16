/* ============================================================
 * File  : imageguidewidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-16
 * Description : a widget to display an image with a guide
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

// Qt includes.

#include <qregion.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qrect.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Local includes.

#include "dimg.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace Digikam
{

class ImageGuideWidgetPriv
{
public:

    ImageGuideWidgetPriv()
    {
        pixmap  = 0;
        iface   = 0;
        flicker = 0;
        timerID = 0;
        focus   = false;
    }

    int                  width;
    int                  height;
    
    int                  timerID;
    int                  guideMode;
    int                  guideSize;
    int                  flicker;
    int                  getColorFrom;

    bool                 sixteenBit;
    bool                 focus;
    bool                 spotVisible;
    
    // Current spot position in preview coordinates.
    QPoint               spot;
    
    QRect                rect;
    
    QColor               guideColor;
        
    QPixmap             *pixmap;
    
    ImageIface          *iface;
    
    DImg                 preview;
};

ImageGuideWidget::ImageGuideWidget(int w, int h, QWidget *parent,
                                   bool spotVisible, int guideMode, 
                                   QColor guideColor, int guideSize, bool blink,
                                   int getColorFrom)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageGuideWidgetPriv;
    d->spotVisible  = spotVisible;
    d->guideMode    = guideMode;
    d->guideColor   = guideColor;
    d->guideSize    = guideSize;
    d->getColorFrom = getColorFrom;
    
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    d->iface         = new ImageIface(w, h);
    uchar *data     = d->iface->getPreviewImage();
    d->width             = d->iface->previewWidth();
    d->height             = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview       = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    delete [] data;

    d->pixmap  = new QPixmap(w, h);
    d->rect    = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

    resetSpotPosition();
    setSpotVisible(d->spotVisible, blink);
}

ImageGuideWidget::~ImageGuideWidget()
{
    delete d->iface;
    
    if (d->timerID)
        killTimer(d->timerID);
           
    if (d->pixmap)
        delete d->pixmap;
        
    delete d;
}

Digikam::ImageIface* ImageGuideWidget::imageIface()
{
    return d->iface;
}

void ImageGuideWidget::resetSpotPosition(void)
{
    d->spot.setX( d->width / 2 );
    d->spot.setY( d->height / 2 );
    updatePreview();
}

QPoint ImageGuideWidget::getSpotPosition(void)
{
    return (QPoint::QPoint( (int)((float)d->spot.x() * (float)d->iface->originalWidth() / (float)d->width),
                            (int)((float)d->spot.y() * (float)d->iface->originalHeight() / (float)d->height)));
}

DColor ImageGuideWidget::getSpotColor(int getColorFrom)
{
    if (getColorFrom == OriginalImage)                          // Get point color from original image
        return (d->iface->getColorInfoFromOriginalImage(getSpotPosition()));
    else if (getColorFrom == PreviewImage)                      // Get point color from preview image
        return (d->iface->getColorInfoFromPreviewImage(d->spot));

    // In other cases, get point color from target preview image
    return (d->iface->getColorInfoFromTargetPreviewImage(d->spot));
}

void ImageGuideWidget::setSpotVisible(bool spotVisible, bool blink)
{
    d->spotVisible = spotVisible;
    
    if (blink)
    {
        if (d->spotVisible)
            d->timerID = startTimer(800);
        else
        {
            killTimer(d->timerID);
            d->timerID = 0;
        }
    }
       
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideColor(const QColor &color)
{
    d->guideColor = color;
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePreview();
}

void ImageGuideWidget::updatePixmap( void )
{
    d->pixmap->fill(colorGroup().background());
    d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(),
                   d->rect.width(), d->rect.height());

    if (d->spotVisible)
    {
       // Adapt spot from image coordinate to widget coordinate.
       int xspot = d->spot.x() + d->rect.x();
       int yspot = d->spot.y() + d->rect.y();
       
       switch (d->guideMode)
       {
          case HVGuideMode:
          {
             QPainter p(d->pixmap);
             p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
             p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
             p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
             p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
             p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
             p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
             p.end();
             break;
          }
            
          case PickColorMode:
          {
             QPainter p(d->pixmap);
             p.setPen(QPen(d->guideColor, 1, Qt::SolidLine));
             p.drawLine(xspot-10, yspot-10, xspot+10, yspot+10);
             p.drawLine(xspot+10, yspot-10, xspot-10, yspot+10);
             p.setPen(QPen(d->guideColor, 3, Qt::SolidLine));
             p.drawEllipse( xspot-5, yspot-5, 11, 11 );

             if (d->flicker%2 != 0)
             {
                p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                p.drawEllipse( xspot-5, yspot-5, 11, 11 );
             }
                
             p.end();
             break;
          }
       }
    }
}    

void ImageGuideWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, d->pixmap);
}

void ImageGuideWidget::updatePreview( void )
{
    updatePixmap();
    repaint(false);
}

void ImageGuideWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == d->timerID)
    {
       if (d->flicker == 5) d->flicker=0;
       else d->flicker++;
       updatePreview();
    }
    else
       QWidget::timerEvent(e);
}

void ImageGuideWidget::resizeEvent(QResizeEvent * e)
{
    blockSignals(true);
    delete d->pixmap;
    int w = e->size().width();
    int h = e->size().height();
    int old_w = d->width;
    int old_h = d->height;

    uchar *data     = d->iface->setPreviewImageSize(w, h);
    d->width             = d->iface->previewWidth();
    d->height             = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview       = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    delete [] data;

    d->pixmap  = new QPixmap(w, h);
    d->rect    = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

    d->spot.setX((int)((float)d->spot.x() * ( (float)d->width / (float)old_w)));
    d->spot.setY((int)((float)d->spot.y() * ( (float)d->height / (float)old_h)));
    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageGuideWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( !d->focus && e->button() == Qt::LeftButton &&
         d->rect.contains( e->x(), e->y() ) && d->spotVisible )
    {
       d->focus = true;
       d->spot.setX(e->x()-d->rect.x());
       d->spot.setY(e->y()-d->rect.y());;
       updatePreview();
    }
}

void ImageGuideWidget::mouseReleaseEvent ( QMouseEvent *e )
{
    if ( d->rect.contains( e->x(), e->y() ) && d->focus && d->spotVisible)
    {
       d->focus = false;
       updatePreview();
       d->spot.setX(e->x()-d->rect.x());
       d->spot.setY(e->y()-d->rect.y());
       
       DColor color = getSpotColor(d->getColorFrom);
       QPoint point = getSpotPosition();
       emit spotPositionChanged( color, true, d->spot );
       QToolTip::add( this, i18n("(%1,%2)<br>RGBA:%3,%4,%5,%6")
                                 .arg(point.x()).arg(point.y())
                                 .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()) );
    }
}

void ImageGuideWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( d->rect.contains( e->x(), e->y() ) && !d->focus && d->spotVisible )
    {
       setCursor( KCursor::crossCursor() );
    }
    else if ( d->rect.contains( e->x(), e->y() ) && d->focus && d->spotVisible )
    {
       d->spot.setX(e->x()-d->rect.x());
       d->spot.setY(e->y()-d->rect.y());
    }
    else
       setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imageguidewidget.moc"
