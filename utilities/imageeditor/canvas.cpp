/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-01-09
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju, Gilles Caulier
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

#include <cstdio>
#include <cmath>
 
// Qt includes.
 
#include <qfile.h>
#include <qstring.h>
#include <qevent.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qimage.h>
#include <qregion.h>
#include <qtimer.h>
#include <qcache.h>
#include <qcolor.h>

// Local includes.

#include "imlibinterface.h"
#include "canvas.h"

using namespace Digikam;

class CanvasPrivate {

public:

    CanvasPrivate() :
        maxZoom(8.0),
        tileSize(128)
    {
        tileCache.setMaxCost((10*1024*1024)/(tileSize*tileSize*4));
        tileCache.setAutoDelete(true);
    }

    
    ImlibInterface    *im;
    QPixmap            qcheck;
    QColor             bgColor;
                     
    double             zoom;
    bool               autoZoom;
    QRect              pixmapRect;
    bool               fullScreen;

    QRect             *rubber;
    bool               pressedMoved;
    bool               pressedMoving;
    bool               ltActive;
    bool               rtActive;
    bool               lbActive;
    bool               rbActive;
    bool               midButtonPressed;
    int                midButtonX;
    int                midButtonY;

    QTimer            *paintTimer;

    const double       maxZoom;
    const int          tileSize;
    QCache<QPixmap>    tileCache;
    QPixmap*           tileTmpPix;
};

Canvas::Canvas(QWidget *parent)
      : QScrollView(parent)
{
    viewport()->setBackgroundMode(Qt::NoBackground);
    
    d = new CanvasPrivate;

    d->im = ImlibInterface::instance();
    d->zoom = 1.0;
    d->autoZoom = false;
    d->fullScreen = false;
    d->bgColor.setRgb( 0, 0, 0 );
    d->tileTmpPix = new QPixmap(d->tileSize, d->tileSize);

    d->rubber = 0;
    d->pressedMoved  = false;
    d->pressedMoving = false;
    d->ltActive      = false;
    d->rtActive      = false;
    d->lbActive      = false;
    d->rbActive      = false;
    d->midButtonPressed = false;
    d->midButtonX       = 0;
    d->midButtonY       = 0;

    d->paintTimer = new QTimer;
    
    d->qcheck.resize(16, 16);
    QPainter p(&d->qcheck);
    p.fillRect(0, 0, 8, 8, QColor(144,144,144));
    p.fillRect(8, 8, 8, 8, QColor(144,144,144));
    p.fillRect(0, 8, 8, 8, QColor(100,100,100));
    p.fillRect(8, 0, 8, 8, QColor(100,100,100));
    p.end();
    
    connect(d->im, SIGNAL(signalRequestUpdate()),
            SLOT(slotRequestUpdate()));
    connect(this, SIGNAL(signalSelected(bool)),
            SLOT(slotSelected()));
    connect(d->paintTimer, SIGNAL(timeout()),
            SLOT(slotPaintSmooth()));

    viewport()->setMouseTracking(false);
}

Canvas::~Canvas()
{
    d->paintTimer->stop();
    delete d->paintTimer;

    delete d->tileTmpPix;
    
    delete d->im;

    if (d->rubber)
        delete d->rubber;

    delete d;
}

void Canvas::load(const QString& filename)
{
    if (d->rubber) {
        delete d->rubber;
        d->rubber = 0;
        emit signalSelected(false);
    }

    viewport()->setUpdatesEnabled(false);

    d->tileCache.clear();
    d->im->load(filename);

    d->zoom = 1.0;
    d->im->zoom(d->zoom);
    
    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();

    viewport()->setUpdatesEnabled(true);
    viewport()->update();

    emit signalChanged(false);
    emit signalZoomChanged(d->zoom);
}

void Canvas::preload(const QString& filename)
{
    d->im->preload(filename);
}

int Canvas::save(const QString& filename, int JPEGcompression,
                 int PNGcompression, bool TIFFcompression)
{
    int result = d->im->save(filename, JPEGcompression, PNGcompression, 
                             TIFFcompression);
    if ( result == true ) emit signalChanged(false);
    return result;
}

