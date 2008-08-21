/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright (C) 2006-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qstring.h>
#include <qcache.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qtimer.h>
#include <qguardedptr.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "previewwidget.h"
#include "previewwidget.moc"

namespace Digikam
{

class PreviewWidgetPriv
{
public:

    PreviewWidgetPriv() :
        tileSize(128), zoomMultiplier(1.2) 
    {
        midButtonX = 0;
        midButtonY = 0;
        autoZoom   = false;
        fullScreen = false;
        zoom       = 1.0;
        minZoom    = 0.1;
        maxZoom    = 12.0;
        zoomWidth  = 0;
        zoomHeight = 0;
        tileTmpPix = new QPixmap(tileSize, tileSize);

        tileCache.setMaxCost((10*1024*1024)/(tileSize*tileSize*4));
        tileCache.setAutoDelete(true);
    }

    bool            autoZoom;
    bool            fullScreen;

    const int       tileSize;
    int             midButtonX;
    int             midButtonY;
    int             zoomWidth;
    int             zoomHeight;

    double          zoom;
    double          minZoom;
    double          maxZoom;
    const double    zoomMultiplier;

    QPoint          centerZoomPoint;

    QRect           pixmapRect;

    QCache<QPixmap> tileCache;

    QPixmap*        tileTmpPix;

    QColor          bgColor;
};

PreviewWidget::PreviewWidget(QWidget *parent)
             : QScrollView(parent, 0, Qt::WDestructiveClose)
{
    d = new PreviewWidgetPriv;
    d->bgColor.setRgb(0, 0, 0);
    m_movingInProgress = false;

    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(false);

    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );

    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain); 
    setMargin(0); 
    setLineWidth(1);
}

PreviewWidget::~PreviewWidget()
{
    delete d->tileTmpPix;
    delete d;
}

void PreviewWidget::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
        return;

    d->bgColor = color;
    viewport()->update();
}

void PreviewWidget::slotReset()
{
    d->tileCache.clear();
    resetPreview();
}

QRect PreviewWidget::previewRect()
{
    return d->pixmapRect;
}

int PreviewWidget::tileSize()
{
    return d->tileSize;
}

int PreviewWidget::zoomWidth()
{
    return d->zoomWidth;
}

int PreviewWidget::zoomHeight()
{
    return d->zoomHeight;
}

double PreviewWidget::zoomMax()
{
    return d->maxZoom;
}

double PreviewWidget::zoomMin()
{
    return d->minZoom;
}

void PreviewWidget::setZoomMax(double z)
{
    d->maxZoom = ceilf(z * 10000.0) / 10000.0;
}

void PreviewWidget::setZoomMin(double z)
{
    d->minZoom = floor(z * 10000.0) / 10000.0;
}

bool PreviewWidget::maxZoom()
{
    return (d->zoom >= d->maxZoom);
}

bool PreviewWidget::minZoom()
{
    return (d->zoom <= d->minZoom);
}

double PreviewWidget::snapZoom(double zoom)
{
    // If the zoom value gets changed from d->zoom to zoom
    // across 50%, 100% or fit-to-window, then return the
    // the corresponding special value. Otherwise zoom is returned unchanged.
    double fit = calcAutoZoomFactor(ZoomInOrOut);
    QValueList<double> snapValues;
    snapValues.append(0.5);
    snapValues.append(1.0);
    snapValues.append(fit);
    qHeapSort(snapValues);
    QValueList<double>::const_iterator it;

    if (d->zoom < zoom) 
    {
        for(it = snapValues.constBegin(); it != snapValues.constEnd(); ++it)
        {
            double z = *it;
            if ((d->zoom < z) && (zoom > z))
            {
                 zoom = z;
                 break;
            }
        }
    } 
    else
    {
        for(it = snapValues.constEnd(); it != snapValues.constBegin(); --it)
        {
            double z = *it;
            if ((d->zoom > z) && (zoom < z))
            {
                 zoom = z;
                 break;
            }
        }
    }

    return zoom;
}

