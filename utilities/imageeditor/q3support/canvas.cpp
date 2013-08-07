/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-09
 * Description : image editor canvas management class
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "canvas.moc"

// C++ includes

#include <cstdio>
#include <cmath>

// Qt includes

#include <QCache>
#include <QFile>
#include <QString>
#include <QEvent>
#include <QPoint>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QStyle>
#include <QApplication>
#include <QCursor>
#include <QFileInfo>
#include <QImage>
#include <QRegion>
#include <QTimer>
#include <QColor>
#include <QClipboard>
#include <QToolButton>
#include <QFrame>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>

// KDE includes

#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdatetable.h>
#include <kglobalsettings.h>
#include <kapplication.h>

// Local includes

#include "autocrop.h"
#include "imagehistogram.h"
#include "paniconwidget.h"
#include "editorcore.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "iofilesettings.h"
#include "loadingcacheinterface.h"
#include "drubberband.h"

namespace Digikam
{

class Canvas::Private
{

public:

    Private() :
        snapArea(8), tileSize(128), minZoom(0.1), maxZoom(12.0), zoomMultiplier(1.2)
    {
        pressedMoved     = false;
        pressedMoving    = false;
        ltActive         = false;
        rtActive         = false;
        lbActive         = false;
        rbActive         = false;
        lsActive         = false;
        rsActive         = false;
        bsActive         = false;
        tsActive         = false;
        dragActive       = false;
        midButtonPressed = false;
        midButtonX       = 0;
        midButtonY       = 0;
        panIconPopup     = 0;
        cornerButton     = 0;
        parent           = 0;
        im               = 0;
        rubber           = 0;
        autoZoom         = false;
        fullScreen       = false;
        zoom             = 1.0;
        initialZoom      = true;
        tileTmpPix       = new QPixmap(tileSize, tileSize);

        tileCache.setMaxCost((10 * 1024 * 1024) / (tileSize * tileSize * 4));
    }

    bool                     autoZoom;
    bool                     fullScreen;
    bool                     pressedMoved;
    bool                     pressedMoving;
    bool                     ltActive;
    bool                     rtActive;
    bool                     lbActive;
    bool                     rbActive;
    bool                     lsActive;
    bool                     rsActive;
    bool                     bsActive;
    bool                     tsActive;
    bool                     dragActive;
    bool                     midButtonPressed;
    bool                     initialZoom;

    const int                snapArea;
    const int                tileSize;
    int                      midButtonX;
    int                      midButtonY;

    double                   zoom;
    const double             minZoom;
    const double             maxZoom;
    const double             zoomMultiplier;

    QToolButton*             cornerButton;

    DRubberBand*             rubber;
    QRect                    pixmapRect;
    QPoint                   dragStart;
    QRect                    dragStartRect;

    QCache<QString, QPixmap> tileCache;

    QPixmap*                 tileTmpPix;
    QPixmap                  qcheck;

    QColor                   bgColor;

    QWidget*                 parent;

    KPopupFrame*             panIconPopup;

    EditorCore*           im;

    QString                  errorMessage;
};

Canvas::Canvas(QWidget* const parent)
    : Q3ScrollView(parent), d(new Private)
{
    d->im     = new EditorCore();
    d->parent = parent;
    d->bgColor.setRgb(0, 0, 0);

    d->qcheck = QPixmap(16, 16);
    QPainter p(&d->qcheck);
    p.fillRect(0, 0, 8, 8, QColor(144, 144, 144));
    p.fillRect(8, 8, 8, 8, QColor(144, 144, 144));
    p.fillRect(0, 8, 8, 8, QColor(100, 100, 100));
    p.fillRect(8, 0, 8, 8, QColor(100, 100, 100));
    p.end();

    d->cornerButton = PanIconWidget::button();
    setCornerWidget(d->cornerButton);

    QPalette palette;
    palette.setColor(viewport()->backgroundRole(), Qt::WA_NoBackground);
    viewport()->setPalette(palette);
    viewport()->setMouseTracking(false);
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::ClickFocus);

    d->rubber = new DRubberBand(this);

    d->im->setDisplayingWidget(this);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(d->im, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->im, SIGNAL(signalLoadingStarted(QString)),
            this, SIGNAL(signalLoadingStarted(QString)));

    connect(d->im, SIGNAL(signalImageLoaded(QString,bool)),
            this, SLOT(slotImageLoaded(QString,bool)));