int Canvas::saveAs(const QString& filename, int JPEGcompression, 
                   int PNGcompression, bool TIFFcompression, 
                   const QString& mimeType)
{
    int result = d->im->saveAs(filename, JPEGcompression, PNGcompression, 
                               TIFFcompression, mimeType);
    if ( result == true ) emit signalChanged(false);
    return result;
}

int Canvas::saveAsTmpFile(const QString& filename, int JPEGcompression,
                          int PNGcompression, bool TIFFcompression, 
                          const QString& mimeType)
{
    int result = d->im->saveAs(filename, JPEGcompression, PNGcompression, 
                               TIFFcompression, mimeType);
    return result;
}

int Canvas::imageWidth()
{
    return d->im->origWidth();  
}

int Canvas::imageHeight()
{
    return d->im->origHeight();
}

QRect Canvas::getSelectedArea()
{
    int x, y, w, h;
    
    d->im->getSelectedArea(x, y, w, h);
    return ( QRect(x, y, w, h) );
}

void Canvas::updateAutoZoom()
{
    double srcWidth, srcHeight, dstWidth, dstHeight;

    srcWidth  = d->im->origWidth();
    srcHeight = d->im->origHeight();
    dstWidth  = contentsRect().width();
    dstHeight = contentsRect().height();

    if (dstWidth > srcWidth &&
        dstHeight > srcHeight)
        d->zoom = 1.0;
    else
        d->zoom = QMIN(dstWidth/srcWidth,
                       dstHeight/srcHeight);

    d->im->zoom(d->zoom);
    
    emit signalZoomChanged(d->zoom);
}

void Canvas::updateContentsSize()
{
    viewport()->setUpdatesEnabled(false);

    d->paintTimer->stop();
    d->ltActive      = false;
    d->rtActive      = false;
    d->lbActive      = false;
    d->rbActive      = false;
    viewport()->unsetCursor();
    viewport()->setMouseTracking(false);
    
    if (d->rubber) {
        delete d->rubber;
        d->rubber = 0;
        d->pressedMoved = false;
        emit signalSelected(false);
    }

    int wZ = int(d->im->width());
    int hZ = int(d->im->height());
    
    if (visibleWidth() > wZ || visibleHeight() > hZ) {

        // Center the image
        int centerx = contentsRect().width()/2;
        int centery = contentsRect().height()/2;
        int xoffset = int(centerx - wZ/2);
        int yoffset = int(centery - hZ/2);
        xoffset = QMAX(xoffset, 0);
        yoffset = QMAX(yoffset, 0);

        d->pixmapRect = QRect(xoffset, yoffset,
                              wZ, hZ);
    }
    else {
        d->pixmapRect = QRect(0, 0, wZ, hZ);
    }

    d->tileCache.clear();    
    resizeContents(wZ, hZ);
    viewport()->setUpdatesEnabled(true);
}


void Canvas::resizeEvent(QResizeEvent* e)
{
    if (!e)
        return;

    QScrollView::resizeEvent(e);
    if (d->autoZoom) {
        updateAutoZoom();
    }

    updateContentsSize();

    // No need to repaint. its called
    // automatically after resize
}

void Canvas::viewportPaintEvent(QPaintEvent *e)
{
    QRect er(e->rect());
    er = QRect(QMAX(er.x()-1,0),
               QMAX(er.y()-1,0),
               QMIN(er.width()+2, contentsRect().width()),
               QMIN(er.height()+2,contentsRect().height()));
    
    paintViewport(er, d->zoom <= 1.0);
    if (d->zoom > 1.0)
        d->paintTimer->start(100, true);
}

