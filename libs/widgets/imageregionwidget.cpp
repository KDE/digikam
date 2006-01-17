/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-17
 * Description : a widget to draw an image clip region.
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
#include <kdebug.h>
#include <kglobal.h>
#include <kdebug.h>

// Local includes

#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{
class ImageRegionWidgetPriv
{
public:

    ImageRegionWidgetPriv()
    {
        movingInProgress = false;
        image            = 0;
        pixmap           = 0;
        pixmapRegion     = 0;
        separateView     = ImageRegionWidget::SeparateViewVertical;
    }

    bool         movingInProgress;

    int          separateView;
    int          xpos;
    int          ypos;

    QPixmap     *pixmap;                // Entire content widget pixmap.
    QPixmap     *pixmapRegion;          // Pixmap of current region to render.
    
    QPointArray  hightlightPoints;
    
    DImg        *image;                // Entire content image.
};

ImageRegionWidget::ImageRegionWidget(int wp, int hp, QWidget *parent, bool scrollBar)
                 : QScrollView(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageRegionWidgetPriv;

    if( !scrollBar ) 
    {
       setVScrollBarMode( QScrollView::AlwaysOff );
       setHScrollBarMode( QScrollView::AlwaysOff );
    }
    
    setMinimumSize(wp, hp);
    viewport()->setMouseTracking(true);

    ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    d->image = new DImg(w, h, sixteenBit, hasAlpha, data);
    delete [] data;
    
    updateOriginalImage();
}

ImageRegionWidget::~ImageRegionWidget()
{
    if (d->pixmap) delete d->pixmap;
    if (d->pixmapRegion) delete d->pixmapRegion;
    if (d->image) delete d->image;
    delete d;
}

void ImageRegionWidget::viewportResizeEvent(QResizeEvent *)
{
    updateOriginalImage();
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
    updateOriginalImage();
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::updateOriginalImage()
{
    updatePixmap(d->image);
}

void ImageRegionWidget::updatePreviewImage(DImg *img)
{
    QPixmap pix = img->convertToPixmap();
    QRect area  = getTargetImageRegion();
    copyBlt( d->pixmap, area.x(), area.y(), &pix, 0, 0, pix.width(), pix.height() );
}

void ImageRegionWidget::updatePixmap(DImg *img)
{
    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );
    
    int w = img->width();
    int h = img->height();
    
    if (d->pixmap)
    {
        delete d->pixmap;
        d->pixmap = 0;
    }
    
    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
        {
            d->pixmap = new QPixmap(w, h);
            d->pixmap->convertFromImage(img->copyQImage());
            resizeContents(w, h);
        }
            break;
        case SeparateViewDuplicateVert:
        {
            QPixmap pix = img->convertToPixmap();
            d->pixmap = new QPixmap(w+visibleWidth()/2, h);
            d->pixmap->fill(paletteBackgroundColor().rgb());
            copyBlt( d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w+visibleWidth()/2, h);
            break;
        }
        case SeparateViewDuplicateHorz:
        {
            QPixmap pix = img->convertToPixmap();
            d->pixmap = new QPixmap(w, h+visibleHeight()/2);
            d->pixmap->fill(paletteBackgroundColor().rgb());
            copyBlt( d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w, h+visibleHeight()/2);
            break;
        }
        default:
            kdWarning() << "Unknown separation view specified"
                        << endl;
    }

    repaintContents(false);    
}

DImg ImageRegionWidget::getImageRegionImage(void)
{
    return ( d->image->copy(getImageRegionToRender()) );
}

void ImageRegionWidget::backupPixmapRegion(void)
{
    if (d->pixmapRegion) delete d->pixmapRegion;
    QRect area = getTargetImageRegion();
    d->pixmapRegion = new QPixmap(area.size());
    copyBlt( d->pixmapRegion, 0, 0, d->pixmap, area.x(), area.y(), area.width(), area.height() );
}

void ImageRegionWidget::restorePixmapRegion(void)
{
    if (!d->pixmapRegion) return;
    QRect area = getTargetImageRegion();
    copyBlt( d->pixmap, area.x(), area.y(), d->pixmapRegion, 0, 0, d->pixmap->width(), d->pixmap->height() );
}

void ImageRegionWidget::drawContents(QPainter *p, int x, int y, int w, int h)
{
    if(!d->pixmap) return;
    else p->drawPixmap(x, y, *d->pixmap, x, y, w, h);
    
    if (!d->movingInProgress)
    {
        // Drawing separate view.
        
        switch (d->separateView)
        {
            case SeparateViewVertical:
            case SeparateViewDuplicateVert:
            {
                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(getTargetImageRegion().topLeft().x(),    
                            getTargetImageRegion().topLeft().y(),
                            getTargetImageRegion().bottomLeft().x(),
                            getTargetImageRegion().bottomLeft().y());
                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(getTargetImageRegion().topLeft().x(),    
                            getTargetImageRegion().topLeft().y()+1,
                            getTargetImageRegion().bottomLeft().x(),
                            getTargetImageRegion().bottomLeft().y()-1);
                            
                p->setPen(QPen::QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p->fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint::QPoint(getTargetImageRegion().topLeft().x()+20, 
                                                   getTargetImageRegion().topLeft().y()+20));
                textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height()) );
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint::QPoint(contentsX()+20, contentsY()+20));
                textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height() ) );       
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                break;
            }
            case SeparateViewHorizontal:
            case SeparateViewDuplicateHorz:
            {
                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(getTargetImageRegion().topLeft().x()+1,
                            getTargetImageRegion().topLeft().y(),
                            getTargetImageRegion().topRight().x()-1,
                            getTargetImageRegion().topRight().y());
                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(getTargetImageRegion().topLeft().x(),
                            getTargetImageRegion().topLeft().y(),
                            getTargetImageRegion().topRight().x(),
                            getTargetImageRegion().topRight().y());
                            
                p->setPen(QPen::QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p->fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint::QPoint(getTargetImageRegion().topLeft().x()+20, 
                                                   getTargetImageRegion().topLeft().y()+20));
                textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height()) );
                p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p->drawRect(textRect);
                p->drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint::QPoint(contentsX()+20, contentsY()+20));
                textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height() ) );       
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
                    ptArea.setSize(QSize::QSize(12, 12));
                    ptArea.moveCenter(pt);
                    p->drawEllipse(ptArea);
                    ptArea.setSize(QSize::QSize(8, 8));
                    ptArea.moveCenter(pt);
                    p->drawEllipse(ptArea);
                    p->setPen(QPen(Qt::black, 1, Qt::SolidLine));
                    ptArea.setSize(QSize::QSize(10, 10));
                    ptArea.moveCenter(pt);
                    p->drawEllipse(ptArea);
                    ptArea.setSize(QSize::QSize(6, 6));
                    ptArea.moveCenter(pt);
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

