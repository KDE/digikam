/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-17
 * Description : a widget to draw an image clip region.
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

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qpen.h>
#include <qimage.h>
#include <qbrush.h>
#include <qfont.h> 
#include <qfontmetrics.h> 
#include <qpointarray.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kapplication.h>

// Local includes

#include "ddebug.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "imageregionwidget.moc"

namespace Digikam
{
class ImageRegionWidgetPriv
{
public:

    ImageRegionWidgetPriv()
    {
        iface            = 0;
        movingInProgress = false;
        pixmapRegion     = 0;
        zoomFactor       = 1.0;
        separateView     = ImageRegionWidget::SeparateViewVertical;
    }

    bool         movingInProgress;

    int          separateView;
    int          xpos;
    int          ypos;

    double       zoomFactor;

    QPixmap      pixmap;                // Entire content widget pixmap.
    QPixmap     *pixmapRegion;          // Pixmap of current region to render.
    
    QPointArray  hightlightPoints;
    
    DImg         image;                 // Entire content image to render pixmap.
    
    ImageIface  *iface;
};

ImageRegionWidget::ImageRegionWidget(int wp, int hp, QWidget *parent, bool scrollBar)
                 : QScrollView(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageRegionWidgetPriv;
    d->iface = new ImageIface(0, 0);

    if( !scrollBar ) 
    {
       setVScrollBarMode( QScrollView::AlwaysOff );
       setHScrollBarMode( QScrollView::AlwaysOff );
    }
    
    setMinimumSize(wp, hp);
    viewport()->setMouseTracking(true);
    viewport()->setPaletteBackgroundColor(colorGroup().background());
    slotZoomFactorChanged(1.0);
}

ImageRegionWidget::~ImageRegionWidget()
{
    if (d->iface)        delete d->iface;
    if (d->pixmapRegion) delete d->pixmapRegion;
    delete d;
}

void ImageRegionWidget::slotZoomFactorChanged(double factor)
{
    QPoint currentPos((int)(contentsX() / d->zoomFactor), (int)(contentsY() / d->zoomFactor));
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    d->image = d->iface->getOriginalImg()->copy(); 

    d->zoomFactor = factor;

    if(d->zoomFactor != 1.0)
    {
        DImg img = d->image.smoothScale((uint)(d->image.width()  * d->zoomFactor), 
                                        (uint)(d->image.height() * d->zoomFactor));
        d->image = img;
    }
    
    updateOriginalImage();
    setContentsPos((int)(currentPos.x() * d->zoomFactor), (int)(currentPos.y() * d->zoomFactor));
    kapp->restoreOverrideCursor();
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::viewportResizeEvent(QResizeEvent *)
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    updateOriginalImage();
    kapp->restoreOverrideCursor();
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::slotTimerResizeEvent()
{
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::setHighLightPoints(QPointArray pointsList)
{
    d->hightlightPoints = pointsList;
    repaintContents(false);   
}

void ImageRegionWidget::slotSeparateViewToggled(int mode)
{
    d->separateView = mode;
    kapp->setOverrideCursor( KCursor::waitCursor() );
    updateOriginalImage();
    kapp->restoreOverrideCursor();
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::updateOriginalImage()
{
    updatePixmap(d->image);
}

void ImageRegionWidget::updatePixmap(DImg& img)
{
    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );
    
    int w = img.width();
    int h = img.height();
    
    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
        {
            d->pixmap = d->iface->convertToPixmap(img);
            resizeContents(w, h);
            break;
        }
        case SeparateViewDuplicateVert:
        {
            QPixmap pix = d->iface->convertToPixmap(img);
            d->pixmap   = QPixmap(w+visibleWidth()/2, h);
            d->pixmap.fill(paletteBackgroundColor().rgb());
            copyBlt( &d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w+visibleWidth()/2, h);
            break;
        }
        case SeparateViewDuplicateHorz:
        {
            QPixmap pix = d->iface->convertToPixmap(img);
            d->pixmap   = QPixmap(w, h+visibleHeight()/2);
            d->pixmap.fill(paletteBackgroundColor().rgb());
            copyBlt( &d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w, h+visibleHeight()/2);
            break;
        }
        default:
            DWarning() << "Unknown separation view specified" << endl;
    }

    repaintContents(false);    
}

void ImageRegionWidget::drawContents(QPainter *p, int x, int y, int w, int h)
{
    p->drawPixmap(x, y, d->pixmap, x, y, w, h);
    
    if (!d->movingInProgress)
    {
        // Drawing separate view.
        
        switch (d->separateView)
        {
            case SeparateViewVertical:
            case SeparateViewDuplicateVert:
            {
                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(getLocalTargetImageRegion().topLeft().x(),
                            getLocalTargetImageRegion().topLeft().y(),
                            getLocalTargetImageRegion().bottomLeft().x(),
                            getLocalTargetImageRegion().bottomLeft().y());
                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(getLocalTargetImageRegion().topLeft().x(),
                            getLocalTargetImageRegion().topLeft().y()+1,
                            getLocalTargetImageRegion().bottomLeft().x(),
                            getLocalTargetImageRegion().bottomLeft().y()-1);
                            
                p->setPen(QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p->fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(getLocalTargetImageRegion().topLeft().x()+20,
                                           getLocalTargetImageRegion().topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width(), fontRect.height()) );
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(contentsX()+20, contentsY()+20));
                textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );       
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                break;
            }
            case SeparateViewHorizontal:
            case SeparateViewDuplicateHorz:
            {
                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(getLocalTargetImageRegion().topLeft().x()+1,
                            getLocalTargetImageRegion().topLeft().y(),
                            getLocalTargetImageRegion().topRight().x()-1,
                            getLocalTargetImageRegion().topRight().y());
                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(getLocalTargetImageRegion().topLeft().x(),
                            getLocalTargetImageRegion().topLeft().y(),
                            getLocalTargetImageRegion().topRight().x(),
                            getLocalTargetImageRegion().topRight().y());
                            
                p->setPen(QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p->fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(getLocalTargetImageRegion().topLeft().x()+20,
                                           getLocalTargetImageRegion().topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width(), fontRect.height()) );
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(contentsX()+20, contentsY()+20));
                textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );       
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                break;
            }
        }
        
        // Drawing HighLighted points.
        
        if (!d->hightlightPoints.isEmpty())
        {
            QPoint pt;
            QRect  ptArea;
            
            for (uint i = 0 ; i < d->hightlightPoints.count() ; i++)
            {
                pt = d->hightlightPoints.point(i);
                
                if ( getImageRegionToRender().contains(pt) )
                {
                    p->setPen(QPen(Qt::white, 1, Qt::SolidLine));
                    ptArea.setSize(QSize(12, 12));
                    ptArea.moveCenter(QPoint((int)(pt.x() * d->zoomFactor), (int)(pt.y() * d->zoomFactor)));
                    p->drawEllipse(ptArea);
                    ptArea.setSize(QSize(8, 8));
                    ptArea.moveCenter(QPoint((int)(pt.x() * d->zoomFactor), (int)(pt.y() * d->zoomFactor)));
                    p->drawEllipse(ptArea);
                    p->setPen(QPen(Qt::black, 1, Qt::SolidLine));
                    ptArea.setSize(QSize(10, 10));
                    ptArea.moveCenter(QPoint((int)(pt.x() * d->zoomFactor), (int)(pt.y() * d->zoomFactor)));
                    p->drawEllipse(ptArea);
                    ptArea.setSize(QSize(6, 6));
                    ptArea.moveCenter(QPoint((int)(pt.x() * d->zoomFactor), (int)(pt.y() * d->zoomFactor)));
                    p->drawEllipse(ptArea);
                }
            }
        }
    }
}

