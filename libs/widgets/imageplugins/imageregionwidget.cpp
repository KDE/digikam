/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-08-17
 * Description : a widget to draw an image clip region.
 * 
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cmath>

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

// Local includes.

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
        iface        = 0;
        separateView = ImageRegionWidget::SeparateViewVertical;
    }

    int          separateView;
    int          xpos;
    int          ypos;

    QPixmap      pixmapRegion;          // Pixmap of current region to render.
    
    QPointArray  hightlightPoints;
    
    DImg         image;                 // Entire content image to render pixmap.
    
    ImageIface  *iface;
};

ImageRegionWidget::ImageRegionWidget(int wp, int hp, QWidget *parent, bool scrollBar)
                 : PreviewWidget(parent)
{
    d = new ImageRegionWidgetPriv;
    d->iface = new ImageIface(0, 0);
    d->image = d->iface->getOriginalImg()->copy(); 

    setMinimumSize(wp, hp);
    setBackgroundColor(colorGroup().background());

    if( !scrollBar ) 
    {
       setVScrollBarMode( QScrollView::AlwaysOff );
       setHScrollBarMode( QScrollView::AlwaysOff );
    }
    
    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged()));
}

ImageRegionWidget::~ImageRegionWidget()
{
    if (d->iface) delete d->iface;
    delete d;
}

void ImageRegionWidget::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    QScrollView::resizeEvent(e);

    // NOTE: We will always adapt the min. zoom factor to the visible size of canvas
 
    double srcWidth  = previewWidth();
    double srcHeight = previewHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();
    double zoom      = QMAX(dstWidth/srcWidth, dstHeight/srcHeight); 

    setZoomMin(zoom);
    setZoomMax(zoom*12.0);
    setZoomFactor(zoom);
}

int ImageRegionWidget::previewWidth()
{
    return d->image.width();
}

int ImageRegionWidget::previewHeight()
{
    return d->image.height();
}

bool ImageRegionWidget::previewIsNull()
{
    return d->image.isNull();
}

void ImageRegionWidget::resetPreview()
{
    d->image.reset();
}

void ImageRegionWidget::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img = d->image.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());    
    QPixmap pix2 = d->iface->convertToPixmap(img);
    bitBlt(pix, 0, 0, &pix2, 0, 0);
}

void ImageRegionWidget::setHighLightPoints(const QPointArray& pointsList)
{
    d->hightlightPoints = pointsList;
    repaintContents(false);   
}

void ImageRegionWidget::slotZoomFactorChanged()
{
    emit signalContentsMovedEvent(true);
}

void ImageRegionWidget::slotSeparateViewToggled(int mode)
{
    d->separateView = mode;
    updateContentsSize();
    slotZoomFactorChanged(); 
}

QRect ImageRegionWidget::getImageRegion()
{
    QRect region;    

    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
            region = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()); 
            break;
        case SeparateViewDuplicateVert:
            region = QRect(contentsX(), contentsY(), visibleWidth()/2, visibleHeight());  
            break;
        case SeparateViewDuplicateHorz:
            region = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()/2);  
            break;
    }
        
    return region;   
}