void PreviewWidget::slotIncreaseZoom()
{
    double zoom = d->zoom * d->zoomMultiplier;
    zoom = snapZoom(zoom > zoomMax() ? zoomMax() : zoom);
    setZoomFactor(zoom);
}

void PreviewWidget::slotDecreaseZoom()
{
    double zoom = d->zoom / d->zoomMultiplier;
    zoom = snapZoom(zoom < zoomMin() ? zoomMin() : zoom);
    setZoomFactor(zoom);
}

void PreviewWidget::setZoomFactorSnapped(double zoom)
{
    double fit = calcAutoZoomFactor(ZoomInOrOut);
    if (fabs(zoom-1.0) < 0.05) 
    {
        zoom = 1.0;
    }
    if (fabs(zoom-0.5) < 0.05) 
    {
        zoom = 0.5;
    }
    if (fabs(zoom-fit) < 0.05) 
    {
        zoom = fit;
    }

    setZoomFactor(zoom);
}

void PreviewWidget::setZoomFactor(double zoom)
{
    setZoomFactor(zoom, false);
}

void PreviewWidget::setZoomFactor(double zoom, bool centerView)
{
    // Zoom using center of canvas and given zoom factor.

    double oldZoom = d->zoom;
    double cpx, cpy;

    if (d->centerZoomPoint.isNull())
    {
        // center on current center
        // store old center pos
        cpx = contentsX() + visibleWidth()  / 2.0;
        cpy = contentsY() + visibleHeight() / 2.0;

        cpx = ( cpx / d->tileSize ) * floor(d->tileSize / d->zoom);
        cpy = ( cpy / d->tileSize ) * floor(d->tileSize / d->zoom);
    }
    else
    {
        // keep mouse pointer position constant
        // store old content pos
        cpx = contentsX();
        cpy = contentsY();
    }

    // To limit precision of zoom value and reduce error with check of max/min zoom.
    d->zoom       = floor(zoom * 10000.0) / 10000.0;
    d->zoomWidth  = (int)(previewWidth()  * d->zoom);
    d->zoomHeight = (int)(previewHeight() * d->zoom);

    updateContentsSize();

    // adapt step size to zoom factor. Overall, using a finer step size than scrollbar default.
    int step = QMAX(2, 2*lround(d->zoom));
    horizontalScrollBar()->setLineStep( step );
    horizontalScrollBar()->setPageStep( step * 10 );
    verticalScrollBar()->setLineStep( step );
    verticalScrollBar()->setPageStep( step * 10 );

    viewport()->setUpdatesEnabled(false);
    if (d->centerZoomPoint.isNull())
    {
        cpx = ( cpx * d->tileSize ) / floor(d->tileSize / d->zoom);
        cpy = ( cpy * d->tileSize ) / floor(d->tileSize / d->zoom);

        if (centerView)
        {
            cpx = d->zoomWidth/2.0;
            cpy = d->zoomHeight/2.0;
        }

        center((int)cpx, (int)(cpy));
    }
    else
    {
        cpx = d->zoom * d->centerZoomPoint.x() / oldZoom - d->centerZoomPoint.x() + cpx;
        cpy = d->zoom * d->centerZoomPoint.y() / oldZoom - d->centerZoomPoint.y() + cpy;

        setContentsPos((int)cpx, (int)(cpy));
    }
    viewport()->setUpdatesEnabled(true);
    viewport()->update();

    zoomFactorChanged(d->zoom);
}

double PreviewWidget::zoomFactor()
{
    return d->zoom; 
}

bool PreviewWidget::isFitToWindow()
{
    return d->autoZoom;
}

void PreviewWidget::fitToWindow()
{
    updateAutoZoom();
    updateContentsSize();
    zoomFactorChanged(d->zoom);
    viewport()->update();
}

