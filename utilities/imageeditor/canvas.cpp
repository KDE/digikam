/* ============================================================
 * File  : canvas.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-09
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qfile.h>
#include <qstring.h>
#include <qevent.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qstyle.h>
#include <qapp.h>
#include <qcursor.h>
#include <qimage.h>
#include <qregion.h>

#include "imlibinterface.h"
#include "canvas.h"

#include <math.h>
#include <cstdio>

using namespace Digikam;

const double maxZoom_ = 8.0;

class CanvasPrivate {

public:

    ImlibInterface *im;
    QPixmap*        bgPix;
    
    double zoom;
    bool autoZoom;
    QRect pixmapRect;
    QRect *rubber;
    bool  pressedMoved;
    bool fullScreen;
};

Canvas::Canvas(QWidget *parent)
    : QScrollView(parent)
{
    viewport()->setBackgroundMode(Qt::NoBackground);

    d = new CanvasPrivate;

    d->im = ImlibInterface::instance();
    d->zoom = 1.0;
    d->autoZoom = false;
    d->rubber = 0;
    d->pressedMoved = false;
    d->fullScreen = false;

    d->bgPix   = 0;

    connect(d->im, SIGNAL(signalRequestUpdate()),
            viewport(), SLOT(update()));
    connect(d->im, SIGNAL(signalRequestUpdate()),
            SLOT(slotChanged()));
    connect(this, SIGNAL(signalSelected(bool)),
            SLOT(slotSelected()));
}

Canvas::~Canvas()
{
    if (d->bgPix)
        delete d->bgPix;
    
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
    
    d->zoom = 1.0;
    
    d->im->load(filename);

    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();
    viewport()->update();

    emit signalChanged(false);
    emit signalZoomChanged(d->zoom);
}

int Canvas::imageWidth()
{
    return d->im->origWidth();  
}

int Canvas::imageHeight()
{
    return d->im->origHeight();
}

void Canvas::updateAutoZoom()
{
    double srcWidth, srcHeight, dstWidth, dstHeight;

    srcWidth  = d->im->origWidth();
    srcHeight = d->im->origHeight();
    dstWidth  = frameRect().width() - 20;
    dstHeight = frameRect().height() - 20;

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
    resizeContents(d->im->width(),
                   d->im->height());
    viewport()->setUpdatesEnabled(true);

    
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
        int centerx = frameRect().width()/2;
        int centery = frameRect().height()/2;
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

    viewport()->setUpdatesEnabled(false);
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

    if (!d->bgPix) 
        d->bgPix = new QPixmap(frameRect().width(),
                               frameRect().height());

    d->bgPix->resize(frameRect().width(), frameRect().height());
    
    updateContentsSize();

    // No need to repaint. its called
    // automatically after resize
}

void Canvas::viewportPaintEvent(QPaintEvent *e)
{
    if (!e)
        return;

    QRect er(e->rect());

    QRegion paintRegion(e->region());    

    QRect pr(contentsToViewport(QPoint(d->pixmapRect.x(),
                                       d->pixmapRect.y())),
             d->pixmapRect.size());

    if (pr.intersects(er)) {

        QRect ir(pr.intersect(er));
        paintRegion -= QRegion(ir);

        int x = ir.x() - pr.x();
        int y = ir.y() - pr.y();
        int w = ir.width();
        int h = ir.height();

        d->im->paint(d->bgPix, x, y, w, h, ir.x(), ir.y());

        if (d->rubber && d->pressedMoved) {

            QRect r(d->rubber->normalize());

            QPainter p(d->bgPix);
            QRect vr(0,0,d->bgPix->width(),d->bgPix->height());
            QRegion reg(vr);
            reg -= QRegion(QRect(contentsToViewport(QPoint(r.x(),
                                                           r.y())),
                                 r.size()));
            p.setClipRegion(reg);
            p.fillRect(vr,QBrush(Qt::white,Qt::Dense3Pattern)); 
            p.end(); 
        }

        bitBlt(viewport(), ir.x(), ir.y(),
               d->bgPix, ir.x(), ir.y(), w, h, Qt::CopyROP, true);
    }

    if (!paintRegion.isEmpty()) {
        QPainter p(viewport());
        p.setClipRegion(paintRegion);
        p.fillRect(er, colorGroup().base());
        p.end();
    }
}


void Canvas::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e || e->button() == Qt::RightButton)
        return;

    if (d->rubber) {
        delete d->rubber;
        d->rubber = 0;        
    }

    d->rubber = new QRect(e->x(), e->y(), 0, 0);

    d->pressedMoved = false;

    viewport()->update();
}

void Canvas::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e || !d->rubber) return;

    if (e->state() != Qt::LeftButton) return;

    int r, b;
    r = (e->x() > d->pixmapRect.left()) ? e->x() : d->pixmapRect.left();
    r = (r < d->pixmapRect.right()) ? r : d->pixmapRect.right();
    b = (e->y() > d->pixmapRect.top()) ? e->y() : d->pixmapRect.top();
    b = (b < d->pixmapRect.bottom()) ? b : d->pixmapRect.bottom();

    d->rubber->setRight(r);
    d->rubber->setBottom(b);

    d->pressedMoved = true;
    viewport()->update();
}

void Canvas::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->button() == Qt::RightButton)
        emit signalRightButtonClicked();
    else {
        if (d->pressedMoved && d->rubber)
            emit signalSelected(true);
        else
            emit signalSelected(false);
    }
}


void Canvas::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->state() == Qt::ControlButton ||
        e->state() == Qt::ShiftButton) {

        if (e->delta() < 0)
            emit signalShowNextImage();
        else if (e->delta() > 0)
            emit signalShowPrevImage();

        return;
        
    }

    QScrollView::contentsWheelEvent(e);
}
bool Canvas::maxZoom()
{
    return (d->zoom >= maxZoom_);
}

bool Canvas::minZoom()
{
    return (d->zoom <= 1.0/maxZoom_);
}

void Canvas::slotIncreaseZoom()
{
    if (d->autoZoom || maxZoom())
        return;

    if (d->zoom >= 1.0) {
        d->zoom = d->zoom + 0.25;
    }
    else {
        d->zoom = 1.0/( ceil(1/d->zoom)-1.0 );
    }

    d->im->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->update();

    emit signalZoomChanged(d->zoom);
}

void Canvas::slotDecreaseZoom()
{
    if (d->autoZoom || minZoom())
        return;

    if (d->zoom > 1.0) {
        d->zoom = d->zoom - 0.25;    
    } else {
        d->zoom=1/( floor(1/d->zoom)+1.0 );
    }

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

void Canvas::resize(int w, int h)
{
    d->im->resize(w, h);

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();

    emit signalChanged(true);
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

void Canvas::slotGammaPlus()
{
    d->im->changeGamma(1);
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotGammaMinus()
{
    d->im->changeGamma(-1);
    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotBrightnessPlus()
{
    d->im->changeBrightness(1);

    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotBrightnessMinus()
{
     d->im->changeBrightness(-1);
     viewport()->update();

     emit signalChanged(true);
}

void Canvas::slotContrastPlus()
{
    d->im->changeContrast(1);

    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotContrastMinus()
{
    d->im->changeContrast(-1);

    viewport()->update();

    emit signalChanged(true);
}

void Canvas::slotChanged()
{
    emit signalChanged(true);    
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

#include "canvas.moc"