    connect(d->im, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    connect(d->im, SIGNAL(signalLoadingProgress(QString,float)),
            this, SIGNAL(signalLoadingProgress(QString,float)));

    connect(d->im, SIGNAL(signalSavingStarted(QString)),
            this, SIGNAL(signalSavingStarted(QString)));

    connect(d->im, SIGNAL(signalSavingProgress(QString,float)),
            this, SIGNAL(signalSavingProgress(QString,float)));

    connect(this, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected()));
}

Canvas::~Canvas()
{
    delete d->tileTmpPix;
    delete d->im;
    delete d->rubber;
    delete d;
}

void Canvas::resetImage()
{
    reset();
    d->im->resetImage();
}

void Canvas::reset()
{
    if (d->rubber->isActive())
    {
        d->rubber->setActive(false);

        if (d->im->imageValid())
        {
            emit signalSelected(false);
        }
    }

    d->errorMessage.clear();
    d->tileCache.clear();
}

void Canvas::load(const QString& filename, IOFileSettings* const IOFileSettings)
{
    //qDebug()<<"Canvas::load";
    reset();

    emit signalPrepareToLoad();
    d->im->load(filename, IOFileSettings);
}

void Canvas::slotImageLoaded(const QString& filePath, bool success)
{
    //qDebug()<<"Canvas::slotImageLoaded";
    d->im->zoom(d->zoom);

    if (d->autoZoom || d->initialZoom)
    {
        d->initialZoom = false;
        updateAutoZoom();
    }

    // Note: in showFoto, we using a null filename to clear canvas.
    if (!success && !filePath.isEmpty())
    {
        QFileInfo info(filePath);
        d->errorMessage = i18n("Failed to load image\n\"%1\"", info.fileName());
    }
    else
    {
        d->errorMessage.clear();
    }

    updateContentsSize(true);

    viewport()->update();

    emit signalZoomChanged(d->zoom);

    emit signalLoadingFinished(filePath, success);
}

void Canvas::applyTransform(const IccTransform& t)
{
    qDebug()<<"Canvas::applyTransform";
    IccTransform transform(t);

    if (transform.willHaveEffect())
    {
        d->im->applyTransform(transform);
    }
    else
    {
        d->im->updateColorManagement();
        d->tileCache.clear();
        viewport()->update();
    }
}

void Canvas::preload(const QString& /*filename*/)
{
    //    d->im->preload(filename);
}

void Canvas::slotImageSaved(const QString& filePath, bool success)
{
    emit signalSavingFinished(filePath, success);
}

void Canvas::abortSaving()
{
    d->im->abortSaving();
}

void Canvas::setModified()
{
    d->im->setModified();
}

QString Canvas::ensureHasCurrentUuid() const
{
    return d->im->ensureHasCurrentUuid();
}

DImg Canvas::currentImage() const
{
    DImg* image = d->im->getImg();

    if (image)
    {
        return DImg(*image);
    }

    return DImg();
}

QString Canvas::currentImageFileFormat() const
{
    return d->im->getImageFormat();
}

QString Canvas::currentImageFilePath() const
{
    return d->im->getImageFilePath();
}

int Canvas::imageWidth() const
{
    return d->im->origWidth();
}

int Canvas::imageHeight() const
{
    return d->im->origHeight();
}

bool Canvas::isReadOnly() const
{
    return d->im->isReadOnly();
}

QRect Canvas::getSelectedArea() const
{
    return (d->im->getSelectedArea());
}

QRect Canvas::visibleArea() const
{
    return (QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()));
}

EditorCore* Canvas::interface() const
{
    return d->im;
}

void Canvas::makeDefaultEditingCanvas()
{
    EditorCore::setDefaultInstance(d->im);
}

double Canvas::calcAutoZoomFactor() const
{
    qDebug()<<"Canvas::calcAutoZoomFactor";
    if (!d->im->imageValid())
    {
        return d->zoom;
    }

    double srcWidth  = d->im->origWidth();
    double srcHeight = d->im->origHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();
    return qMin(dstWidth / srcWidth, dstHeight / srcHeight);
}

void Canvas::updateAutoZoom()
{
    qDebug()<<"Canvas::updateAutoZoom";
    d->zoom = calcAutoZoomFactor();
    d->im->zoom(d->zoom);
    emit signalZoomChanged(d->zoom);
}

