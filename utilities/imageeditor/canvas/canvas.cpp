/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2003-01-09
 * Description : image editor canvas management class
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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
#include <qdragobject.h> 
#include <qclipboard.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "imagehistogram.h"
#include "dimginterface.h"
#include "iccsettingscontainer.h"
#include "exposurecontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "canvas.h"
#include "canvas.moc"

namespace Digikam
{

class CanvasPrivate
{

public:

    CanvasPrivate() :
        tileSize(128), maxZoom(8.0)
    {
        tileCache.setMaxCost((10*1024*1024)/(tileSize*tileSize*4));
        tileCache.setAutoDelete(true);

        parent          = 0;
        im              = 0;
        rubber          = 0;
        paintTimer      = 0;
    }

    bool               autoZoom;
    bool               fullScreen;
    bool               pressedMoved;
    bool               pressedMoving;
    bool               ltActive;
    bool               rtActive;
    bool               lbActive;
    bool               rbActive;
    bool               midButtonPressed;

    const int          tileSize;
    int                midButtonX;
    int                midButtonY;
    
    double             zoom;
    const double       maxZoom;

    QRect             *rubber;
    QRect              pixmapRect;
    
    QTimer            *paintTimer;

    QCache<QPixmap>    tileCache;

    QPixmap*           tileTmpPix;
    QPixmap            qcheck;

    QColor             bgColor;

    QWidget           *parent;
    
    DImgInterface     *im;

};

Canvas::Canvas(QWidget *parent)
      : QScrollView(parent)
{
    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(false);
    setFrameStyle( QFrame::NoFrame );

    d = new CanvasPrivate;

    d->im         = DImgInterface::instance();
    d->parent     = parent;
    d->zoom       = 1.0;
    d->autoZoom   = false;
    d->fullScreen = false;
    d->tileTmpPix = new QPixmap(d->tileSize, d->tileSize);
    d->bgColor.setRgb( 0, 0, 0 );

    d->rubber           = 0;
    d->pressedMoved     = false;
    d->pressedMoving    = false;
    d->ltActive         = false;
    d->rtActive         = false;
    d->lbActive         = false;
    d->rbActive         = false;
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

    // ------------------------------------------------------------
    
    connect(d->im, SIGNAL(signalColorManagementTool()),
            this, SIGNAL(signalColorManagementTool()));
            
    connect(d->im, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->im, SIGNAL(signalUndoStateChanged(bool, bool, bool)),
            this, SIGNAL(signalUndoStateChanged(bool, bool, bool)));

    connect(d->im, SIGNAL(signalImageLoaded(const QString&, bool)),
            this, SLOT(slotImageLoaded(const QString&, bool)));

    connect(d->im, SIGNAL(signalImageSaved(const QString&, bool)),
            this, SLOT(slotImageSaved(const QString&, bool)));

    connect(d->im, SIGNAL(signalLoadingProgress(const QString&, float)),
            this, SIGNAL(signalLoadingProgress(const QString&, float)));

    connect(d->im, SIGNAL(signalSavingProgress(const QString&, float)),
            this, SIGNAL(signalSavingProgress(const QString&, float)));
            
    connect(this, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected()));
            