void ImageRegionWidget::viewportPaintExtraData()
{   
    if (!m_movingInProgress && !d->pixmapRegion.isNull())
    {
        QPainter p(viewport());
        QRect region = getLocalTargetImageRegion();
        QRect rt(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight())); 

        region = getLocalImageRegionToRender();
        QRect ro(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight())); 

        bitBlt(viewport(), rt.x(), rt.y(), &d->pixmapRegion, 0, 0, rt.width(), rt.height());

        // Drawing separate view.
        
        switch (d->separateView)
        {
            case SeparateViewVertical:
            case SeparateViewDuplicateVert:
            {
                p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p.drawLine(rt.topLeft().x(),    rt.topLeft().y(),
                           rt.bottomLeft().x(), rt.bottomLeft().y());
                p.setPen(QPen(Qt::red, 2, Qt::DotLine));
                p.drawLine(rt.topLeft().x(),    rt.topLeft().y()+1,
                           rt.bottomLeft().x(), rt.bottomLeft().y()-1);
                            
                p.setPen(QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p.fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
                p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p.drawRect(textRect);
                p.drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 

                if (d->separateView == SeparateViewVertical)
                    ro.moveBy(-ro.width(), 0);

                textRect.setTopLeft(QPoint(ro.topLeft().x()+20, ro.topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );       
                p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p.drawRect(textRect);
                p.drawText(textRect, Qt::AlignCenter, text);
                break;
            }
            case SeparateViewHorizontal:
            case SeparateViewDuplicateHorz:
            {
                p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p.drawLine(rt.topLeft().x()+1,  rt.topLeft().y(),
                           rt.topRight().x()-1, rt.topRight().y());
                p.setPen(QPen(Qt::red, 2, Qt::DotLine));
                p.drawLine(rt.topLeft().x(),   rt.topLeft().y(),
                           rt.topRight().x(), rt.topRight().y());
                            
                p.setPen(QPen(Qt::red, 1)) ;                    
                QFontMetrics fontMt = p.fontMetrics();
                
                QString text(i18n("Target"));
                QRect textRect;
                QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
                textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
                p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p.drawRect(textRect);
                p.drawText(textRect, Qt::AlignCenter, text);
                            
                text = i18n("Original");                    
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);

                if (d->separateView == SeparateViewHorizontal)
                    ro.moveBy(0, -ro.height());

                textRect.setTopLeft(QPoint(ro.topLeft().x()+20, ro.topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );       
                p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
                p.drawRect(textRect);
                p.drawText(textRect, Qt::AlignCenter, text);
                break;
            }
        }
        
        // Drawing HighLighted points.
        
        if (!d->hightlightPoints.isEmpty())
        {
            QPoint pt;
            QRect  hpArea;
            
            for (int i = 0 ; i < d->hightlightPoints.count() ; i++)
            {
                pt = d->hightlightPoints.point(i);
                
                if ( getImageRegionToRender().contains(pt) )
                {
                    int x = (int)(((double)pt.x() * tileSize()) / floor(tileSize() / zoomFactor()));
                    int y = (int)(((double)pt.y() * tileSize()) / floor(tileSize() / zoomFactor()));

                    QPoint hp(contentsToViewport(QPoint(x, y)));
                    hpArea.setSize(QSize((int)(16*zoomFactor()), (int)(16*zoomFactor())));
                    hpArea.moveCenter(hp);

                    p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
                    p.drawLine(hp.x(), hpArea.y(), 
                               hp.x(), hp.y()-(int)(3*zoomFactor()));
                    p.drawLine(hp.x(), hp.y()+(int)(3*zoomFactor()), 
                               hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(),                   hp.y(), 
                               hp.x()-(int)(3*zoomFactor()), hp.y());
                    p.drawLine(hp.x()+(int)(3*zoomFactor()), hp.y(), 
                               hpArea.right(),                hp.y());

                    p.setPen(QPen(Qt::red, 2, Qt::DotLine));
                    p.drawLine(hp.x(), hpArea.y(), 
                               hp.x(), hp.y()-(int)(3*zoomFactor()));
                    p.drawLine(hp.x(), hp.y()+(int)(3*zoomFactor()), 
                               hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(),                   hp.y(), 
                               hp.x()-(int)(3*zoomFactor()), hp.y());
                    p.drawLine(hp.x()+(int)(3*zoomFactor()), hp.y(), 
                               hpArea.right(),                hp.y());
                }
            }
        }
        p.end();
    }
}

void ImageRegionWidget::setCenterContentsPosition()
{
    center(contentsWidth()/2, contentsHeight()/2);    
    slotZoomFactorChanged();
}

