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

#include <qtooltip.h>
#include <qcache.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qtimer.h>
#include <qguardedptr.h>
#include <qtoolbutton.h>

// KDE include.

#include <kcursor.h>
#include <kprocess.h>
#include <klocale.h>
#include <kdatetbl.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "fastscale.h"
#include "albumsettings.h"
#include "paniconwidget.h"
#include "previewwidget.h"
#include "previewwidget.moc"

namespace Digikam
{

class PreviewWidgetPriv
{
public:

    PreviewWidgetPriv() :
        tileSize(128), minZoom(0.1), maxZoom(12.0), zoomMultiplier(1.2) 
    {
        pressedMoving    = false;
        midButtonPressed = false;
        midButtonX       = 0;
        midButtonY       = 0;
        autoZoom         = false;
        fullScreen       = false;
        zoom             = 1.0;
        zoomWidth        = 0;
        zoomHeight       = 0;
        panIconPopup     = 0;
        panIconWidget    = 0;
        cornerButton     = 0;
        tileTmpPix       = new QPixmap(tileSize, tileSize);

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

    QToolButton         *cornerButton;

    QRect                pixmapRect;
    
    QCache<QPixmap>      tileCache;

    QPixmap*             tileTmpPix;

    QColor               bgColor;

    QImage               preview;

    KPopupFrame         *panIconPopup;

    PanIconWidget       *panIconWidget;
};

PreviewWidget::PreviewWidget(QWidget *parent)
             : QScrollView(parent)
{
    d = new PreviewWidgetPriv;
    d->bgColor.setRgb(0, 0, 0);
    
    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(false);

    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain); 
    setMargin(0); 
    setLineWidth(1); 

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIconSet(SmallIcon("move"));
    d->cornerButton->hide();
    QToolTip::add(d->cornerButton, i18n("Pan the image to a region"));
    setCornerWidget(d->cornerButton);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));
}

PreviewWidget::~PreviewWidget()
{
    delete d->tileTmpPix;
    delete d;
}

void PreviewWidget::setImage(const QImage& image)
{   
    d->preview = image;

    updateAutoZoom();
    updateContentsSize();

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

void PreviewWidget::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
        return;
    
    d->bgColor = color;
    viewport()->update();
}

void PreviewWidget::resetImage()
{
    d->tileCache.clear();
    viewport()->setUpdatesEnabled(false);
    d->preview.reset();
}

void PreviewWidget::updateAutoZoom()
{
    d->zoom       = calcAutoZoomFactor();
    d->zoomWidth  = (int)(d->preview.width()  * d->zoom);
    d->zoomHeight = (int)(d->preview.height() * d->zoom);

    emit signalZoomFactorChanged(d->zoom);
}

double PreviewWidget::calcAutoZoomFactor()
{
    if (d->preview.isNull()) return d->zoom;

    double srcWidth  = d->preview.width();
    double srcHeight = d->preview.height();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();

    return QMIN(dstWidth/srcWidth, dstHeight/srcHeight);
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
    resizeContents(d->zoomWidth, d->zoomHeight);
    viewport()->setUpdatesEnabled(true);
}

void PreviewWidget::resizeEvent(QResizeEvent* e)
{
    if (!e)
        return;

    QScrollView::resizeEvent(e);

    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();

    // No need to repaint. its called   
    // automatically after resize

    // To be sure than corner widget used to pan image will be hide/show 
    // accordinly with resize event.
    slotZoomChanged(d->zoom);
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

                    sx = (int)floor((double)i  / d->tileSize ) * step;
                    sy = (int)floor((double)j  / d->tileSize ) * step;
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

void PreviewWidget::contentsMousePressEvent(QMouseEvent *e)
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

void PreviewWidget::contentsMouseMoveEvent(QMouseEvent *e)
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
    
void PreviewWidget::contentsMouseReleaseEvent(QMouseEvent *e)
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

void PreviewWidget::contentsWheelEvent(QWheelEvent *e)
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

double PreviewWidget::zoomMax()
{
    return d->maxZoom;
}

double PreviewWidget::zoomMin()
{
    return d->minZoom;
}

bool PreviewWidget::maxZoom()
{
    return (d->zoom >= d->maxZoom);
}

bool PreviewWidget::minZoom()
{
    return (d->zoom <= d->minZoom);
}

void PreviewWidget::slotIncreaseZoom()
{
    double zoom = d->zoom * d->zoomMultiplier;
    setZoomFactor(zoom > zoomMax() ? zoomMax() : zoom);
}

void PreviewWidget::slotDecreaseZoom()
{
    double zoom = d->zoom / d->zoomMultiplier;
    setZoomFactor(zoom < zoomMin() ? zoomMin() : zoom);
}

void PreviewWidget::setZoomFactor(double zoom)
{
    if (d->autoZoom || zoom == d->zoom)
        return;

    // Zoom using center of canvas and given zoom factor.

    double cpx = contentsX() + visibleWidth()  / 2.0; 
    double cpy = contentsY() + visibleHeight() / 2.0;

    cpx = ( cpx / d->tileSize ) * floor(d->tileSize / d->zoom);
    cpy = ( cpy / d->tileSize ) * floor(d->tileSize / d->zoom);

    // To limit precision of zoom value and reduce error with check of max/min zoom. 
    d->zoom       = floor(zoom * 10000.0) / 10000.0;
    d->zoomWidth  = (int)(d->preview.width()  * d->zoom);
    d->zoomHeight = (int)(d->preview.height() * d->zoom);

    updateContentsSize();

    viewport()->setUpdatesEnabled(false);
    center((int)((cpx * d->tileSize ) / floor(d->tileSize / d->zoom)), 
           (int)((cpy * d->tileSize ) / floor(d->tileSize / d->zoom)));
    viewport()->setUpdatesEnabled(true);
    viewport()->update();

    emit signalZoomFactorChanged(d->zoom);
}

double PreviewWidget::zoomFactor()
{
    return d->zoom; 
}

bool PreviewWidget::fitToWindow()
{
    return d->autoZoom;
}

void PreviewWidget::toggleFitToWindow()
{
    d->autoZoom = !d->autoZoom;

    if (d->autoZoom)
        updateAutoZoom();
    else
    {
        d->zoom = 1.0;
        emit signalZoomFactorChanged(d->zoom);
    }

    updateContentsSize();
    viewport()->update();
}

void PreviewWidget::slotCornerButtonPressed()
{    
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget *pan = new PanIconWidget(d->panIconPopup);
    pan->setImage(180, 120, d->preview); 
    d->panIconPopup->setMainWidget(pan);

    QRect r((int)(contentsX()    / d->zoom), (int)(contentsY()     / d->zoom),
            (int)(visibleWidth() / d->zoom), (int)(visibleHeight() / d->zoom));
    pan->setRegionSelection(r);
    pan->setMouseFocus();

    connect(pan, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect, bool)));
    
    connect(pan, SIGNAL(signalHiden()),
            this, SLOT(slotPanIconHiden()));
    
    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(), 
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void PreviewWidget::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void PreviewWidget::slotPanIconSelectionMoved(QRect r, bool b)
{
    setContentsPos((int)(r.x()*d->zoom), (int)(r.y()*d->zoom));

    if (b)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
        slotPanIconHiden();
    }
}

void PreviewWidget::slotZoomChanged(double zoom)
{
    if (zoom > calcAutoZoomFactor())
        d->cornerButton->show();
    else
        d->cornerButton->hide();        
}

}  // NameSpace Digikam