QRect ImageRegionWidget::getImageRegion(void)
{
    QRect region;    

    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
            region = QRect::QRect(contentsX(), contentsY(), 
                (visibleWidth()  < d->image->width()) ? visibleWidth()  : d->image->width(),
                (visibleHeight() < d->image->height()) ? visibleHeight() : d->image->height());
            break;
        case SeparateViewDuplicateVert:
            region = QRect::QRect(contentsX(), contentsY(), 
                (visibleWidth()/2  < d->image->width()) ? visibleWidth()/2  : d->image->width(),
                (visibleHeight() < d->image->height()) ? visibleHeight() : d->image->height());
            break;
        case SeparateViewDuplicateHorz:
            region = QRect::QRect(contentsX(), contentsY(), 
                (visibleWidth()  < d->image->width()) ? visibleWidth()  : d->image->width(),
                (visibleHeight()/2 < d->image->height()) ? visibleHeight()/2 : d->image->height());
            break;
    }
        
    return region;        
}

QRect ImageRegionWidget::getImageRegionToRender(void)
{
    int normalizedW, normalizedH;
    
    // For large screen.
    if (visibleWidth()  > d->image->width())  normalizedW = d->image->width();
    else normalizedW = visibleWidth();
    if (visibleHeight() > d->image->height()) normalizedH = d->image->height();
    else normalizedH = visibleHeight();

    QRect region;
    
    if (d->separateView == SeparateViewVertical)
        region = QRect::QRect(contentsX()+normalizedW/2, contentsY(), normalizedW/2, normalizedH);
    else if (d->separateView == SeparateViewHorizontal)
        region = QRect::QRect(contentsX(), contentsY()+normalizedH/2, normalizedW, normalizedH/2);
    else if (d->separateView == SeparateViewDuplicateVert)
        region = QRect::QRect(contentsX(), contentsY(), normalizedW/2, normalizedH);
    else if (d->separateView == SeparateViewDuplicateHorz)
        region = QRect::QRect(contentsX(), contentsY(), normalizedW, normalizedH/2);
    else 
        region = QRect::QRect(contentsX(), contentsY(), normalizedW, normalizedH);
            
    return (region);
}

QRect ImageRegionWidget::getTargetImageRegion(void)
{
    QRect region = getImageRegionToRender();
    
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
    setCursor( KCursor::arrowCursor() ); 
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

// ---------------------------------------------------------------------------
// FIXME remove these methods when all image plugins will be ported to DIMG.

void ImageRegionWidget::updatePreviewImage(QImage *img)
{
    QPixmap pix(img->width(), img->height());
    pix.convertFromImage(*img);
    QRect area = getTargetImageRegion();
    copyBlt( d->pixmap, area.x(), area.y(), &pix, 0, 0, pix.width(), pix.height() );
}

void ImageRegionWidget::updatePixmap(QImage *img)
{
    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );
    
    int w = img->width();
    int h = img->height();
    
    if (d->pixmap)
    {
        delete d->pixmap;
        d->pixmap = 0;
    }
    
    switch (d->separateView)
        {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
            {
            d->pixmap = new QPixmap(w, h);
            d->pixmap->convertFromImage(*img);
            resizeContents(w, h);
            }
            break;
        case SeparateViewDuplicateVert:
            {
            QPixmap pix(*img); 
            d->pixmap = new QPixmap(w+visibleWidth()/2, h);
            d->pixmap->fill(paletteBackgroundColor().rgb());
            copyBlt( d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w+visibleWidth()/2, h);
            break;
            }
        case SeparateViewDuplicateHorz:
            {
            QPixmap pix(*img); 
            d->pixmap = new QPixmap(w, h+visibleHeight()/2);
            d->pixmap->fill(paletteBackgroundColor().rgb());
            copyBlt( d->pixmap, 0, 0, &pix, 0, 0, w, h );
            resizeContents(w, h+visibleHeight()/2);
            break;
            }
        default:
            kdWarning() << "Unknown separation view specified"
                        << endl;
        }

    repaintContents(false);    
}

QImage ImageRegionWidget::getImageRegionData(void)
{
    return ( d->image->copyQImage(getImageRegionToRender()) );
}

}  // NameSpace Digikam

#include "imageregionwidget.moc"