void Canvas::updateContentsSize(bool deleteRubber)
{
    qDebug()<<"Canvas::updateContentsSize";
    viewport()->setUpdatesEnabled(false);

    if (deleteRubber && d->rubber->isActive())
    {
        d->rubber->setActive(false);
        d->ltActive     = false;
        d->rtActive     = false;
        d->lbActive     = false;
        d->rbActive     = false;
        d->lsActive     = false;
        d->rsActive     = false;
        d->bsActive     = false;
        d->tsActive     = false;
        d->dragActive   = false;
        d->pressedMoved = false;
        viewport()->unsetCursor();
        viewport()->setMouseTracking(false);

        if (d->im->imageValid())
        {
            emit signalSelected(false);
        }
    }

    int wZ = d->im->width();
    int hZ = d->im->height();

    if (visibleWidth() > wZ || visibleHeight() > hZ)
    {
        // Center the image
        int centerx   = contentsRect().width()  / 2;
        int centery   = contentsRect().height() / 2;
        int xoffset   = int(centerx - wZ / 2);
        int yoffset   = int(centery - hZ / 2);
        xoffset       = qMax(xoffset, 0);
        yoffset       = qMax(yoffset, 0);
        d->pixmapRect = QRect(xoffset, yoffset, wZ, hZ);
    }
    else
    {
        d->pixmapRect = QRect(0, 0, wZ, hZ);
    }

    // always restrict rubberband to area covered by image
    d->rubber->setRestrictionOnContents(d->pixmapRect);

    if (!deleteRubber && d->rubber->isActive())
    {
        QRect sel = d->im->getSelectedArea();
        int xSel  = (int)((sel.x()      * d->tileSize) / floor(d->tileSize / d->zoom));
        int ySel  = (int)((sel.y()      * d->tileSize) / floor(d->tileSize / d->zoom));
        int wSel  = (int)((sel.width()  * d->tileSize) / floor(d->tileSize / d->zoom));
        int hSel  = (int)((sel.height() * d->tileSize) / floor(d->tileSize / d->zoom));
        QRect rubberRect(xSel, ySel, wSel, hSel);
        rubberRect.translate(d->pixmapRect.x(), d->pixmapRect.y());
        d->rubber->setRectOnViewport(rubberRect);
    }

    d->tileCache.clear();
    resizeContents(wZ, hZ);
    viewport()->setUpdatesEnabled(true);
}

void Canvas::resizeEvent(QResizeEvent* e)
{
    qDebug()<<"Canvas::resizeEvent";
    if (!e)
    {
        return;
    }

    Q3ScrollView::resizeEvent(e);

    if (d->autoZoom)
    {
        updateAutoZoom();
    }

    updateContentsSize(false);

    // No need to repaint. its called
    // automatically after resize

    // To be sure than corner widget used to pan image will be hide/show
    // accordingly with resize event.
    slotZoomChanged(d->zoom);
}

void Canvas::viewportPaintEvent(QPaintEvent* e)
{
    qDebug()<<"Canvas::viewportPaintEvent";
    QRect er(e->rect());
    er = QRect(qMax(er.x()      - 1, 0),
               qMax(er.y()      - 1, 0),
               qMin(er.width()  + 2, contentsRect().width()),
               qMin(er.height() + 2, contentsRect().height()));

    paintViewport(er, (d->zoom <= 1.0) ? true : false);
}

