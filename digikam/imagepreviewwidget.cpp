/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006-2007 Gilles Caulier
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

#include <qcache.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qtimer.h>
#include <qguardedptr.h>

// KDE include.

#include <kcursor.h>
#include <kprocess.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "fastscale.h"
#include "themeengine.h"
#include "albumsettings.h"
#include "imagepreviewwidget.h"
#include "imagepreviewwidget.moc"

namespace Digikam
{

class ImagePreviewWidgetPriv
{
public:

    ImagePreviewWidgetPriv() :
        tileSize(128), minZoom(0.1), maxZoom(10.0), zoomMultiplier(1.2) 
    {
        pressedMoving        = false;
        midButtonPressed     = false;
        midButtonX           = 0;
        midButtonY           = 0;
        autoZoom             = false;
        fullScreen           = false;
        zoom                 = 1.0;
        zoomWidth            = 0;
        zoomHeight           = 0;
        tileTmpPix           = new QPixmap(tileSize, tileSize);

        tileCache.setMaxCost((10*1024*1024)/(tileSize*tileSize*4));
        tileCache.setAutoDelete(true);
    }

    bool                 autoZoom;
    bool                 fullScreen;
    bool                 pressedMoving;
    bool                 midButtonPressed;

    const int            tileSize;
    int                  midButtonX;
    int                  midButtonY;
    int                  zoomWidth;
    int                  zoomHeight;
    
    double               zoom;
    const double         minZoom;
    const double         maxZoom;
    const double         zoomMultiplier;

    QRect                pixmapRect;
    
    QCache<QPixmap>      tileCache;

    QPixmap*             tileTmpPix;

    QColor               bgColor;

    QImage               preview;
};

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
                  : QScrollView(parent)
{
    d = new ImagePreviewWidgetPriv;
    d->bgColor.setRgb(0, 0, 0);
    
    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(false);

    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain); 
    setMargin(0); 
    setLineWidth(1); 

    // ------------------------------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

ImagePreviewWidget::~ImagePreviewWidget()
{
    delete d->tileTmpPix;
    delete d;
}

void ImagePreviewWidget::setImage(const QImage& image)
{   
    d->preview = image;

    updateAutoZoom();
    updateContentsSize();

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

void ImagePreviewWidget::slotThemeChanged()
{
    if (d->bgColor == ThemeEngine::instance()->baseColor())
        return;
    
    d->bgColor = ThemeEngine::instance()->baseColor();
    viewport()->update();
}

void ImagePreviewWidget::resetImage()
{
    d->tileCache.clear();
    viewport()->setUpdatesEnabled(false);
    d->preview.reset();
}

void ImagePreviewWidget::updateAutoZoom()
{
    d->zoom       = calcAutoZoomFactor();
    d->zoomWidth  = (int)(d->preview.width()  * d->zoom);
    d->zoomHeight = (int)(d->preview.height() * d->zoom);
}

double ImagePreviewWidget::calcAutoZoomFactor()
{
    if (d->preview.isNull()) return d->zoom;

    double srcWidth  = d->preview.width();
    double srcHeight = d->preview.height();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();

    return QMIN(dstWidth/srcWidth, dstHeight/srcHeight);
}

void ImagePreviewWidget::updateContentsSize()
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
    resizeContents(d->zoomWidth, d->zoomHeight);
    viewport()->setUpdatesEnabled(true);
}

void ImagePreviewWidget::resizeEvent(QResizeEvent* e)
{
    if (!e)
        return;

    QScrollView::resizeEvent(e);

    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();

    // No need to repaint. its called   
    // automatically after resize
}

void ImagePreviewWidget::viewportPaintEvent(QPaintEvent *e)
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

    if (!cr.isEmpty() && !d->preview.isNull())
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

                    sx = (int)floor(((double)i / d->zoom) / (d->tileSize / d->zoom)) * step;
                    sy = (int)floor(((double)j / d->zoom) / (d->tileSize / d->zoom)) * step;
                    sw = step;
                    sh = step;

                    // Fast smooth scale method from Antonio.
                    QImage img = FastScale::fastScaleQImage(d->preview.copy(sx, sy, sw, sh),
                                                            d->tileSize, d->tileSize);
                    bitBlt(pix, 0, 0, &img, 0, 0);
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

