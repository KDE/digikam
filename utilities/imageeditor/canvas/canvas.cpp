/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2003-01-09
 * Description : image editor canvas management class
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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
#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "imagehistogram.h"
#include "dimginterface.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "canvas.h"

namespace Digikam
{

const int HISTOGRAM_WIDTH  = (256 * 4) / 3;
const int HISTOGRAM_HEIGHT = (114 * 4) / 3;

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
        histogramPixmap = 0;
        imageHistogram  = 0;
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

    // -- Blended Histogram data. Need to be moved in an extarenal class. --------------------
    
    // state variables for histogram visibility
    bool               showHistogram;                       //!< visibility
    bool               histoMovingRepainting;               //!< is about to repaint
    bool               histogramReady;                      //!< state for histogram data thread

    // state variables for repainting when the contents of the
    // image are "moved", i.e. the image is scrolled
    bool               contentsMovingRepaintingHistogram;   //!< repaint cycle state for
                                                            //   histogram while content 
                                                            //   is moving underneath it
    bool               histogramMoving;                     //!< positioning histogram with mouse
        
    int                histoChannelType;                    //!< histogram type; \sa setHistogramType
    
    QRect              contentsMovingDirtyRect;             //!< rectangles for histogram
                                                            //   that should be updated
                                                            //   in the next repaint state

    QRect              histogramRect;                       //!< x y width rect (\sa getHistogramRect)

    QPoint             histoMovingOffset;                   //!< offset

    QRect              histoMovingDirtyRect;                //!< dirty rectangle while moving histogram
    
    QPixmap           *histogramPixmap;                     //!< histogram pixmap buffer

    ImageHistogram    *imageHistogram;                      //!< image histogram data

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

    d->showHistogram   = false;
    d->histogramReady  = false;
    d->imageHistogram  = 0;
    
    d->histogramPixmap = 0;
    createHistogramPixmap();

    d->contentsMovingRepaintingHistogram = false;
   
    d->histogramMoving = false;
    // by default emptyRect so histogram sticks half-centered 
    // on the canvas
    d->histogramRect = QRect(0, 0, 0, 0);
    d->histoMovingOffset = QPoint(0, 0);
    d->histoMovingDirtyRect = QRect(0, 0, 0, 0);
    d->histoMovingRepainting = false;

    d->histoChannelType = ImageHistogram::ValueChannel;

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
    
    connect(this, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotContentsMoving(int,int)));
}

void Canvas::customEvent(QCustomEvent *event)
{
    if (!event) return;

    ImageHistogram::EventData *ed = (ImageHistogram::EventData*) event->data();

    if (!ed) return;

    if (ed->success)
    {
        d->histogramReady = true;
        drawHistogramPixmap();
        QRect rc;
        getHistogramRect(rc);
        viewport()->update(rc);
    }
    
    delete ed;
}

/**
 * Creates a pixmap stored in d->histogramPixmap, which can be alpha
 * blended onto the canvas. 
 */
void Canvas::createHistogramPixmap()
{
    QImage image(HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT, 8, 2);

    image.setAlphaBuffer(true);
    image.setColor(0, qRgba(0, 0, 0, 127));
    image.setColor(1, qRgba(0xff, 0xff, 0xff, 127));
    image.fill(1);

    d->histogramPixmap = new QPixmap(image);
}

/**
 * Draws "busy" on histogram pixmap
 */
void Canvas::drawHistogramPixmapBusy()
{
    uint   wWidth  = HISTOGRAM_WIDTH;
    uint   wHeight = HISTOGRAM_HEIGHT;
    QPainter p(d->histogramPixmap);
    QFont font = p.font();

    font.setBold(true);
    font.setPointSize(12);
    
    p.fillRect(QRect(0, 0, wWidth, wHeight), QBrush(qRgba(0xff, 0xff, 0xff, 127)));
    p.setPen(QPen(qRgba(0, 0, 0, 127), 1, Qt::SolidLine));
    p.setFont(font);
    p.drawText(QRect(0, 0, wWidth, wHeight), 
               AlignCenter | SingleLine | WordBreak,
               i18n("Calculating..."));
}

/**
 * Draws new histogram on pixmap 
 */