void Canvas::paintViewport(const QRect& er, bool antialias)
{
    QRect cr(viewportToContents(er.topLeft()),
             viewportToContents(er.bottomRight()));

    QRegion clipRegion(er);
    
    cr = d->pixmapRect.intersect(cr);
    if (!cr.isEmpty())
    {
        clipRegion -= QRect(contentsToViewport(cr.topLeft()), cr.size());

        QRect pr = QRect(cr.x()-d->pixmapRect.x(),
                         cr.y()-d->pixmapRect.y(),
                         cr.width(), cr.height());

        int x1 = (int)floor((float)pr.x()/(float)d->tileSize)      * d->tileSize;
        int y1 = (int)floor((float)pr.y()/(float)d->tileSize)      * d->tileSize;
        int x2 = (int)ceilf((float)pr.right()/(float)d->tileSize)  * d->tileSize;
        int y2 = (int)ceilf((float)pr.bottom()/(float)d->tileSize) * d->tileSize;

        QPixmap pix(d->tileSize, d->tileSize);
        int sx, sy, sw, sh;

        for (int j=y1; j<y2; j+=d->tileSize)
        {
            for (int i=x1; i<x2; i+=d->tileSize)
            {
                QString key = QString("%1,%1")
                              .arg(i)
                              .arg(j);

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

                    if (d->im->hasAlpha())
                    {
                        QPainter p(pix);
                        p.drawTiledPixmap(0, 0, d->tileSize, d->tileSize,
                                          d->qcheck, 0, 0);
                        p.end();
                    }
                    else
                    {
                        pix->fill(d->bgColor);
                    }

                    sx = (int)floor(i/d->zoom);
                    sy = (int)floor(j/d->zoom);
                    sw = (int)floor(d->tileSize/d->zoom);
                    sh = (int)floor(d->tileSize/d->zoom);

                    if (d->rubber && d->pressedMoved)
                    {
                        QRect rr(d->rubber->normalize());
                        rr = QRect(d->rubber->normalize());
                        QRect  r(i,j,d->tileSize,d->tileSize);
                        d->im->paintOnDevice(pix, sx, sy, sw, sh,
                                             0, 0, d->tileSize, d->tileSize,
                                             rr.x() - i - d->pixmapRect.x(),
                                             rr.y() - j -d->pixmapRect.y(),
                                             rr.width(), rr.height(),
                                             antialias);

                        rr = QRect(d->rubber->normalize());
                        rr.moveBy(-i-d->pixmapRect.x(),
                                  -j-d->pixmapRect.y()); 
                        QPainter p(pix);
                        p.setPen(QPen(QColor(250,250,255),1));
                        p.drawRect(rr);
                        if (rr.width() >= 10 && rr.height() >= 10)
                        {
                            p.drawRect(rr.x(), rr.y(), 5, 5);
                            p.drawRect(rr.x(), rr.y()+rr.height()-5, 5, 5);
                            p.drawRect(rr.x()+rr.width()-5, rr.y()+rr.height()-5, 5, 5);
                            p.drawRect(rr.x()+rr.width()-5, rr.y(), 5, 5);
                        }
                        p.end();
                    }
                    else
                    {
                        d->im->paintOnDevice(pix, sx, sy, sw, sh,
                                             0, 0, d->tileSize, d->tileSize,
                                             antialias);
                    }
                }

                QRect  r(i,j,d->tileSize,d->tileSize);
                QRect  ir = pr.intersect(r);
                QPoint pt(contentsToViewport(QPoint(ir.x()+d->pixmapRect.x(),
                                                    ir.y()+d->pixmapRect.y())));

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

void Canvas::drawRubber()
{
    if (!d->rubber)
        return;

    QPainter p(viewport());
    p.setRasterOp(Qt::NotROP );
    p.setPen(QPen(color0, 1));
    p.setBrush(NoBrush);

    QRect r(d->rubber->normalize());
    r = QRect(contentsToViewport(QPoint(r.x(),r.y())),
              r.size());

    QPoint pnt(r.x(), r.y());

    style().drawPrimitive(QStyle::PE_FocusRect, &p,
                          QRect( pnt.x(), pnt.y(),
                                 r.width(), r.height() ),
                          colorGroup(), QStyle::Style_Default,
                          QStyleOption(colorGroup().base()));
    p.end();
}

void Canvas::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e || e->button() == Qt::RightButton)
        return;

    d->midButtonPressed = false;
    
    if (e->button() == Qt::LeftButton)
    {
        if (d->ltActive || d->rtActive ||
            d->lbActive || d->rbActive) {

            Q_ASSERT( d->rubber );
            if (!d->rubber)
                return;

            // Set diagonally opposite corner as anchor
        
            QRect r(d->rubber->normalize());

            if (d->ltActive) {
                d->rubber->setTopLeft(r.bottomRight());
                d->rubber->setBottomRight(r.topLeft());
            }
            else if (d->rtActive) {
                d->rubber->setTopLeft(r.bottomLeft());
                d->rubber->setBottomRight(r.topRight());
            }
            else if (d->lbActive) {
                d->rubber->setTopLeft(r.topRight());
                d->rubber->setBottomRight(r.bottomLeft());
            }
            else if (d->rbActive) {
                d->rubber->setTopLeft(r.topLeft());
                d->rubber->setBottomRight(r.bottomLeft());
            }
        
            viewport()->setMouseTracking(false);

            d->pressedMoving = true;
            return;
        }
    }
    else if (e->button() == Qt::MidButton)
    {
        if (visibleWidth()  < d->im->width() ||
            visibleHeight() < d->im->height())
        {
            viewport()->setCursor(Qt::SizeAllCursor);
            d->midButtonPressed = true;
            d->midButtonX       = e->x();
            d->midButtonY       = e->y();
        }
        return;
    }
    
    if (d->rubber) {
        delete d->rubber;
        d->rubber = 0;        
    }

    d->rubber = new QRect(e->x(), e->y(), 0, 0);

    if (d->pressedMoved)
    {
        d->tileCache.clear();
        viewport()->update();
    }
    
    d->pressedMoved  = false;
    d->pressedMoving = true;

    viewport()->setMouseTracking(false);
}

void Canvas::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e)
        return;

    if (e->state() == Qt::MidButton)
    {
        if (d->midButtonPressed)
        {
            scrollBy(d->midButtonX - e->x(),
                     d->midButtonY - e->y());
        }
    }
    else if (!viewport()->hasMouseTracking())
    {
        if (!d->rubber)
            return;
        
        if (e->state() != Qt::LeftButton &&
            !(d->ltActive || d->rtActive ||
              d->lbActive || d->rbActive))
            return;

        drawRubber();

        int r, b;
        r = (e->x() > d->pixmapRect.left()) ? e->x() : d->pixmapRect.left();
        r = (r < d->pixmapRect.right()) ? r : d->pixmapRect.right();
        b = (e->y() > d->pixmapRect.top()) ? e->y() : d->pixmapRect.top();
        b = (b < d->pixmapRect.bottom()) ? b : d->pixmapRect.bottom();

        d->rubber->setRight(r);
        d->rubber->setBottom(b);

        d->pressedMoved  = true;
        d->pressedMoving = true;

        drawRubber();
    }
    else {

        if (!d->rubber)
            return;
        
        QRect r(d->rubber->normalize());
        
        QRect lt(r.x()-5,r.y()-5,10,10);
        QRect rt(r.x()+r.width()-5,r.y()-5,10,10);
        QRect lb(r.x()-5,r.y()+r.height()-5,10,10);
        QRect rb(r.x()+r.width()-5,r.y()+r.height()-5,10,10);

        d->ltActive = false;
        d->rtActive = false;
        d->lbActive = false;
        d->rbActive = false;
        
        if (lt.contains(e->x(), e->y())) {
            viewport()->setCursor(Qt::SizeFDiagCursor);
            d->ltActive = true;
        }
        else if (rb.contains(e->x(), e->y())) {
            viewport()->setCursor(Qt::SizeFDiagCursor);
            d->rbActive = true;
        }
        else if (lb.contains(e->x(), e->y())) {
            viewport()->setCursor(Qt::SizeBDiagCursor);
            d->lbActive = true;
        }
        else if (rt.contains(e->x(), e->y())) {
            viewport()->setCursor(Qt::SizeBDiagCursor);
            d->rtActive = true;
        }
        else
            viewport()->unsetCursor();
    }
}
    