void ImageRegionWidget::setContentsPosition(int x, int y, bool targetDone)
{
    if( targetDone )
        m_movingInProgress = false;

    setContentsPos(x, y);    
    
    if( targetDone )
       slotZoomFactorChanged();
}

void ImageRegionWidget::backupPixmapRegion()
{
    d->pixmapRegion = QPixmap();
}

void ImageRegionWidget::restorePixmapRegion()
{
    m_movingInProgress = true;
    viewport()->repaint(false);
}

void ImageRegionWidget::updatePreviewImage(DImg *img)
{
    DImg image = img->copy();
    QRect r    = getLocalImageRegionToRender();
    image.resize(r.width(), r.height());

    // Because image plugins are tool witch only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to 
    // restore the embedded ICC color profile. 
    image.setICCProfil(d->image.getICCProfil());
    d->pixmapRegion = d->iface->convertToPixmap(image);
}

DImg ImageRegionWidget::getImageRegionImage()
{
    return (d->image.copy(getImageRegionToRender()));
}

QRect ImageRegionWidget::getImageRegionToRender()
{
    QRect r = getLocalImageRegionToRender();

    int x = (int)(((double)r.x()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int y = (int)(((double)r.y()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int w = (int)(((double)r.width()  / tileSize()) * floor(tileSize() / zoomFactor()));   
    int h = (int)(((double)r.height() / tileSize()) * floor(tileSize() / zoomFactor()));

    QRect rect(x, y, w, h);
    return (rect);
}

QRect ImageRegionWidget::getLocalImageRegionToRender()
{
    QRect region;
    
    if (d->separateView == SeparateViewVertical)
    {
        region = QRect((int)ceilf(contentsX()+visibleWidth()/2.0), contentsY(), 
                       (int)ceilf(visibleWidth()/2.0),             visibleHeight());
    }
    else if (d->separateView == SeparateViewHorizontal)
    {
        region = QRect(contentsX(),    (int)ceilf(contentsY()+visibleHeight()/2.0), 
                       visibleWidth(), (int)ceilf(visibleHeight()/2.0));
    }
    else if (d->separateView == SeparateViewDuplicateVert)
    {
        region = QRect(contentsX(),                    contentsY(), 
                       (int)ceilf(visibleWidth()/2.0), visibleHeight());
    }
    else if (d->separateView == SeparateViewDuplicateHorz)
    {
        region = QRect(contentsX(),    contentsY(), 
                       visibleWidth(), (int)ceilf(visibleHeight()/2.0));
    }
    else 
    {
        region = QRect(contentsX(),    contentsY(), 
                       visibleWidth(), visibleHeight());
    }
    
    return (region);
}

QRect ImageRegionWidget::getLocalTargetImageRegion()
{
    QRect region = getLocalImageRegionToRender();
    
    if (d->separateView == SeparateViewDuplicateVert)
        region.moveBy(region.width(), 0);
    else if (d->separateView == SeparateViewDuplicateHorz)
        region.moveBy(0, region.height());
    
    return region;
}

void ImageRegionWidget::setContentsSize()
{
    switch (d->separateView)
    {
        case SeparateViewVertical:
        case SeparateViewHorizontal:
        case SeparateViewNone:
        {
            PreviewWidget::setContentsSize();
            break;
        }
        case SeparateViewDuplicateVert:
        {
            resizeContents(zoomWidth()+visibleWidth()/2, zoomHeight());
            break;
        }
        case SeparateViewDuplicateHorz:
        {
            resizeContents(zoomWidth(), zoomHeight()+visibleHeight()/2);
            break;
        }
        default:
            DWarning() << "Unknown separation view specified" << endl;
    }
}

void ImageRegionWidget::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->state() & Qt::ControlButton)
    {
        if (e->delta() < 0 && !maxZoom())
            slotIncreaseZoom();
        else if (e->delta() > 0 && !minZoom())
            slotDecreaseZoom();
        return;
    }
}

}  // NameSpace Digikam