void PreviewWidget::toggleFitToWindow()
{
    d->autoZoom = !d->autoZoom;

    if (d->autoZoom)
    {
        updateAutoZoom();
    }
    else
    {
        d->zoom = 1.0;
        zoomFactorChanged(d->zoom);
    }

    updateContentsSize();
    viewport()->update();
}

void PreviewWidget::toggleFitToWindowOr100()
{
    // If the current zoom is 100%, then fit to window.
    if (d->zoom == 1.0) 
    {
        fitToWindow();
    }
    else
    {
        setZoomFactor(1.0, true);
    }
}

void PreviewWidget::updateAutoZoom(AutoZoomMode mode)
{
    d->zoom       = calcAutoZoomFactor(mode);
    d->zoomWidth  = (int)(previewWidth()  * d->zoom);
    d->zoomHeight = (int)(previewHeight() * d->zoom);

    zoomFactorChanged(d->zoom);
}

double PreviewWidget::calcAutoZoomFactor(AutoZoomMode mode)
{
    if (previewIsNull()) return d->zoom;

    double srcWidth  = previewWidth();
    double srcHeight = previewHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();

    double zoom = QMIN(dstWidth/srcWidth, dstHeight/srcHeight);
    // limit precision as above
    zoom = floor(zoom * 10000.0) / 10000.0;
    if (mode == ZoomInOrOut)
        // fit to available space, scale up or down
        return zoom;
    else
        // ZoomInOnly: accept that an image is smaller than available space, dont scale up
        return QMIN(1.0, zoom);
}

void PreviewWidget::updateContentsSize()
{
    viewport()->setUpdatesEnabled(false);

    if (visibleWidth() > d->zoomWidth || visibleHeight() > d->zoomHeight)
    {
        // Center the image
        int centerx = contentsRect().width()/2;
        int centery = contentsRect().height()/2;
        int xoffset = int(centerx - d->zoomWidth/2);
        int yoffset = int(centery - d->zoomHeight/2);
        xoffset     = QMAX(xoffset, 0);
        yoffset     = QMAX(yoffset, 0);

        d->pixmapRect = QRect(xoffset, yoffset, d->zoomWidth, d->zoomHeight);
    }
    else
    {
        d->pixmapRect = QRect(0, 0, d->zoomWidth, d->zoomHeight);
    }

    d->tileCache.clear();
    setContentsSize();
    viewport()->setUpdatesEnabled(true);
}

void PreviewWidget::setContentsSize()
{
    resizeContents(d->zoomWidth, d->zoomHeight);
}

void PreviewWidget::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    QScrollView::resizeEvent(e);

    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();

    // No need to repaint. its called   
    // automatically after resize

    // To be sure than corner widget used to pan image will be hide/show 
    // accordinly with resize event.
    zoomFactorChanged(d->zoom);
}