void Canvas::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e)
        return;

    d->midButtonPressed = false;
    
    if (d->pressedMoving) {
        d->pressedMoving = false;
        viewport()->update();
    }

    if (d->pressedMoved && d->rubber)
    {
        d->tileCache.clear();
        viewport()->setMouseTracking(true);
        emit signalSelected(true);
    }
    else {
        d->ltActive = false;
        d->rtActive = false;
        d->lbActive = false;
        d->rbActive = false;
        viewport()->setMouseTracking(false);
        emit signalSelected(false);
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


void Canvas::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->state() == Qt::ShiftButton) {

        if (e->delta() < 0)
            emit signalShowNextImage();
        else if (e->delta() > 0)
            emit signalShowPrevImage();
        return;
    }
    else if (e->state() == Qt::ControlButton) {

        if (e->delta() < 0)
            slotIncreaseZoom();
        else if (e->delta() > 0)
            slotDecreaseZoom();
        return;
    }

    QScrollView::contentsWheelEvent(e);
}

bool Canvas::maxZoom()
{
    return ((d->zoom + 1.0/16.0) >= d->maxZoom);
}

bool Canvas::minZoom()
{
    return ((d->zoom - 1.0/16.0) <= 0.1);
}

bool Canvas::exifRotated()
{
    return d->im->exifRotated();
}