void Canvas::paintViewport(const QRect& er, bool antialias)
{
    qDebug()<<"Canvas::paintViewport";
    QRect o_cr(viewportToContents(er.topLeft()), viewportToContents(er.bottomRight()));
    QRect cr = o_cr;

    QRegion clipRegion(er);
    cr = d->pixmapRect.intersect(cr);
    QPainter painter(viewport());

    if (!cr.isEmpty() && d->im->imageValid())
    {
        clipRegion -= QRect(contentsToViewport(cr.topLeft()), cr.size());

        QRect pr = QRect(cr.x() - d->pixmapRect.x(), cr.y() - d->pixmapRect.y(),
                         cr.width(), cr.height());

        int x1   = (int)floor((double)pr.x()      / (double)d->tileSize) * d->tileSize;
        int y1   = (int)floor((double)pr.y()      / (double)d->tileSize) * d->tileSize;
        int x2   = (int)ceilf((double)pr.right()  / (double)d->tileSize) * d->tileSize;
        int y2   = (int)ceilf((double)pr.bottom() / (double)d->tileSize) * d->tileSize;

        QPixmap pix(d->tileSize, d->tileSize);
        int sx, sy, sw, sh;
        int step = (int)floor(d->tileSize / d->zoom);

        //bool hasRubber = (d->rubber->isVisible() && d->pressedMoved && d->pressedMoving && d->rubber->geometry().intersects(pr));

        for (int j = y1 ; j < y2 ; j += d->tileSize)
        {
            for (int i = x1 ; i < x2 ; i += d->tileSize)
            {
                QString key  = QString("%1,%2").arg(i).arg(j);
                QPixmap* pix = d->tileCache.object(key);

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

                    // NOTE : with implementations <= 0.9.1, the canvas doesn't work properly using high zoom level (> 500).
                    // The sx, sy, sw, sh values haven't be computed properly and "tile" artifacts been appears
                    // over the image.
                    // Note these "tile" artifacts are not the real tiles of canvas.
                    // The new implementation below fix this problem to handle properly the areas to
                    // use from the source image to generate the canvas pixmap tiles.

                    sx = (int)floor((double)i / d->tileSize) * step;
                    sy = (int)floor((double)j / d->tileSize) * step;
                    sw = step;
                    sh = step;

                    if (d->rubber->isActive() && d->pressedMoved && !d->pressedMoving)
                    {
                        QRect rr(d->rubber->rubberBandAreaOnContents());
                        rr = QRect(rr.x(), rr.y(), rr.width() - 1, rr.height() - 1);
                        QRect r(i, j, d->tileSize, d->tileSize);

                        d->im->paintOnDevice(pix,
                                             QRect(sx, sy, sw, sh),
                                             QRect(0, 0, d->tileSize, d->tileSize),
                                             QRect(rr.x() - i - d->pixmapRect.x(), rr.y() - j - d->pixmapRect.y(), rr.width(), rr.height()),
                                             antialias);

                        rr.translate(-i - d->pixmapRect.x(), -j - d->pixmapRect.y());

                        QPainter p(pix);
                        p.setPen(d->rubber->palette().color(QPalette::Active, QPalette::Highlight));
                        p.drawRect(rr);

                        const int halfSA = d->snapArea / 2;

                        if (rr.width() >= d->snapArea && rr.height() >= d->snapArea)
                        {
                            p.drawRect(QRect(rr.x() - halfSA,              rr.y() - halfSA,               d->snapArea, d->snapArea));
                            p.drawRect(QRect(rr.x() - halfSA,              rr.y() + rr.height() - halfSA, d->snapArea, d->snapArea));
                            p.drawRect(QRect(rr.x() + rr.width() - halfSA, rr.y() + rr.height() - halfSA, d->snapArea, d->snapArea));
                            p.drawRect(QRect(rr.x() + rr.width() - halfSA, rr.y() - halfSA,               d->snapArea, d->snapArea));
                        }

                        p.end();
                    }
                    else
                    {
                        d->im->paintOnDevice(pix,
                                             QRect(sx, sy, sw, sh),
                                             QRect(0, 0, d->tileSize, d->tileSize),
                                             antialias);
                    }
                }

                QRect  r(i, j, d->tileSize, d->tileSize);
                QRect  ir = pr.intersect(r);
                QPoint pt(contentsToViewport(QPoint(ir.x() + d->pixmapRect.x(),
                                                    ir.y() + d->pixmapRect.y())));

                painter.drawPixmap(pt.x(), pt.y(),
                                   *pix,
                                   ir.x() - r.x(), ir.y() - r.y(),
                                   ir.width(), ir.height());
            }
        }
    }
    else if (!d->im->imageValid() && !d->errorMessage.isEmpty())
    {
        QRect fullRect(0, 0, visibleWidth(), visibleHeight());
        QRect textRect = painter.boundingRect(fullRect, Qt::AlignCenter | Qt::TextWordWrap, d->errorMessage);
        painter.fillRect(textRect, kapp->palette().color(QPalette::Base));
        painter.setPen(QPen(kapp->palette().color(QPalette::Text)));
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, d->errorMessage);

        clipRegion -= textRect;
    }

    painter.setClipRegion(clipRegion);
    painter.fillRect(er, d->bgColor);
    painter.end();
}