void Canvas::drawHistogramPixmap()
{
    uint   wWidth  = HISTOGRAM_WIDTH;
    uint   wHeight = HISTOGRAM_HEIGHT;
    uint   x = 0, y = 0;
    double max = 0.0;
    ImageHistogram *histogram = d->imageHistogram;
    ImageHistogram::HistogramChannelType type;

    if (d->histoChannelType <= 0 || d->histoChannelType > 5)
        return;

    type = static_cast<ImageHistogram::HistogramChannelType>(d->histoChannelType - 1);
    max = histogram->getMaximum(type); 
    
    // make transparent white
    QPainter p(d->histogramPixmap);

    p.fillRect(QRect(0, 0, wWidth, wHeight), QBrush(qRgba(0xff, 0xff, 0xff, 127)));

    // logic stolen from histogramwidget.cpp
     
    for (x = 0 ; x < wWidth ; ++x)
    {
        double value = 0.0; 
        int    i, j;

        i = (x * histogram->getHistogramSegment()) / wWidth;
        j = i + 1;

        do
        {
            double v = 0.0;

            v = histogram->getValue(type, i);
            i++;

            if (v > value)
            value = v;
        }
        while (i < j);

        y = (int) ((wHeight * value) / max);

        QColor hiscolor;

        switch (type)
        {
            case ImageHistogram::RedChannel:
                hiscolor = qRgba(0xFF, 0, 0, 127);
                break;
            case ImageHistogram::GreenChannel:
                hiscolor = qRgba(0, 0xFF, 0, 127);
                break;
            case ImageHistogram::BlueChannel:
                hiscolor = qRgba(0, 0, 0xFF, 127);
                break;
            default:
                hiscolor = qRgba(0, 0, 0, 127);
                break;
        }

        p.setPen(QPen(hiscolor, 1, Qt::SolidLine));
        p.drawLine(x, wHeight, x, wHeight - y);                 
    }

    p.end();
}