void Canvas::slotIncreaseZoom()
{
    if (d->autoZoom || maxZoom())
        return;

    d->zoom = d->zoom + 1.0/16.0;

    d->im->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->update();

    emit signalZoomChanged(d->zoom);
}

void Canvas::slotDecreaseZoom()
{
    if (d->autoZoom || minZoom())
        return;

    d->zoom = d->zoom - 1.0/16.0;    

    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalZoomChanged(d->zoom);
}

void Canvas::slotSetAutoZoom(bool val)
{
    if (d->autoZoom == val)
        return;

    d->autoZoom = val;
    if (d->autoZoom)
        updateAutoZoom();
    else {
        d->zoom = 1.0;
        emit signalZoomChanged(d->zoom);
    }

    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();
}

void Canvas::slotToggleAutoZoom()
{
    slotSetAutoZoom(!d->autoZoom);
}

void Canvas::slotRotate90()
{
    d->im->rotate90();

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotRotate180()
{
    d->im->rotate180();

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotRotate270()
{
    d->im->rotate270();

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotFlipHoriz()
{
    d->im->flipHoriz();

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotFlipVert()
{
    d->im->flipVert();

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotCrop()
{
     if (!d->rubber) return;

     QRect r(d->rubber->normalize());
     if (!r.isValid()) return;

     r.moveBy(- d->pixmapRect.x(),
              - d->pixmapRect.y());

     double scale = 1.0/d->zoom;

     int x = (int)((double)r.x() * scale);
     int y = (int)((double)r.y() * scale);
     int w = (int)((double)r.width() * scale);
     int h = (int)((double)r.height() * scale);

     x = QMAX(x, 0);
     y = QMAX(y, 0);
     x = QMIN(imageWidth(),  x);
     y = QMIN(imageHeight(), y);

     w = QMAX(w, 0);
     h = QMAX(h, 0);
     w = QMIN(imageWidth(),  w);
     h = QMIN(imageHeight(), h);

     d->im->crop(x, y, w, h);

     if (d->autoZoom)
         updateAutoZoom();
     d->im->zoom(d->zoom);
  
     updateContentsSize();
     viewport()->update();
  

     emit signalChanged(true);
}

void Canvas::resizeImage(int w, int h)
{
    d->im->resize(w, h);

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::rotateImage(double angle)
{
    d->im->rotate(angle);

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
        return;
    
    d->bgColor = color;
    viewport()->update();
}

void Canvas::setExifOrient(bool exifOrient)
{
    d->im->setExifOrient(exifOrient);    
}

void Canvas::slotRestore()
{
    d->im->restore();
    
    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);
  
    updateContentsSize();
    viewport()->update();
  
    emit signalChanged(false);
}

void Canvas::slotSelected()
{
    int x, y, w, h;
    x = y = w = h = 0;
    
    if (d->rubber && d->pressedMoved) {

        QRect r(d->rubber->normalize());
        if (r.isValid()) {

            r.moveBy(- d->pixmapRect.x(),
                     - d->pixmapRect.y());

            double scale = 1.0/d->zoom;

            x = (int)((double)r.x() * scale);
            y = (int)((double)r.y() * scale);
            w = (int)((double)r.width() * scale);
            h = (int)((double)r.height() * scale);

            x = QMAX(x, 0);
            y = QMAX(y, 0);
            x = QMIN(imageWidth(),  x);
            y = QMIN(imageHeight(), y);

            w = QMAX(w, 0);
            h = QMAX(h, 0);
            w = QMIN(imageWidth(),  w);
            h = QMIN(imageHeight(), h);
        }
    }

    d->im->setSelectedArea(x, y, w, h);
}

void Canvas::slotRequestUpdate()
{
    updateContentsSize();
    viewport()->update();
    emit signalChanged(true);    
}

void Canvas::slotPaintSmooth()
{
    if (d->paintTimer->isActive())
        return;

    paintViewport(contentsRect(), true);
}

#include "canvas.moc"