void Canvas::contentsMousePressEvent(QMouseEvent* e)
{
    if (!e || e->button() == Qt::RightButton)
    {
        return;
    }

    d->midButtonPressed = false;

    if (e->button() == Qt::LeftButton)
    {
        if (d->ltActive || d->rtActive ||
            d->lbActive || d->rbActive ||
            d->lsActive || d->rsActive ||
            d->tsActive || d->bsActive ||
            d->dragActive)
        {
            if (!d->rubber->isActive())
            {
                return;
            }

            // Set diagonally opposite corner as anchor

            QRect r(d->rubber->rubberBandAreaOnContents());

            if (d->ltActive || d->tsActive)
            {
                d->rubber->setFirstPointOnViewport(r.bottomRight());
                d->rubber->setSecondPointOnViewport(r.topLeft());
            }
            else if (d->rtActive || d->rsActive)
            {
                d->rubber->setFirstPointOnViewport(r.bottomLeft());
                d->rubber->setSecondPointOnViewport(r.topRight());
            }
            else if (d->lbActive || d->lsActive)
            {
                d->rubber->setFirstPointOnViewport(r.topRight());
                d->rubber->setSecondPointOnViewport(r.bottomLeft());
            }
            else if (d->rbActive || d->bsActive)
            {
                d->rubber->setFirstPointOnViewport(r.topLeft());
                d->rubber->setSecondPointOnViewport(r.bottomRight());
            }
            else
            {
                d->dragStartRect = d->rubber->rubberBandAreaOnContents();
                d->dragStart = e->pos();
            }

            viewport()->setMouseTracking(false);
            d->pressedMoved  = false;
            d->pressedMoving = true;

            d->tileCache.clear();
            viewport()->repaint();

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

    d->rubber->setFirstPointOnViewport(e->pos());

    if (d->pressedMoved)
    {
        d->tileCache.clear();
        viewport()->update();
    }

    d->pressedMoved  = false;
    d->pressedMoving = true;

    viewport()->setMouseTracking(false);
}

void Canvas::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (e->buttons() & Qt::MidButton)
    {
        if (d->midButtonPressed)
        {
            scrollBy(d->midButtonX - e->x(),
                     d->midButtonY - e->y());
        }
    }
    else if (!viewport()->hasMouseTracking())
    {
        if (!d->rubber->isActive())
        {
            return;
        }

        if (e->buttons() != Qt::LeftButton &&
            !(d->ltActive || d->rtActive ||
              d->lbActive || d->rbActive ||
              d->lsActive || d->rsActive ||
              d->tsActive || d->bsActive ||
              d->dragActive))
        {
            return;
        }

        // Move content if necessary.
        blockSignals(true);
        setUpdatesEnabled(false);
        ensureVisible(e->x(), e->y(), 10, 10);
        setUpdatesEnabled(true);
        blockSignals(false);

        // set the new rubber position.
        if (!(d->lsActive || d->rsActive ||
              d->tsActive || d->bsActive ||
              d->dragActive))
        {
            d->rubber->setSecondPointOnViewport(e->pos());
        }
        else
        {
            QRect r(d->rubber->rubberBandAreaOnContents());

            if (d->lsActive)
            {
                d->rubber->setSecondPointOnViewport(QPoint(e->x(), r.bottom()));
            }
            else if (d->rsActive)
            {
                d->rubber->setSecondPointOnViewport(QPoint(e->x(), r.y()));
            }
            else if (d->tsActive)
            {
                d->rubber->setSecondPointOnViewport(QPoint(r.x(), e->y()));
            }
            else if (d->bsActive)
            {
                d->rubber->setSecondPointOnViewport(QPoint(r.right(), e->y()));
            }
            else
            {
                QPoint tr = e->pos() - d->dragStart;
                QRect nr = d->dragStartRect;
                nr.translate(tr);

                if (nr.left() <= d->pixmapRect.left())
                {
                    nr.moveLeft(d->pixmapRect.left());
                }
                else if (nr.right() >= d->pixmapRect.right())
                {
                    nr.moveRight(d->pixmapRect.right());
                }

                if (nr.top() <= d->pixmapRect.top())
                {
                    nr.moveTop(d->pixmapRect.top());
                }
                else if (nr.bottom() >= d->pixmapRect.bottom())
                {
                    nr.moveBottom(d->pixmapRect.bottom());
                }

                d->rubber->setRectOnViewport(nr);
                d->dragStart = e->pos();
                d->dragStartRect = nr;
            }
        }

        d->pressedMoved  = true;
        d->pressedMoving = true;

        // To refresh editor status bar with current selection.
        emit signalSelectionChanged(calcSelectedArea());
    }
    else
    {
        if (!d->rubber->isActive())
        {
            return;
        }

        const int halfSA = d->snapArea / 2;

        QRect r(d->rubber->rubberBandAreaOnContents());

        QRect lt(r.x() - halfSA,             r.y() - halfSA,              d->snapArea,              d->snapArea);

        QRect rt(r.x() + r.width() - halfSA, r.y() - halfSA,              d->snapArea,              d->snapArea);

        QRect lb(r.x() - halfSA,             r.y() + r.height() - halfSA, d->snapArea,              d->snapArea);

        QRect rb(r.x() + r.width() - halfSA, r.y() + r.height() - halfSA, d->snapArea,              d->snapArea);

        QRect ls(r.x() - halfSA,             r.y() + halfSA,              d->snapArea,              r.height() - d->snapArea);

        QRect rs(r.x() + r.width() - halfSA, r.y() + halfSA,              d->snapArea,              r.height() - d->snapArea);

        QRect ts(r.x() + halfSA,             r.y() - halfSA,              r.width() - d->snapArea,  d->snapArea);

        QRect bs(r.x() + halfSA,             r.y() + r.height() - halfSA, r.width() - d->snapArea,  d->snapArea);

        QRect dg(r.x() + halfSA,             r.y() + halfSA,              r.width() - d->snapArea,  r.height() - d->snapArea);

        d->ltActive   = false;

        d->rtActive   = false;

        d->lbActive   = false;

        d->rbActive   = false;

        d->lsActive   = false;

        d->rsActive   = false;

        d->bsActive   = false;

        d->tsActive   = false;

        d->dragActive = false;

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
        else if (ls.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeHorCursor);
            d->lsActive = true;
        }
        else if (ts.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeVerCursor);
            d->tsActive = true;
        }
        else if (rs.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeHorCursor);
            d->rsActive = true;
        }
        else if (bs.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeVerCursor);
            d->bsActive = true;
        }
        else if (dg.contains(e->x(), e->y()))
        {
            viewport()->setCursor(Qt::SizeAllCursor);
            d->dragActive = true;
        }
        else
        {
            viewport()->unsetCursor();
        }
    }
}

