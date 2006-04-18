/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-22
 * Description : a widget to display a panel to choose
 *               a rectangular image area.
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
#include <qbrush.h> 
#include <qpixmap.h>
#include <qpen.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Local includes.

#include "imageiface.h"
#include "imagepaniconwidget.h"
#include "imageregionwidget.h"

namespace Digikam
{

class ImagePanIconWidgetPriv
{
public:

    ImagePanIconWidgetPriv()
    {
        pixmap        = 0;
        iface         = 0;
        data          = 0;
        moveSelection = false;
    }

    bool         moveSelection;

    uchar *      data;

    int          separateView;
    
    int          width;
    int          height;
    
    int          zoomedOrgWidth;
    int          zoomedOrgHeight;
    
    int          xpos;
    int          ypos;
    
    QRect        rect;
    QRect        regionSelection;         // Original size image selection.
    QRect        localRegionSelection;    // Thumbnail size selection.
    
    QPixmap     *pixmap;
    
    QPointArray  hightlightPoints;
    
    ImageIface  *iface;
};

ImagePanIconWidget::ImagePanIconWidget(int w, int h, QWidget *parent)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePanIconWidgetPriv;
    
    d->separateView    = ImageRegionWidget::SeparateViewVertical;
    d->iface           = new ImageIface(w, h);
    d->data            = d->iface->getPreviewImage();
    d->width           = d->iface->previewWidth();
    d->height          = d->iface->previewHeight();
    d->zoomedOrgWidth  = d->iface->originalWidth();
    d->zoomedOrgHeight = d->iface->originalHeight();
    d->pixmap          = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(d->width, d->height);
    setMouseTracking(true);

    d->rect = QRect(width()/2-d->width/2, height()/2-d->height/2, d->width, d->height);
    updatePixmap();
}

ImagePanIconWidget::~ImagePanIconWidget()
{
    delete [] d->data;
    delete d->iface;
    delete d->pixmap;
    delete d;
}

void ImagePanIconWidget::slotZoomFactorChanged(double factor)
{
    d->zoomedOrgWidth  = (int)(d->iface->originalWidth()  * factor);
    d->zoomedOrgHeight = (int)(d->iface->originalHeight() * factor);
    updatePixmap();
    repaint(false);
}

void ImagePanIconWidget::setRegionSelection(QRect regionSelection)
{
    d->regionSelection = regionSelection;
    d->localRegionSelection.setX( 1 + d->rect.x() + (int)((float)d->regionSelection.x() *
                                  ( (float)d->width / (float)d->zoomedOrgWidth )) );
                                            
    d->localRegionSelection.setY( 1 + d->rect.y() + (int)((float)d->regionSelection.y() *
                                  ( (float)d->height / (float)d->zoomedOrgHeight )) );
                                            
    d->localRegionSelection.setWidth( (int)((float)d->regionSelection.width() *
                                      ( (float)d->width / (float)d->zoomedOrgWidth )) );
                                     
    d->localRegionSelection.setHeight( (int)((float)d->regionSelection.height() *
                                       ( (float)d->height / (float)d->zoomedOrgHeight )) );

    updatePixmap();
    repaint(false);
}

QRect ImagePanIconWidget::getRegionSelection(void)
{
    return (d->regionSelection);
}

void ImagePanIconWidget::setCenterSelection(void)
{
    setRegionSelection(QRect( 
             (int)(((float)d->zoomedOrgWidth / 2.0)  - ((float)d->regionSelection.width() / 2.0)),
             (int)(((float)d->zoomedOrgHeight / 2.0) - ((float)d->regionSelection.height() / 2.0)),
             d->regionSelection.width(),
             d->regionSelection.height()));
}

void ImagePanIconWidget::setHighLightPoints(QPointArray pointsList)
{
    d->hightlightPoints = pointsList;
    updatePixmap();
    repaint(false);
}
       
void ImagePanIconWidget::regionSelectionMoved( bool targetDone )
{
    if (targetDone)
    {
       updatePixmap();          
       repaint(false);
    }
    
    int x = ROUND( ((float)d->localRegionSelection.x() - (float)d->rect.x() ) *
                   ( (float)d->zoomedOrgWidth / (float)d->width ));
                                            
    int y = ROUND( ((float)d->localRegionSelection.y() - (float)d->rect.y() ) *
                   ( (float)d->zoomedOrgHeight / (float)d->height ));
                                            
    int w = ROUND((float)d->localRegionSelection.width() *
                 ( (float)d->zoomedOrgWidth / (float)d->width ));
                                     
    int h = ROUND((float)d->localRegionSelection.height() *
                 ( (float)d->zoomedOrgHeight / (float)d->height ));
                     
    d->regionSelection.setX(x);
    d->regionSelection.setY(y);
    d->regionSelection.setWidth(w);
    d->regionSelection.setHeight(h);
       
    emit signalSelectionMoved( d->regionSelection, targetDone );
}