void PreviewWidget::viewportPaintEvent(QPaintEvent *e)
{
    QRect er(e->rect());
    er = QRect(QMAX(er.x()      - 1, 0),
               QMAX(er.y()      - 1, 0),
               QMIN(er.width()  + 2, contentsRect().width()),
               QMIN(er.height() + 2, contentsRect().height()));
    
    bool antialias = (d->zoom <= 1.0) ? true : false;

    QRect o_cr(viewportToContents(er.topLeft()), viewportToContents(er.bottomRight()));
    QRect cr = o_cr;

    QRegion clipRegion(er);
    cr = d->pixmapRect.intersect(cr);

    if (!cr.isEmpty() && !previewIsNull())
    {
        clipRegion -= QRect(contentsToViewport(cr.topLeft()), cr.size());

        QRect pr = QRect(cr.x() - d->pixmapRect.x(), cr.y() - d->pixmapRect.y(),
                         cr.width(), cr.height());

        int x1 = (int)floor((double)pr.x()      / (double)d->tileSize) * d->tileSize;
        int y1 = (int)floor((double)pr.y()      / (double)d->tileSize) * d->tileSize;
        int x2 = (int)ceilf((double)pr.right()  / (double)d->tileSize) * d->tileSize;
        int y2 = (int)ceilf((double)pr.bottom() / (double)d->tileSize) * d->tileSize;

        QPixmap pix(d->tileSize, d->tileSize);
        int sx, sy, sw, sh;
        int step = (int)floor(d->tileSize / d->zoom); 

        for (int j = y1 ; j < y2 ; j += d->tileSize)
        {
            for (int i = x1 ; i < x2 ; i += d->tileSize)
            {
                QString key  = QString("%1,%2").arg(i).arg(j);
                QPixmap *pix = d->tileCache.find(key);
                
                if (!pix)
                {
                    if (antialias)
                    {
                        pix = new QPixmap(d->tileSize, d->tileSize);
                        d->tileCache.insert(key, pix);
                    }
                    else
                    {
                        pix = d->tileTmpPix;
                    }

                    pix->fill(d->bgColor);

                    sx = (int)floor((double)i / d->tileSize ) * step;
                    sy = (int)floor((double)j / d->tileSize ) * step;
                    sw = step;
                    sh = step;

                    paintPreview(pix, sx, sy, sw, sh);
                }

                QRect  r(i, j, d->tileSize, d->tileSize);
                QRect  ir = pr.intersect(r);
                QPoint pt(contentsToViewport(QPoint(ir.x() + d->pixmapRect.x(),
                                                    ir.y() + d->pixmapRect.y())));

                bitBlt(viewport(), pt.x(), pt.y(),
                       pix,
                       ir.x()-r.x(), ir.y()-r.y(),
                       ir.width(), ir.height());
            }
        }
    }

    QPainter p(viewport());
    p.setClipRegion(clipRegion);
    p.fillRect(er, d->bgColor);
    p.end();

    viewportPaintExtraData();
}

void PreviewWidget::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e || e->button() == Qt::RightButton)
        return;

    m_movingInProgress = false;

    if (e->button() == Qt::LeftButton)
    {
        emit signalLeftButtonClicked();
    }
    else if (e->button() == Qt::MidButton)
    {
        if (visibleWidth()  < d->zoomWidth ||
            visibleHeight() < d->zoomHeight)
        {
            m_movingInProgress = true;
            d->midButtonX      = e->x();
            d->midButtonY      = e->y();
            viewport()->repaint(false);
            viewport()->setCursor(Qt::SizeAllCursor);            
        }
        return;
    }
    
    viewport()->setMouseTracking(false);
}

void PreviewWidget::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->state() & Qt::MidButton)
    {
        if (m_movingInProgress)
        {
            scrollBy(d->midButtonX - e->x(),
                     d->midButtonY - e->y());
            emit signalContentsMovedEvent(false);
        }
    }
}
    
void PreviewWidget::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    m_movingInProgress = false;

    if (e->button() == Qt::MidButton)
    {
        emit signalContentsMovedEvent(true);
        viewport()->unsetCursor();
        viewport()->repaint(false);
    }

    if (e->button() == Qt::RightButton)
    {
        emit signalRightButtonClicked();
    }
}

void PreviewWidget::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->state() & Qt::ShiftButton)
    {
        if (e->delta() < 0)
            emit signalShowNextImage();
        else if (e->delta() > 0)
            emit signalShowPrevImage();
        return;
    }
    else if (e->state() & Qt::ControlButton)
    {
        // When zooming with the mouse-wheel, the image center is kept fixed.
        d->centerZoomPoint = e->pos();
        if (e->delta() < 0 && !minZoom())
            slotDecreaseZoom();
        else if (e->delta() > 0 && !maxZoom())
            slotIncreaseZoom();
        d->centerZoomPoint = QPoint();
        return;
    }

    QScrollView::contentsWheelEvent(e);
}

void PreviewWidget::zoomFactorChanged(double zoom)
{
    emit signalZoomFactorChanged(zoom);
}

}  // NameSpace Digikam