void Canvas::contentsMouseReleaseEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    d->midButtonPressed = false;

    if (d->pressedMoving)
    {
        d->pressedMoving = false;
        viewport()->update();
    }

    if (d->pressedMoved && d->rubber->isActive())
    {
        d->tileCache.clear();
        viewport()->setMouseTracking(true);

        if (d->im->imageValid())
        {
            emit signalSelected(true);
        }
    }
    else
    {
        d->ltActive   = false;
        d->rtActive   = false;
        d->lbActive   = false;
        d->rbActive   = false;
        d->lsActive   = false;
        d->rsActive   = false;
        d->bsActive   = false;
        d->tsActive   = false;
        d->dragActive = false;
        d->rubber->setActive(false);
        viewport()->setMouseTracking(false);
        viewport()->unsetCursor();

        if (d->im->imageValid())
        {
            emit signalSelected(false);
        }
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

void Canvas::contentsWheelEvent(QWheelEvent* e)
{
    e->accept();

    if (e->modifiers() & Qt::ShiftModifier)
    {
        if (e->delta() < 0)
        {
            emit signalShowNextImage();
        }
        else if (e->delta() > 0)
        {
            emit signalShowPrevImage();
        }

        return;
    }
    else if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->delta() < 0)
        {
            slotDecreaseZoom();
        }
        else if (e->delta() > 0)
        {
            slotIncreaseZoom();
        }

        return;
    }

    Q3ScrollView::contentsWheelEvent(e);
}

bool Canvas::maxZoom() const
{
    return ((d->zoom * d->zoomMultiplier) >= d->maxZoom);
}

bool Canvas::minZoom() const
{
    return ((d->zoom / d->zoomMultiplier) <= d->minZoom);
}

double Canvas::zoomMax() const
{
    return d->maxZoom;
}

double Canvas::zoomMin() const
{
    return d->minZoom;
}

bool Canvas::exifRotated() const
{
    return d->im->exifRotated();
}

double Canvas::snapZoom(double zoom) const
{
    qDebug()<<"Canvas::snapZoom"<<zoom;
    // If the zoom value gets changed from d->zoom to zoom
    // across 50%, 100% or fit-to-window, then return the
    // the corresponding special value. Otherwise zoom is returned unchanged.
    double fit = calcAutoZoomFactor();
    QList<double> snapValues;
    snapValues.append(0.5);
    snapValues.append(1.0);
    snapValues.append(fit);

    if (d->zoom < zoom)
    {
        qStableSort(snapValues);
    }
    else
    {
        qStableSort(snapValues.begin(), snapValues.end(), qGreater<double>());
    }

    for (QList<double>::const_iterator it = snapValues.constBegin(); it != snapValues.constEnd(); ++it)
    {
        double z = *it;

        if ((d->zoom < z) && (zoom > z))
        {
            zoom = z;
            break;
        }
    }

    return zoom;
}

void Canvas::slotIncreaseZoom()
{
    if (maxZoom())
    {
        return;
    }

    double zoom = d->zoom * d->zoomMultiplier;
    zoom        = snapZoom(zoom);
    setZoomFactor(zoom);
}