    connect(d->paintTimer, SIGNAL(timeout()),
            this, SLOT(slotPaintSmooth()));
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

void Canvas::resetImage()
{
    reset();
    viewport()->setUpdatesEnabled(false);
    d->im->resetImage();
}

void Canvas::reset()
{
    if (d->rubber)
    {
        delete d->rubber;
        d->rubber = 0;
        if (d->im->imageValid())
            emit signalSelected(false);
    }

    d->tileCache.clear();
}

void Canvas::load(const QString& filename, IOFileSettingsContainer *IOFileSettings)
{
    reset();

    viewport()->setUpdatesEnabled(false);

    d->im->load( filename, IOFileSettings, d->parent );
    emit signalLoadingStarted(filename);
}

void Canvas::slotImageLoaded(const QString& filePath, bool success)
{
    d->zoom = 1.0;
    d->im->zoom(d->zoom);
    
    if (d->autoZoom)
        updateAutoZoom();

    updateContentsSize();

    viewport()->setUpdatesEnabled(true);
    viewport()->update();

    emit signalZoomChanged(d->zoom);

    emit signalLoadingFinished(filePath, success);
}

void Canvas::preload(const QString& /*filename*/)
{
//    d->im->preload(filename);
}

/*
These code paths are unused and untested
void Canvas::save(const QString& filename, IOFileSettingsContainer *IOFileSettings)
{
    d->im->save(filename, IOFileSettings);
    emit signalSavingStarted(filename);
}

void Canvas::saveAs(const QString& filename,IOFileSettingsContainer *IOFileSettings,
                    const QString& mimeType)
{
    d->im->saveAs(filename, IOFileSettings, mimeType);
    emit signalSavingStarted(filename);
}
*/

void Canvas::saveAs(const QString& filename, IOFileSettingsContainer *IOFileSettings,
                    bool setExifOrientationTag, const QString& mimeType)
{
    d->im->saveAs(filename, IOFileSettings, setExifOrientationTag, mimeType);
    emit signalSavingStarted(filename);
}

void Canvas::slotImageSaved(const QString& filePath, bool success)
{
    emit signalSavingFinished(filePath, success);
}

void Canvas::switchToLastSaved(const QString& newFilename)
{
    d->im->switchToLastSaved(newFilename);
}

void Canvas::abortSaving()
{
    d->im->abortSaving();
}

void Canvas::setModified()
{
    d->im->setModified();
}

void Canvas::readMetadataFromFile(const QString &file)
{
    d->im->readMetadataFromFile(file);
}

void Canvas::clearUndoHistory()
{
    d->im->clearUndoManager();
}

void Canvas::setUndoHistoryOrigin()
{
    d->im->setUndoManagerOrigin();
}

void Canvas::updateUndoState()
{
    d->im->updateUndoState();
}

DImg Canvas::currentImage()
{
    return DImg(*d->im->getImg());
}

QString Canvas::currentImageFileFormat()
{
    return d->im->getImageFormat();
}

int Canvas::imageWidth()
{
    return d->im->origWidth();  
}

int Canvas::imageHeight()
{
    return d->im->origHeight();
}

bool Canvas::isReadOnly()
{
    return d->im->isReadOnly();
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
    d->ltActive = false;
    d->rtActive = false;
    d->lbActive = false;
    d->rbActive = false;
    viewport()->unsetCursor();
    viewport()->setMouseTracking(false);

    if (d->rubber)
    {
        delete d->rubber;
        d->rubber = 0;
        d->pressedMoved = false;
        if (d->im->imageValid())
            emit signalSelected(false);
    }

    int wZ = int(d->im->width());
    int hZ = int(d->im->height());
    
    if (visibleWidth() > wZ || visibleHeight() > hZ)
    {
        // Center the image
        int centerx = contentsRect().width()/2;
        int centery = contentsRect().height()/2;
        int xoffset = int(centerx - wZ/2);
        int yoffset = int(centery - hZ/2);
        xoffset = QMAX(xoffset, 0);
        yoffset = QMAX(yoffset, 0);

        d->pixmapRect = QRect(xoffset, yoffset, wZ, hZ);
    }
    else
    {
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
    if (d->autoZoom)
    {
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
    QRect o_cr(viewportToContents(er.topLeft()),
               viewportToContents(er.bottomRight()));
    QRect cr = o_cr;

    QRegion clipRegion(er);
    cr = d->pixmapRect.intersect(cr);

    if (!cr.isEmpty() && d->im->imageValid())
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
    if (!d->rubber || !d->im->imageValid())
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
            d->lbActive || d->rbActive)
        {
            Q_ASSERT( d->rubber );
            if (!d->rubber)
                return;

            // Set diagonally opposite corner as anchor
        
            QRect r(d->rubber->normalize());

            if (d->ltActive)
            {
                d->rubber->setTopLeft(r.bottomRight());
                d->rubber->setBottomRight(r.topLeft());
            }
            else if (d->rtActive)
            {
                d->rubber->setTopLeft(r.bottomLeft());
                d->rubber->setBottomRight(r.topRight());
            }
            else if (d->lbActive)
            {
                d->rubber->setTopLeft(r.topRight());
                d->rubber->setBottomRight(r.bottomLeft());
            }
            else if (d->rbActive)
            {
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
    
    if (d->rubber)
    {
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
    else
    {
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
        
        if (lt.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeFDiagCursor);
            d->ltActive = true;
        }
        else if (rb.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeFDiagCursor);
            d->rbActive = true;
        }
        else if (lb.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeBDiagCursor);
            d->lbActive = true;
        }
        else if (rt.contains(e->x(), e->y()))
        {
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
    
    if (d->pressedMoving)
    {
        d->pressedMoving = false;
        viewport()->update();
    }

    if (d->pressedMoved && d->rubber)
    {
        d->tileCache.clear();
        viewport()->setMouseTracking(true);
        if (d->im->imageValid())
            emit signalSelected(true);
    }
    else
    {
        d->ltActive = false;
        d->rtActive = false;
        d->lbActive = false;
        d->rbActive = false;
        viewport()->setMouseTracking(false);
        if (d->im->imageValid())
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
    else
    {
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
}

void Canvas::slotRotate180()
{
    d->im->rotate180();
}

void Canvas::slotRotate270()
{
    d->im->rotate270();
}

void Canvas::slotFlipHoriz()
{
    d->im->flipHoriz();
}

void Canvas::slotFlipVert()
{
    d->im->flipVert();
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
}

void Canvas::resizeImage(int w, int h)
{
    d->im->resize(w, h);
}

void Canvas::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
        return;
    
    d->bgColor = color;
    viewport()->update();
}

void Canvas::setICCSettings(ICCSettingsContainer *cmSettings)
{
    d->im->setICCSettings(cmSettings);
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::setExposureSettings(ExposureSettingsContainer *expoSettings)
{
    d->im->setExposureSettings(expoSettings);
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::setExifOrient(bool exifOrient)
{
    d->im->setExifOrient(exifOrient);
    viewport()->update();
}

void Canvas::increaseGamma()
{
    d->im->changeGamma(1);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::decreaseGamma()
{
    d->im->changeGamma(-1);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::increaseBrightness()
{
    d->im->changeBrightness(1);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::decreaseBrightness()
{
    d->im->changeBrightness(-1);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::increaseContrast()
{
    d->im->changeContrast(5);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::decreaseContrast()
{
    d->im->changeContrast(-5);    
    d->tileCache.clear();    
    viewport()->update();
}

void Canvas::slotRestore()
{
    d->im->restore();
}

void Canvas::slotUndo(int steps)
{
    while(steps > 0)
    {
        d->im->undo();
        --steps;
    }
}

void Canvas::getUndoHistory(QStringList &titles)
{
    d->im->getUndoHistory(titles);
}

void Canvas::getRedoHistory(QStringList &titles)
{
    d->im->getRedoHistory(titles);
}

void Canvas::slotRedo(int steps)
{
    while(steps > 0)
    {
        d->im->redo();
        --steps;
    }
}

void Canvas::slotCopy()
{
    int x, y, w, h;
    d->im->getSelectedArea(x, y, w, h);

    if (!w && !h )  // No current selection.
        return;

    QApplication::setOverrideCursor (Qt::waitCursor);
    uchar* data = d->im->getImageSelection();
    DImg selDImg = DImg(w, h, d->im->sixteenBit(), d->im->hasAlpha(), data);
    delete [] data;

    QImage selImg = selDImg.copyQImage();
    QApplication::clipboard()->setData(new QImageDrag(selImg), QClipboard::Clipboard);
    QApplication::restoreOverrideCursor ();
}

void Canvas::slotSelected()
{
    int x, y, w, h;
    x = y = w = h = 0;
    
    if (d->rubber && d->pressedMoved) 
    {
        QRect r(d->rubber->normalize());
        
        if (r.isValid()) 
        {
            r.moveBy(- d->pixmapRect.x(), - d->pixmapRect.y());

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

            // Avoid empty selection by rubberband - at least mark one pixel
            // At high zoom factors, the rubberband may operate at subpixel level!
            if (w == 0)
                w = 1;
            if (h == 0)
                h = 1;
        }
    }

    d->im->setSelectedArea(x, y, w, h);
}

void Canvas::slotModified()
{
    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();
    emit signalChanged();
}

void Canvas::slotPaintSmooth()
{
    if (d->paintTimer->isActive())
        return;

    paintViewport(contentsRect(), true);
}

}  // namespace Digikam