void ImageRegionWidget::setCenterContentsPosition(void)
{
    center(contentsWidth()/2, contentsHeight()/2);    
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::setContentsPosition(int x, int y, bool targetDone)
{
    setContentsPos(x, y);    
    
    if( targetDone )
       d->movingInProgress = false;
    else
       d->movingInProgress = true;
    
    repaintContents(false);    
    
    if( targetDone )
       emit contentsMovedEvent(true);
}

void ImageRegionWidget::backupPixmapRegion(void)
{
    if (d->pixmapRegion) delete d->pixmapRegion;
    QRect area      = getLocalTargetImageRegion();
    d->pixmapRegion = new QPixmap(area.size());
    copyBlt( d->pixmapRegion, 0, 0, &d->pixmap, area.x(), area.y(), area.width(), area.height() );
}

void ImageRegionWidget::restorePixmapRegion(void)
{
    if (!d->pixmapRegion) return;
    QRect area = getLocalTargetImageRegion();
    copyBlt( &d->pixmap, area.x(), area.y(), d->pixmapRegion, 0, 0, area.width(), area.height() );
}

void ImageRegionWidget::updatePreviewImage(DImg *img)
{
    DImg image = img->copy();

    // Because image plugins are tool witch only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to 
    // restore the embedded ICC color profile. 
    image.setICCProfil(d->image.getICCProfil());

    image.resize((uint)(image.width()  * d->zoomFactor), 
                 (uint)(image.height() * d->zoomFactor));
    QPixmap pix = d->iface->convertToPixmap(image);
    QRect area  = getLocalTargetImageRegion();
    copyBlt( &d->pixmap, area.x(), area.y(), &pix, 0, 0, area.width(), area.height() );
}

DImg ImageRegionWidget::getImageRegionImage(void)
{
    return ( d->iface->getOriginalImg()->copy(getImageRegionToRender()) );
}

QRect ImageRegionWidget::getImageRegion(void)
{
    QRect region;    

    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
            region = QRect(contentsX(), contentsY(), 
                (visibleWidth()  < d->image.width()) ? visibleWidth()  : d->image.width(),
                (visibleHeight() < d->image.height()) ? visibleHeight() : d->image.height());
            break;
        case SeparateViewDuplicateVert:
            region = QRect(contentsX(), contentsY(), 
                (visibleWidth()/2  < d->image.width()) ? visibleWidth()/2  : d->image.width(),
                (visibleHeight() < d->image.height()) ? visibleHeight() : d->image.height());
            break;
        case SeparateViewDuplicateHorz:
            region = QRect(contentsX(), contentsY(), 
                (visibleWidth()  < d->image.width()) ? visibleWidth()  : d->image.width(),
                (visibleHeight()/2 < d->image.height()) ? visibleHeight()/2 : d->image.height());
            break;
    }
        
    return region;        
}