void ImagePanIconWidget::updatePixmap( void )
{
    // Drawing background and image.
    d->pixmap->fill(colorGroup().background());
    d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(), d->rect.width(), d->rect.height());
    
    QPainter p(d->pixmap);
   
    // Drawing HighLighted points.
    
    if (!d->hightlightPoints.isEmpty())
    {
       QPoint pt;
       
       for (uint i = 0 ; i < d->hightlightPoints.count() ; i++)
       {
          pt = d->hightlightPoints.point(i);
          pt.setX((int)(pt.x() * (float)(d->width)/(float)d->iface->originalWidth())); 
          pt.setY((int)(pt.y() * (float)(d->height)/(float)d->iface->originalHeight()));
          p.setPen(QPen(Qt::black, 1, Qt::SolidLine));
          p.drawLine(pt.x(), pt.y()-1, pt.x(), pt.y()+1);
          p.drawLine(pt.x()-1, pt.y(), pt.x()+1, pt.y());
          p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
          p.drawPoint(pt.x()-1, pt.y()-1);
          p.drawPoint(pt.x()+1, pt.y()+1);
          p.drawPoint(pt.x()-1, pt.y()+1);
          p.drawPoint(pt.x()+1, pt.y()-1);
       }
    }   
    
    // Drawing selection border
    p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    p.drawRect(d->localRegionSelection.x(), 
               d->localRegionSelection.y(),
               d->localRegionSelection.width(), 
               d->localRegionSelection.height());
    p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    p.drawRect(d->localRegionSelection.x() + 1, 
               d->localRegionSelection.y() + 1,
               d->localRegionSelection.width() - 2, 
               d->localRegionSelection.height() - 2);
    
    if (d->separateView == ImageRegionWidget::SeparateViewVertical)
    {
        p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        p.drawLine(d->localRegionSelection.topLeft().x() + d->localRegionSelection.width()/2,
                   d->localRegionSelection.topLeft().y(),
                   d->localRegionSelection.bottomLeft().x() + d->localRegionSelection.width()/2,
                   d->localRegionSelection.bottomLeft().y());
        p.setPen(QPen(Qt::red, 1, Qt::DotLine));
        p.drawLine(d->localRegionSelection.topLeft().x() + d->localRegionSelection.width()/2,
                   d->localRegionSelection.topLeft().y() + 1,
                   d->localRegionSelection.bottomLeft().x() + d->localRegionSelection.width()/2,
                   d->localRegionSelection.bottomLeft().y() - 1);
    }
    else if (d->separateView == ImageRegionWidget::SeparateViewHorizontal)
    {
        p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        p.drawLine(d->localRegionSelection.topLeft().x(),
                   d->localRegionSelection.topLeft().y() + d->localRegionSelection.height()/2,
                   d->localRegionSelection.topRight().x(),
                   d->localRegionSelection.topRight().y() + d->localRegionSelection.height()/2);
        p.setPen(QPen(Qt::red, 1, Qt::DotLine));
        p.drawLine(d->localRegionSelection.topLeft().x() + 1,
                   d->localRegionSelection.topLeft().y() + d->localRegionSelection.height()/2,
                   d->localRegionSelection.topRight().x() - 1,
                   d->localRegionSelection.topRight().y() + d->localRegionSelection.height()/2);
    }

    p.end();
}

void ImagePanIconWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, d->pixmap);
}

void ImagePanIconWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         d->localRegionSelection.contains( e->x(), e->y() ) )
    {
       d->xpos = e->x();
       d->ypos = e->y();
       d->moveSelection = true;
       setCursor( KCursor::sizeAllCursor() );           
       emit signalSelectionTakeFocus();
    }
}

void ImagePanIconWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->moveSelection )
    {    
       setCursor( KCursor::arrowCursor() );           
       regionSelectionMoved(true);
       d->moveSelection = false;
    }
}

void ImagePanIconWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( d->moveSelection && e->state() == Qt::LeftButton )
    {
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
    else 
    {
       if ( d->localRegionSelection.contains( e->x(), e->y() ) )
           setCursor( KCursor::handCursor() );           
       else
           setCursor( KCursor::arrowCursor() );           
    }
}

void ImagePanIconWidget::slotSeparateViewToggled(int t)
{
    d->separateView = t;
    updatePixmap();
    repaint(false);
}

}  // NameSpace Digikam

#include "imagepaniconwidget.moc"
