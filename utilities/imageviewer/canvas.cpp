//////////////////////////////////////////////////////////////////////////////
//
//    CANVAS.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt lib includes. 
 
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
#include <qapplication.h>
#include <qcursor.h>

// C ansi includes.

extern "C" 
{
#include <math.h>
}

// Local includes.

#include "canvas.h"
#include "imlibinterface.h"

const double maxZoom_ = 8.0;

class CanvasPrivate 
{
public:

    double zoom;
    bool autoZoom;
    QRect pixmapRect;
    QRect *rubber;
    bool  pressedMoved;
    QPixmap buffer;
    bool fullScreen;
};


/////////////////////////////////// CONSTRUCTOR /////////////////////////////////////////////

Canvas::Canvas(QWidget *parent)
      : QScrollView(parent)
{
    viewport()->setBackgroundMode(Qt::NoBackground);

    iface = new ImlibInterface(viewport());
    d = new CanvasPrivate;
    d->zoom         = 1.0;
    d->autoZoom     = false;
    d->rubber       = 0;
    d->pressedMoved = false;
    d->fullScreen   = false;
    counter         = 0;
}


///////////////////////////////// DESTRUCTOR ////////////////////////////////////////////////

Canvas::~Canvas()
{
    if (d->rubber)
        delete d->rubber;
    
    delete d;
    delete iface;
}


////////////////////////////////// FONCTIONS ////////////////////////////////////////////////