QRect ImageRegionWidget::getImageRegionToRender(void)
{
    QRect region = getLocalImageRegionToRender();
    region.setX((int)(region.x()/d->zoomFactor));
    region.setY((int)(region.y()/d->zoomFactor));
    region.setWidth((int)(region.width()/d->zoomFactor));
    region.setHeight((int)(region.height()/d->zoomFactor));
    return (region);
}

QRect ImageRegionWidget::getLocalImageRegionToRender(void)
{
    int normalizedW, normalizedH;
    
    // For large screen.
    if (visibleWidth()  > d->image.width())  normalizedW = d->image.width();
    else normalizedW = visibleWidth();
    if (visibleHeight() > d->image.height()) normalizedH = d->image.height();
    else normalizedH = visibleHeight();

    QRect region;
    
    if (d->separateView == SeparateViewVertical)
        region = QRect(contentsX()+normalizedW/2, contentsY(), normalizedW/2, normalizedH);
    else if (d->separateView == SeparateViewHorizontal)
        region = QRect(contentsX(), contentsY()+normalizedH/2, normalizedW, normalizedH/2);
    else if (d->separateView == SeparateViewDuplicateVert)
        region = QRect(contentsX(), contentsY(), normalizedW/2, normalizedH);
    else if (d->separateView == SeparateViewDuplicateHorz)
        region = QRect(contentsX(), contentsY(), normalizedW, normalizedH/2);
    else 
        region = QRect(contentsX(), contentsY(), normalizedW, normalizedH);
            
    return (region);
}

QRect ImageRegionWidget::getLocalTargetImageRegion(void)
{
    QRect region = getLocalImageRegionToRender();
    
    if (d->separateView == SeparateViewDuplicateVert)
        region.moveBy(region.width(), 0);
    else if (d->separateView == SeparateViewDuplicateHorz)
        region.moveBy(0, region.height());
    
    return region;
}

void ImageRegionWidget::contentsMousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
       d->xpos = e->x();
       d->ypos = e->y();
       d->movingInProgress = true;
       setCursor( KCursor::sizeAllCursor() );    
       restorePixmapRegion();
    }
}

void ImageRegionWidget::contentsMouseReleaseEvent ( QMouseEvent *  )
{
    d->movingInProgress = false;
    unsetCursor(); 
    backupPixmapRegion();
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
    {
       uint newxpos = e->x();
       uint newypos = e->y();
       
       scrollBy (-(newxpos - d->xpos), -(newypos - d->ypos));
       repaintContents(false);    
       
       d->xpos = newxpos - (newxpos-d->xpos);
       d->ypos = newypos - (newypos-d->ypos);
       emit contentsMovedEvent(false);
       return;
    }

    setCursor( KCursor::handCursor() );    
}

}  // NameSpace Digikam