Canvas::~Canvas()
{
    delete d->imageHistogram;
    delete d->histogramPixmap;

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

    if (d->imageHistogram)
    {
        delete d->imageHistogram;
        d->imageHistogram = 0;
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
    if (d->showHistogram)
        updateHistogram(true);

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

/**
 * Update histogram on canvas. If @p invalidate is true, new data is
 * is retrieved, and the histogram is redrawn entirely. 
 * @return Returns true if viewport() should be update()d. (Currently
 * returns true always.)
 */
bool Canvas::updateHistogram(bool invalidate)
{
    if (invalidate && d->imageHistogram) 
    {
        delete d->imageHistogram;
        d->imageHistogram = 0;
    }

    if (d->imageHistogram == 0)
    {
        d->histogramReady = false;
        d->imageHistogram = new ImageHistogram(d->im->getImage(),
                                               d->im->origWidth(),
                                               d->im->origHeight(),
                                               d->im->sixteenBit(),
                                               this);
        // paint busy
        drawHistogramPixmapBusy();
    }
    return true;
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
    
    // if resize, set histogram semi-centered again if part 
    // of the histogram is outside the visible width
    QRect rc;

    getHistogramRect(rc);

    if (rc.right() > visibleWidth() ||
        rc.bottom() > visibleHeight()) 
    {
        d->histogramRect.setWidth(0);
        d->histogramRect.setHeight(0);
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
    
    paintHistogram(o_cr);
}

/**
 * Returns histogram's rectangle @hrect in viewport coordinates. If histogram's
 * rectangle is empty, or invalid, the histogram is painted semi-centered on
 * the canvas.
 */
void Canvas::getHistogramRect(QRect &hrect)
{
    if (d->histogramRect.isNull())
    {
        QRect phis(contentsX(), contentsY(), HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
        QRect rc(contentsX(), contentsY(), visibleWidth(), visibleHeight());
        QPoint p;
       
        // "semi-centered" location (3/4 of height, half of width)
        p = QPoint(rc.width() / 2, (3 * rc.height()) / 4);
        phis.moveCenter(p);

        d->histogramRect.setTopLeft(phis.topLeft());
        d->histogramRect.setWidth(0);
        d->histogramRect.setHeight(0);
        hrect = phis;
    }
    else
        hrect = QRect(d->histogramRect.x(), d->histogramRect.y(),
                      HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
}

/**
 * Paints histogram from the histogram's pixmap. @p cr is the canvas 
 * rectangle that was requested to update; if the histogram's area 
 * is not in that rectangle, it is not drawn.  
 */
void Canvas::paintHistogram(const QRect& cr)
{
    if (!d->showHistogram) 
       return;
    if (!d->histogramPixmap)
       return;

    QRect rcexposed, rc, rctmp;

    getHistogramRect(rc);
    rcexposed = rc;
    rctmp = cr;
    rctmp.setTopLeft(contentsToViewport(rctmp.topLeft()));
    rctmp.setBottomRight(contentsToViewport(rctmp.bottomRight()));
    
    rcexposed = rcexposed.intersect(rctmp);
    if (!rcexposed.isEmpty())
    {
        bitBlt(viewport(), rcexposed.x(), rcexposed.y(),
               d->histogramPixmap, 
               rcexposed.x() - rc.x(), rcexposed.y() - rc.y(),
               rcexposed.width(), rcexposed.height());
    }               
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
        // if histogram, and not while repainting histogram
        // because of a contents moving event, check whether
        // the histogram should be moved...
        if (d->showHistogram && d->histogramReady &&
            !d->contentsMovingRepaintingHistogram)
        {    
            QRect rc;
            QPoint pt(e->x(), e->y());

            pt = contentsToViewport(pt);
            getHistogramRect(rc);
            if (rc.contains(pt)) {
                d->histogramMoving = true;
                
                // XXX leave the sticky semi-centered state 
                d->histogramRect.setWidth(HISTOGRAM_WIDTH);
                d->histogramRect.setHeight(HISTOGRAM_HEIGHT);
                
                viewport()->setMouseTracking(false);
                setCursor(KCursor::handCursor());

                // offset of cursor in histogram rect
                d->histoMovingOffset = pt - rc.topLeft();
                d->histoMovingDirtyRect = QRect(0, 0, 0, 0);
                return;
            }
        }
    
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
        // if histogram repositioning in effect, clip and move the
        // histogram
        if (d->histogramMoving) 
        {   
            QPoint pt(e->x(), e->y()), pt_org;
            QRect rc, rc_org;
         
            getHistogramRect(rc);
            rc_org = rc;
           
            // set new histogramRect and clip within viewport
            pt = pt_org = contentsToViewport(pt);
            if (pt.x() < d->histoMovingOffset.x())
                pt.setX(d->histoMovingOffset.x());
            if (pt.y() < d->histoMovingOffset.y())
                pt.setY(d->histoMovingOffset.y());
            if (pt.x() - d->histoMovingOffset.x() + rc.width() > visibleWidth())
                pt.setX(visibleWidth() - rc.width() + d->histoMovingOffset.x());
            if (pt.y() - d->histoMovingOffset.y() + rc.height() > visibleHeight())
                pt.setY(visibleHeight() - rc.height() + d->histoMovingOffset.y());
            d->histogramRect.setTopLeft(pt - d->histoMovingOffset);

            // if histogramRect unchanged, record offset change - but keep 
            // the histogram at this spot
            getHistogramRect(rc);
            if (rc_org == rc)
            {
                QPoint tmp = pt_org - rc.topLeft();
                // clip within histogram's width and size
                if (tmp.x() < 0) tmp.setX(0);
                if (tmp.y() < 0) tmp.setY(0);
                if (tmp.x() > HISTOGRAM_WIDTH)  tmp.setX(HISTOGRAM_WIDTH);
                if (tmp.y() > HISTOGRAM_HEIGHT) tmp.setY(HISTOGRAM_HEIGHT);
                d->histoMovingOffset = tmp;
            }
            
            // combine old rectangle and new rectangle to have the "exposed" 
            // parts of the canvas
            rc_org.unite(rc);
            rc_org.setTopLeft(viewportToContents(rc_org.topLeft()));
            rc_org.setBottomRight(viewportToContents(rc_org.bottomRight()));

            d->histoMovingDirtyRect = rc_org.unite(d->histoMovingDirtyRect);
            if (d->histoMovingRepainting)
                return;

            d->histoMovingRepainting = true;
            QTimer::singleShot(200, this, SLOT(slotHistoMovingPaintHistogram()));
            return;
        }
    
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

    if (e->button() == Qt::LeftButton && d->histogramMoving) 
    {
        // redraw if still something dirty, and a 
        // slotHistoMovingPaintHistogram was not fired
        if (!d->histoMovingRepainting && 
            !d->histoMovingDirtyRect.isEmpty())
            slotHistoMovingPaintHistogram();

        // XXX: a previous "shot" slotHistoMovingPaintHistogram
        // might very well still be pending
    
        viewport()->setMouseTracking(true);
        d->histogramMoving = false;
        setCursor(KCursor::arrowCursor());
        return;
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

void Canvas::slotShowHistogram(bool show)
{
    if (d->showHistogram == show)
        return;
    
    bool update = true;

    d->showHistogram = show;
    if (d->showHistogram) 
	    update = updateHistogram(false);
    
    if (update)
    { 
	    QRect rc;
	    getHistogramRect(rc);
        drawHistogramPixmap();
	    viewport()->update(rc);
    }	
}

void Canvas::slotToggleShowHistogram()
{
    slotShowHistogram(!d->showHistogram);
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

/**
 * Set histogram position in viewport coords
 */
void Canvas::setHistogramPosition(const QPoint& pos)
{
    // force width and height (by default width and
    // height are 0, which means histogram is semi-centered
    // on the canvas)
    d->histogramRect = QRect(pos.x(), pos.y(), HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
}

/**
 * Return histogram position in viewport coords
 */
bool Canvas::getHistogramPosition(QPoint &pos)
{
    if (d->histogramRect.isNull()) 
    { 
        pos = QPoint(-1, -1);
        return false;
    }
    else 
    {
        pos = d->histogramRect.topLeft();
        return true;
    }        
}

/**
 * @p type: 0 = none, 1 = lumi, 2 = red, 3 = green,
 * 4 = blue, 5 = alpha
 */
int Canvas::setHistogramType(int t)
{
    if (t == 0)
    {
        d->histoChannelType = t;
        slotShowHistogram(false);
    }
    else if (t != d->histoChannelType && t > 0 && t < 6)
    {
        d->histoChannelType = t;
        slotShowHistogram(true);
        QRect rc;
        getHistogramRect(rc);
        drawHistogramPixmap();
        viewport()->update(rc);
    }        
    return d->histoChannelType;
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
    if (d->showHistogram)
        updateHistogram(true);

    if (d->autoZoom)
        updateAutoZoom();
    d->im->zoom(d->zoom);

    updateContentsSize();
    viewport()->update();
    emit signalChanged();
}

/**
 * When contents are moved (scrolled), collect the "dirty" 
 * histogram parts, and update in a timely manner.
 */
void Canvas::slotContentsMoving(int x, int y)
{
    if (!d->showHistogram) 
        return;

    QPoint po(contentsX(), contentsY()), 
           pn(x, y);
    
    int dx = pn.x() - po.x(), 
        dy = pn.y() - po.y();

    if (dx || dy)
    {
        QRect rchisto, rcdirty;
            
        getHistogramRect(rchisto);

        rchisto.setTopLeft(viewportToContents(rchisto.topLeft()));
        rchisto.setBottomRight(viewportToContents(rchisto.bottomRight()));
        rcdirty = rchisto;
        rcdirty.moveBy(dx, dy);

        d->contentsMovingDirtyRect = rcdirty.unite(d->contentsMovingDirtyRect);

        if (d->contentsMovingRepaintingHistogram)
            return;

        d->contentsMovingDirtyRect = rchisto.unite(d->contentsMovingDirtyRect);	
        d->contentsMovingRepaintingHistogram = true;
        QTimer::singleShot(200, this, SLOT(slotContentsMovingPaintHistogram()));
    }	
}

/**
 * Actual repaint when contents moved, triggered after a time-out
 */
void Canvas::slotContentsMovingPaintHistogram()
{
    QRect rcdirty = d->contentsMovingDirtyRect;

    repaintContents(rcdirty);

    d->contentsMovingDirtyRect = QRect(0, 0, 0, 0);
    d->contentsMovingRepaintingHistogram = false;
}

/**
 * Actual repaint when histogram is being grabbed and moved
 */
void Canvas::slotHistoMovingPaintHistogram()
{
    QRect rcdirty = d->histoMovingDirtyRect;

    repaintContents(rcdirty);

    d->histoMovingDirtyRect = QRect(0, 0, 0, 0);
    d->histoMovingRepainting = false;
}

void Canvas::slotPaintSmooth()
{
    if (d->paintTimer->isActive())
        return;

    paintViewport(contentsRect(), true);
}

}  // namespace Digikam

#include "canvas.moc"