    QPainter painter(viewport());
    painter.setClipRegion(clipRegion);
    painter.fillRect(er, d->bgColor);
    painter.end();
}

void ImagePreviewWidget::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e || e->button() == Qt::RightButton)
        return;

    d->midButtonPressed = false;

    if (e->button() == Qt::LeftButton)
    {
        emit signalLeftButtonClicked();
    }
    else if (e->button() == Qt::MidButton)
    {
        if (visibleWidth()  < d->preview.width() ||
            visibleHeight() < d->preview.height())
        {
            viewport()->setCursor(Qt::SizeAllCursor);
            d->midButtonPressed = true;
            d->midButtonX       = e->x();
            d->midButtonY       = e->y();
        }
        return;
    }
    
    d->pressedMoving = true;

    viewport()->setMouseTracking(false);
}

void ImagePreviewWidget::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->state() == Qt::MidButton)
    {
        if (d->midButtonPressed)
        {
            scrollBy(d->midButtonX - e->x(),
                     d->midButtonY - e->y());
        }
    }
}
    
void ImagePreviewWidget::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    d->midButtonPressed = false;
    
    if (d->pressedMoving)
    {
        d->pressedMoving = false;
        viewport()->update();
    }

    if (e->button() != Qt::LeftButton)
    {
        viewport()->unsetCursor();
    }

    if (e->button() == Qt::RightButton)
    {
        emit signalRightButtonClicked();
    }
}

void ImagePreviewWidget::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->state() == Qt::ShiftButton)
    {
        if (e->delta() < 0)
            emit signalShowNextImage();
        else if (e->delta() > 0)
            emit signalShowPrevImage();
        return;
    }
    else if (e->state() == Qt::ControlButton)
    {
        if (e->delta() < 0)
            slotIncreaseZoom();
        else if (e->delta() > 0)
            slotDecreaseZoom();
        return;
    }

    QScrollView::contentsWheelEvent(e);
}

bool ImagePreviewWidget::maxZoom()
{
    return ((d->zoom * d->zoomMultiplier) >= d->maxZoom);
}

bool ImagePreviewWidget::minZoom()
{
    return ((d->zoom / d->zoomMultiplier) <= d->minZoom);
}

void ImagePreviewWidget::slotIncreaseZoom()
{
    if (maxZoom())
        return;

    setZoomFactor(d->zoom * d->zoomMultiplier);
}

void ImagePreviewWidget::slotDecreaseZoom()
{
    if (minZoom())
        return;

    setZoomFactor(d->zoom / d->zoomMultiplier);
}

void ImagePreviewWidget::setZoomFactor(double zoom)
{
    if (d->autoZoom)
        return;

    // Zoom using center of canvas and given zoom factor.

    double cpx = contentsX() + visibleWidth()  / 2.0; 
    double cpy = contentsY() + visibleHeight() / 2.0;

    cpx = ((cpx / d->zoom) / (d->tileSize / d->zoom)) * floor(d->tileSize / d->zoom);
    cpy = ((cpy / d->zoom) / (d->tileSize / d->zoom)) * floor(d->tileSize / d->zoom);

    d->zoom       = zoom;
    d->zoomWidth  = (int)(d->preview.width()  * d->zoom);
    d->zoomHeight = (int)(d->preview.height() * d->zoom);

    updateContentsSize();

    viewport()->setUpdatesEnabled(false);
    center((int)(((cpx * d->zoom) * (d->tileSize / d->zoom)) / floor(d->tileSize / d->zoom)), 
           (int)(((cpy * d->zoom) * (d->tileSize / d->zoom)) / floor(d->tileSize / d->zoom)));
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

bool ImagePreviewWidget::fitToWindow()
{
    return d->autoZoom;
}

void ImagePreviewWidget::toggleFitToWindow()
{
    d->autoZoom = !d->autoZoom;

    if (d->autoZoom)
        updateAutoZoom();
    else
        d->zoom = 1.0;

    updateContentsSize();
    viewport()->update();
}

}  // NameSpace Digikam