void Canvas::slotDecreaseZoom()
{
    if (minZoom())
    {
        return;
    }

    double zoom = d->zoom / d->zoomMultiplier;
    zoom        = snapZoom(zoom);
    setZoomFactor(zoom);
}

void Canvas::setZoomFactorSnapped(double zoom)
{
    qDebug()<<"Canvas::setZoomFactorSnapped";
    double fit = calcAutoZoomFactor();

    if (fabs(zoom - fit) < 0.05)
    {
        // If 1.0 or 0.5 are even closer to zoom than fit, then choose these.
        if (fabs(zoom - fit) > fabs(zoom - 1.0))
        {
            zoom = 1.0;
        }
        else if (fabs(zoom - fit) > fabs(zoom - 0.5))
        {
            zoom = 0.5;
        }
        else
        {
            zoom = fit;
        }
    }
    else
    {
        if (fabs(zoom - 1.0) < 0.05)
        {
            zoom = 1.0;
        }

        if (fabs(zoom - 0.5) < 0.05)
        {
            zoom = 0.5;
        }
    }

    setZoomFactor(zoom);
}

double Canvas::zoomFactor() const
{
    return d->zoom;
}

void Canvas::setZoomFactor(double zoom)
{
    if (d->autoZoom)
    {
        d->autoZoom = false;
        emit signalToggleOffFitToWindow();
    }

    // Zoom using center of canvas and given zoom factor.

    double cpx = contentsX() + visibleWidth()  / 2.0;
    double cpy = contentsY() + visibleHeight() / 2.0;

    cpx        = (cpx / d->tileSize) * floor(d->tileSize / d->zoom);
    cpy        = (cpy / d->tileSize) * floor(d->tileSize / d->zoom);

    d->zoom    = zoom;

    d->im->zoom(d->zoom);
    updateContentsSize(false);

    viewport()->setUpdatesEnabled(false);
    center((int)((cpx * d->tileSize) / floor(d->tileSize / d->zoom)),
           (int)((cpy * d->tileSize) / floor(d->tileSize / d->zoom)));
    viewport()->setUpdatesEnabled(true);
    viewport()->update();

    emit signalZoomChanged(d->zoom);
}

void Canvas::fitToSelect()
{
    qDebug()<<"Canvas::fitToSelect";
    QRect sel = d->im->getSelectedArea();

    if (!sel.size().isNull())
    {
        // If selected area, use center of selection
        // and recompute zoom factor accordingly.
        double cpx       = sel.x() + sel.width()  / 2.0;
        double cpy       = sel.y() + sel.height() / 2.0;
        double srcWidth  = sel.width();
        double srcHeight = sel.height();
        double dstWidth  = contentsRect().width();
        double dstHeight = contentsRect().height();
        d->zoom          = qMin(dstWidth / srcWidth, dstHeight / srcHeight);
        d->autoZoom      = false;

        emit signalToggleOffFitToWindow();
        d->im->zoom(d->zoom);
        updateContentsSize(true);

        viewport()->setUpdatesEnabled(false);
        center((int)((cpx * d->tileSize) / floor(d->tileSize / d->zoom)),
               (int)((cpy * d->tileSize) / floor(d->tileSize / d->zoom)));
        viewport()->setUpdatesEnabled(true);
        viewport()->update();

        emit signalZoomChanged(d->zoom);
    }
}

bool Canvas::fitToWindow() const
{
    return d->autoZoom;
}

void Canvas::toggleFitToWindow()
{
    setFitToWindow(!d->autoZoom);
}

void Canvas::setFitToWindow(bool fit)
{
    d->autoZoom = fit;

    if (d->autoZoom)
    {
        updateAutoZoom();
    }
    else
    {
        d->zoom = 1.0;
        emit signalZoomChanged(d->zoom);
    }

    d->im->zoom(d->zoom);
    updateContentsSize(false);
    slotZoomChanged(d->zoom);
    viewport()->update();
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

void Canvas::slotAutoCrop()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    AutoCrop ac(d->im->getImg());
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();
    d->im->crop(rect);

    QApplication::restoreOverrideCursor();
}

void Canvas::slotCrop()
{
    qDebug()<<"Canvas::slotCrop()";
    QRect sel = d->im->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    d->im->crop(sel);
}

void Canvas::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
    {
        return;
    }

    d->bgColor = color;
    viewport()->update();
}

void Canvas::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    ICCSettingsContainer old = d->im->getICCSettings();
    d->im->setICCSettings(cmSettings);
    d->tileCache.clear();
    viewport()->update();
}

void Canvas::setSoftProofingEnabled(bool enable)
{
    d->im->setSoftProofingEnabled(enable);
    d->tileCache.clear();
    viewport()->update();
}