void Canvas::load(const QString& filename)
{
    if (d->rubber) 
        {
        delete d->rubber;
        d->rubber = 0;
        emit signalCropSelected(false);
        }
    
    emit signalChanged(false);

    qApp->setOverrideCursor(Qt::WaitCursor);
 
    iface->load(filename);

    if (d->autoZoom)
        updateAutoZoom();
    else
        setZoom(1.0);

    updateContentsSize();
    viewport()->repaint();

    qApp->restoreOverrideCursor();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::preload(const QString& filename)
{
    iface->preload(filename);    
}


/////////////////////////////////////////////////////////////////////////////////////////////

int Canvas::save(const QString& filename)
{
    int result = iface->save(filename);
    emit signalChanged(false);
    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////

int Canvas::saveAs(const QString& filename)
{
    int result = iface->saveAs(filename);
    emit signalChanged(false);
    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////

bool Canvas::autoZoomOn()
{
    return d->autoZoom;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::setZoom(double zoom)
{
    d->zoom = zoom;
    iface->zoom(zoom);
    emit signalZoomChanged(d->zoom);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::updateAutoZoom()
{
    double srcWidth, srcHeight, dstWidth, dstHeight;

    srcWidth  = iface->origWidth();
    srcHeight = iface->origHeight();
    dstWidth  = frameRect().width() - 20;
    dstHeight = frameRect().height() - 20;

    double zoom;
    
    if (dstWidth > srcWidth &&
        dstHeight > srcHeight)
        zoom = 1.0;
    else
        zoom = QMIN(dstWidth/srcWidth,
                       dstHeight/srcHeight);

    setZoom(zoom);

}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::updateContentsSize()
{
    if (d->rubber) {
        delete d->rubber;
        d->rubber = 0;
        emit signalCropSelected(false);
    }
    
    if (d->autoZoom) 
        {
        // Center the image
        int wZ = int(iface->width());
        int hZ = int(iface->height());

        int centerx = frameRect().width()/2;
        int centery = frameRect().height()/2;
        int xoffset = int(centerx - wZ/2);
        int yoffset = int(centery - hZ/2);
        xoffset = QMAX(xoffset, 10);
        yoffset = QMAX(yoffset, 10);

        d->pixmapRect = QRect(xoffset, yoffset,
                              wZ, hZ);

        viewport()->setUpdatesEnabled(false);
        resizeContents(wZ, hZ);
        viewport()->setUpdatesEnabled(true);
        }
    else 
        {
        int wZ = int(iface->width());
        int hZ = int(iface->height());
        
        if (visibleWidth() > wZ || visibleHeight() > hZ) {

            // Center the image
            int centerx = frameRect().width()/2;
            int centery = frameRect().height()/2;
            int xoffset = int(centerx - wZ/2);
            int yoffset = int(centery - hZ/2);
            xoffset = QMAX(xoffset, 10);
            yoffset = QMAX(yoffset, 10);
            d->pixmapRect = QRect(xoffset, yoffset,
                                  wZ, hZ);

            viewport()->setUpdatesEnabled(false);
            resizeContents(wZ, hZ);
            viewport()->setUpdatesEnabled(true);
        }
        else {
            viewport()->setUpdatesEnabled(false);
            resizeContents(wZ, hZ);
            viewport()->setUpdatesEnabled(true);
            d->pixmapRect = QRect(0, 0, wZ, hZ);
        }
    }
    
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::resizeEvent(QResizeEvent* e)
{
    if (!e) return;
    QScrollView::resizeEvent(e);
    if (d->autoZoom) {
        updateAutoZoom();
    }

    updateContentsSize();    
    // No need to repaint. its called automatically after resize
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::viewportPaintEvent(QPaintEvent *e)
{
    if (!e) return;

    QRect er(e->rect());

    QRegion paintRegion(e->region());    

    QRect pr(contentsToViewport(QPoint(d->pixmapRect.x(),
                                       d->pixmapRect.y())),
                     d->pixmapRect.size());

    if (pr.intersects(er)) {

        QRect ir(pr.intersect(er));

        int x = ir.x() - pr.x();
        int y = ir.y() - pr.y();
        int w = ir.width();
        int h = ir.height();

        iface->paint(x, y, w, h, ir.x(), ir.y());
        paintRegion -= QRegion(ir);
        
    }

        
    QPainter p(viewport());
    if (!paintRegion.isEmpty()) {
        p.setClipRegion(paintRegion);
        p.fillRect(er,Qt::black);
        p.setClipping(false);
        p.setPen(QPen(Qt::white, 2));
        p.drawRect(pr);
    }

    if (d->rubber) {
        p.setClipping(false);
        p.setRasterOp( NotROP );
        p.setPen(QPen(color0, 2));
        p.setBrush( NoBrush );
        drawRubber(&p);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->button() == Qt::RightButton)
        return;
    
    if (d->rubber) {

        if (d->pressedMoved) {
            QPainter p;
            p.begin( viewport() );
            p.setRasterOp( NotROP );
            p.setPen(QPen(color0, 2));
            p.setBrush( NoBrush );
            
            drawRubber( &p );
            p.end();
        }

        delete d->rubber;
        d->rubber = 0;        
    }

    d->rubber = new QRect(e->x(), e->y(), 0, 0);

    QPainter p;
    p.begin(viewport());
    p.setRasterOp(Qt::NotROP );
    p.setPen(QPen(color0, 2));
    p.setBrush(NoBrush);
    drawRubber(&p);
    p.end();

    d->pressedMoved = false;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e || !d->rubber) return;

    if (e->state() != Qt::LeftButton) return;
    
    QRect oldRubber = QRect(*d->rubber);

    int r, b;
    r = (e->x() > d->pixmapRect.left()) ? e->x() : d->pixmapRect.left();
    r = (r < d->pixmapRect.right()) ? r : d->pixmapRect.right();
    b = (e->y() > d->pixmapRect.top()) ? e->y() : d->pixmapRect.top();
    b = (b < d->pixmapRect.bottom()) ? b : d->pixmapRect.bottom();

    d->rubber->setRight(r);
    d->rubber->setBottom(b);

    QRect nr = *d->rubber;
    *d->rubber = oldRubber;

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 2 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    p.end();

    *d->rubber = nr;

    p.begin(viewport());
    p.setRasterOp(NotROP);
    p.setPen(QPen(color0, 2));
    p.setBrush(NoBrush);
    drawRubber(&p);
    p.end();

    d->pressedMoved = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    if (d->pressedMoved && d->rubber)
        emit signalCropSelected(true);
    else
        emit signalCropSelected(false);
    
    // if the user pressed but not moved the mouse
    if (!d->pressedMoved && d->rubber &&
        e->button() != Qt::RightButton) {

        QPainter p;
        p.begin(viewport());
        p.setRasterOp(NotROP);
        p.setPen(QPen(color0, 2));
        p.setBrush(NoBrush);
        drawRubber(&p);
        p.end();

        delete d->rubber;
        d->rubber = 0;
    }

    if (e->button() == Qt::RightButton)
        emit signalRightButtonClicked();
}


/////////////////////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::drawRubber(QPainter *p)
{
    if ( !p || !d->rubber )
        return;
    
    QRect r(d->rubber->normalize());

    r = QRect(contentsToViewport(QPoint(r.x(),r.y())),
              r.size());

    QPoint pnt(r.x(), r.y());

    style().drawPrimitive(QStyle::PE_FocusRect, p,
                          QRect( pnt.x(), pnt.y(),
                                 r.width(), r.height() ),
                          colorGroup(), QStyle::Style_Default,
                          QStyleOption(colorGroup().base()));
}


/////////////////////////////////////////////////////////////////////////////////////////////

bool Canvas::maxZoom()
{
    return (d->zoom >= maxZoom_);
}


/////////////////////////////////////////////////////////////////////////////////////////////

bool Canvas::minZoom()
{
    return (d->zoom <= 1.0/maxZoom_);
}


/////////////////////////////////// SLOTS ///////////////////////////////////////////////////

void Canvas::slotIncreaseZoom()
{
    if (d->autoZoom || maxZoom())
        return;

    double zoom;

    if (d->zoom >= 1.0) {
        zoom = d->zoom + 0.25;
    }
    else {
        zoom = 1.0/( ceil(1/d->zoom)-1.0 );
    }

    setZoom(zoom);
    
    updateContentsSize();
    viewport()->repaint();

    emit signalZoomChanged(d->zoom);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotDecreaseZoom()
{
    if (d->autoZoom || minZoom())
        return;

    double zoom;
    
    if (d->zoom > 1.0) {
        zoom = d->zoom - 0.25;    
    } else {
        zoom=1/( floor(1/d->zoom)+1.0 );
    }

    setZoom(zoom); 

    updateContentsSize();
    viewport()->repaint();

    emit signalZoomChanged(d->zoom);

}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotSetZoom1()
{
    if (d->autoZoom || minZoom())
        return;

    setZoom(1.0); 

    updateContentsSize();
    viewport()->repaint();

    emit signalZoomChanged(d->zoom);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotSetAutoZoom(bool val)
{
    if (d->autoZoom == val)
        return;

    d->autoZoom = val;
    if (d->autoZoom)
        updateAutoZoom();
    
    updateContentsSize();
    viewport()->repaint();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotToggleAutoZoom()
{
    slotSetAutoZoom(!d->autoZoom);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotFlipHorizontal()
{
    iface->flipHorizontal();

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotFlipVertical()
{
    iface->flipVertical();

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotRotate90()
{
    iface->rotate90();

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);

    updateContentsSize();
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotRotate180()
{
    iface->rotate180();

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotRotate270()
{
    iface->rotate270();

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);

    updateContentsSize();
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

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

//     x = QMAX(x, 0);
//     y = QMAX(y, 0);
//     x = QMIN(iface->width(),  x);
//     y = QMIN(iface->height(), y);

//     w = QMAX(w, 0);
//     h = QMAX(h, 0);
//     w = QMIN(iface->width(),  w);
//     h = QMIN(iface->height(), h);

    iface->crop(x, y, w, h);

    if (d->autoZoom)
        updateAutoZoom();
    iface->zoom(d->zoom);
    
    updateContentsSize();
    viewport()->repaint();
    
    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotGammaPlus()
{
    iface->changeGamma(6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotGammaMinus()
{
    iface->changeGamma(-6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotBrightnessPlus()
{
    iface->changeBrightness(6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotBrightnessMinus()
{
    iface->changeBrightness(-6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotContrastPlus()
{
    iface->changeContrast(6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotContrastMinus()
{
    iface->changeContrast(-6);
    viewport()->repaint();

    emit signalChanged(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Canvas::slotRestore()
{
    iface->restore();
    if (d->autoZoom)
        updateAutoZoom();
    else
        setZoom(d->zoom);
    updateContentsSize();
    viewport()->repaint();
    
    emit signalChanged(false);
}

#include "canvas.moc"
