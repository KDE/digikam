/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * Copyright 2004-2007 by Gilles Caulier
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

// C++ include.

#include <cmath>

// Qt includes.

#include <qregion.h>
#include <qpainter.h>
#include <qbrush.h> 
#include <qpixmap.h>
#include <qpen.h>
#include <qtimer.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h> 

// Local includes.

#include "ddebug.h"
#include "paniconwidget.h"
#include "paniconwidget.moc"

namespace Digikam
{

class PanIconWidgetPriv
{

public:

    PanIconWidgetPriv()
    {
        flicker       = false;
        timerID       = 0;
        pixmap        = 0;
        moveSelection = false;
    }

    bool         moveSelection;
    bool         flicker;

    int          timerID;
    int          width;
    int          height;
    int          orgWidth;    
    int          orgHeight;    
    int          zoomedOrgWidth;
    int          zoomedOrgHeight;    
    int          xpos;
    int          ypos;
    
    QRect        rect;
    QRect        regionSelection;         // Original size image selection.
    QRect        localRegionSelection;    // Thumbnail size selection.
    
    QPixmap     *pixmap;

    QImage       image;
};

PanIconWidget::PanIconWidget(QWidget *parent, WFlags flags)
             : QWidget(parent, 0, flags)
{
    d = new PanIconWidgetPriv;

    setBackgroundMode(Qt::NoBackground);
    setMouseTracking(true);
}

PanIconWidget::~PanIconWidget()
{
    if (d->timerID)
        killTimer(d->timerID);

    delete d->pixmap;
    delete d;
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const QImage& image)
{
    QSize sz(image.width(), image.height());
    sz.scale(previewWidth, previewHeight, QSize::ScaleMin);
    d->pixmap          = new QPixmap(previewWidth, previewHeight);
    d->width           = sz.width();
    d->height          = sz.height();
    d->image           = image.smoothScale(sz.width(), sz.height());
    d->orgWidth        = image.width();
    d->orgHeight       = image.height();
    d->zoomedOrgWidth  = image.width();
    d->zoomedOrgHeight = image.height();
    setFixedSize(d->width, d->height);
 
    d->rect = QRect(width()/2-d->width/2, height()/2-d->height/2, d->width, d->height);
    updatePixmap();
    d->timerID = startTimer(800);
}

void PanIconWidget::slotZoomFactorChanged(double factor)
{
    d->zoomedOrgWidth  = (int)(d->orgWidth  * factor);
    d->zoomedOrgHeight = (int)(d->orgHeight * factor);
    updatePixmap();
    repaint(false);
}

void PanIconWidget::setRegionSelection(QRect regionSelection)
{
    d->regionSelection = regionSelection;
    d->localRegionSelection.setX( 1 + d->rect.x() + (int)((float)d->regionSelection.x() *
                                  ((float)d->width / (float)d->zoomedOrgWidth)) );
                                            
    d->localRegionSelection.setY( 1 + d->rect.y() + (int)((float)d->regionSelection.y() *
                                  ((float)d->height / (float)d->zoomedOrgHeight)) );
                                            
    d->localRegionSelection.setWidth( (int)((float)d->regionSelection.width() *
                                      ((float)d->width / (float)d->zoomedOrgWidth)) );
                                     
    d->localRegionSelection.setHeight( (int)((float)d->regionSelection.height() *
                                       ((float)d->height / (float)d->zoomedOrgHeight)) );

    updatePixmap();
    repaint(false);
}

QRect PanIconWidget::getRegionSelection(void)
{
    return (d->regionSelection);
}

void PanIconWidget::setCursorToLocalRegionSelectionCenter(void)
{
    QCursor::setPos(mapToGlobal(d->localRegionSelection.center()));
}

void PanIconWidget::setCenterSelection(void)
{
    setRegionSelection(QRect( 
             (int)(((float)d->zoomedOrgWidth  / 2.0) - ((float)d->regionSelection.width()  / 2.0)),
             (int)(((float)d->zoomedOrgHeight / 2.0) - ((float)d->regionSelection.height() / 2.0)),
             d->regionSelection.width(),
             d->regionSelection.height()));
}

void PanIconWidget::regionSelectionMoved( bool targetDone )
{
    if (targetDone)
    {
       updatePixmap();          
       repaint(false);
    }
    
    int x = (int)lround( ((float)d->localRegionSelection.x() - (float)d->rect.x() ) *
                         ((float)d->zoomedOrgWidth / (float)d->width) );
                                            
    int y = (int)lround( ((float)d->localRegionSelection.y() - (float)d->rect.y() ) *
                         ((float)d->zoomedOrgHeight / (float)d->height) );
                                            
    int w = (int)lround( (float)d->localRegionSelection.width() *
                         ((float)d->zoomedOrgWidth / (float)d->width) );
                                     
    int h = (int)lround( (float)d->localRegionSelection.height() *
                         ((float)d->zoomedOrgHeight / (float)d->height) );
                     
    d->regionSelection.setX(x);
    d->regionSelection.setY(y);
    d->regionSelection.setWidth(w);
    d->regionSelection.setHeight(h);
       
    emit signalSelectionMoved( d->regionSelection, targetDone );
}

void PanIconWidget::updatePixmap()
{
    // Drawing background and image.
    d->pixmap->fill(colorGroup().background());
    bitBlt(d->pixmap, d->rect.x(), d->rect.y(), &d->image, 0, 0);
    
    QPainter p(d->pixmap);
   
    // Drawing selection border

    if (d->flicker) p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    else p.setPen(QPen(Qt::red, 1, Qt::SolidLine));

    p.drawRect(d->localRegionSelection.x(), 
               d->localRegionSelection.y(),
               d->localRegionSelection.width(), 
               d->localRegionSelection.height());

    if (d->flicker) p.setPen(QPen(Qt::red, 1, Qt::DotLine));
    else p.setPen(QPen(Qt::white, 1, Qt::DotLine));

    p.drawRect(d->localRegionSelection.x(), 
               d->localRegionSelection.y(),
               d->localRegionSelection.width(), 
               d->localRegionSelection.height());

    p.end();
}

void PanIconWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, d->pixmap);
}

void PanIconWidget::setMouseFocus()
{
    raise();
    d->xpos = d->localRegionSelection.center().x();
    d->ypos = d->localRegionSelection.center().y();
    d->moveSelection = true;
    setCursor( KCursor::sizeAllCursor() );           
    emit signalSelectionTakeFocus();
}

void PanIconWidget::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    if ( d->moveSelection )
    {    
        d->moveSelection = false;
        setCursor( KCursor::arrowCursor() );           
        emit signalHiden();  
    }
}

void PanIconWidget::mousePressEvent ( QMouseEvent * e )
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

void PanIconWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->moveSelection )
    {    
       d->moveSelection = false;
       setCursor( KCursor::arrowCursor() );           
       regionSelectionMoved(true);
    }
}

void PanIconWidget::mouseMoveEvent ( QMouseEvent * e )
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

void PanIconWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == d->timerID)
    {
        d->flicker = !d->flicker;
        updatePixmap();
        repaint(false);
    }
    else
        QWidget::timerEvent(e);
}

}  // NameSpace Digikam