void Canvas::setExposureSettings(ExposureSettingsContainer* const expoSettings)
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

void Canvas::slotRestore()
{
    d->im->restore();
}

void Canvas::slotUndo(int steps)
{
    emit signalUndoSteps(steps);

    while (steps > 0)
    {
        d->im->undo();
        --steps;
    }
}

void Canvas::slotRedo(int steps)
{
    emit signalRedoSteps(steps);

    while (steps > 0)
    {
        d->im->redo();
        --steps;
    }
}

void Canvas::slotCopy()
{
    QRect sel = d->im->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    DImg selDImg        = d->im->getImgSelection();
    QImage selImg       = selDImg.copyQImage();
    QMimeData* mimeData = new QMimeData();
    mimeData->setImageData(selImg);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    QApplication::restoreOverrideCursor();
}

void Canvas::slotSelected()
{
    QRect sel = QRect(0, 0, 0, 0);

    if (d->rubber->isActive() && d->pressedMoved)
    {
        sel = calcSelectedArea();
    }

    d->im->setSelectedArea(sel);
}

QRect Canvas::calcSelectedArea() const
{
    int x = 0, y = 0, w = 0, h = 0;
    QRect r(d->rubber->rubberBandAreaOnContents());

    if (r.isValid())
    {
        r.translate(- d->pixmapRect.x(), - d->pixmapRect.y());

        x = (int)(((double)r.x()      / d->tileSize) * floor(d->tileSize / d->zoom));
        y = (int)(((double)r.y()      / d->tileSize) * floor(d->tileSize / d->zoom));
        w = (int)(((double)r.width()  / d->tileSize) * floor(d->tileSize / d->zoom));
        h = (int)(((double)r.height() / d->tileSize) * floor(d->tileSize / d->zoom));

        x = qMin(imageWidth(),  qMax(x, 0));
        y = qMin(imageHeight(), qMax(y, 0));
        w = qMin(imageWidth(),  qMax(w, 0));
        h = qMin(imageHeight(), qMax(h, 0));

        // Avoid empty selection by rubberband - at least mark one pixel
        // At high zoom factors, the rubberband may operate at subpixel level!
        if (w == 0)
        {
            w = 1;
        }

        if (h == 0)
        {
            h = 1;
        }
    }

    return QRect(x, y, w, h);
}

void Canvas::slotModified()
{
    if (d->autoZoom)
    {
        updateAutoZoom();
    }

    d->im->zoom(d->zoom);

    updateContentsSize(true);
    viewport()->update();

    // To be sure than corner widget used to pan image will be hide/show
    // accordingly with new image size (if changed).
    slotZoomChanged(d->zoom);

    emit signalChanged();
}

void Canvas::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup          = new KPopupFrame(this);
    PanIconWidget* const pan = new PanIconWidget(d->panIconPopup);

    connect(pan, SIGNAL(signalSelectionMoved(QRect,bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect,bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHidden()));

    QRect r((int)(contentsX()    / d->zoom), (int)(contentsY()     / d->zoom),
            (int)(visibleWidth() / d->zoom), (int)(visibleHeight() / d->zoom));
    pan->setImage(180, 120, d->im->getImg()->copyQImage());
    pan->setRegionSelection(r);
    pan->setMouseFocus();
    d->panIconPopup->setMainWidget(pan);

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x() + viewport()->size().width());
    g.setY(g.y() + viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(),
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void Canvas::slotPanIconHidden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void Canvas::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*d->zoom), (int)(r.y()*d->zoom));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHidden();
    }
}

void Canvas::slotZoomChanged(double /*zoom*/)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
    {
        d->cornerButton->show();
    }
    else
    {
        d->cornerButton->hide();
    }
}

void Canvas::slotSelectAll()
{
    d->rubber->setRectOnContents(d->pixmapRect);
    d->pressedMoved = true;
    d->tileCache.clear();
    viewport()->setMouseTracking(true);
    viewport()->update();

    if (d->im->imageValid())
    {
        emit signalSelected(true);
    }
}

void Canvas::slotSelectNone()
{
    reset();
    viewport()->update();
}

void Canvas::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    int mult = 1;

    if ((e->modifiers() & Qt::ControlModifier))
    {
        mult = 10;
    }

    switch (e->key())
    {
        case Qt::Key_Right:
        {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Left:
        {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Up:
        {
            verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Down:
        {
            verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep()*mult);
            break;
        }

        default:
        {
            e->ignore();
            break;
        }
    }
}

}  // namespace Digikam
