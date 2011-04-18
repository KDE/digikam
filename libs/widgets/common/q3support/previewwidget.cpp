/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright (C) 2006-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewwidget.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QCache>
#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFrame>
#include <QMouseEvent>
#include <QToolButton>

// KDE includes

#include <kdatetable.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <klocale.h>

// locale includes

#include "paniconwidget.h"

namespace Digikam
{

class PreviewWidget::PreviewWidgetPriv
{
public:

    PreviewWidgetPriv() :
        tileSize(128), zoomMultiplier(1.2)
    {
        currentFitWindowZoom = 0.0;
        panIconPopup         = 0;
        cornerButton         = 0;
        autoZoom             = false;
        fullScreen           = false;
        zoom                 = 1.0;
        minZoom              = 0.1;
        maxZoom              = 12.0;
        zoomWidth            = 0;
        zoomHeight           = 0;
        tileTmpPix           = new QPixmap(tileSize, tileSize);

        tileCache.setMaxCost((10*1024*1024)/(tileSize*tileSize*4));
    }

    bool                     autoZoom;
    bool                     fullScreen;

    const int                tileSize;
    int                      zoomWidth;
    int                      zoomHeight;
    QPoint                   mousePressPos;

    double                   zoom;
    double                   minZoom;
    double                   maxZoom;
    double                   currentFitWindowZoom;
    const double             zoomMultiplier;

    QPoint                   centerZoomPoint;

    QRect                    pixmapRect;

    QCache<QString, QPixmap> tileCache;

    QPixmap*                 tileTmpPix;

    QColor                   bgColor;

    QToolButton*             cornerButton;

    KPopupFrame*             panIconPopup;
};

PreviewWidget::PreviewWidget(QWidget* parent)
    : Q3ScrollView(parent), d(new PreviewWidgetPriv)
{
    m_movingInProgress = false;
    setAttribute(Qt::WA_DeleteOnClose);

    d->bgColor      = palette().color(QPalette::Base);
    d->cornerButton = PanIconWidget::button();
    setCornerWidget(d->cornerButton);

    viewport()->setMouseTracking(false);

    horizontalScrollBar()->setSingleStep(1);
    horizontalScrollBar()->setPageStep(1);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(1);

    setHScrollBarMode(Q3ScrollView::AlwaysOn);
    setVScrollBarMode(Q3ScrollView::AlwaysOn);
    setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    setMargin(0);
    setLineWidth(1);
    setFocusPolicy(Qt::ClickFocus);

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(horizontalSliderPressed()),
            this, SLOT(slotContentTakeFocus()));

    connect(this, SIGNAL(verticalSliderPressed()),
            this, SLOT(slotContentTakeFocus()));

    connect(this, SIGNAL(horizontalSliderReleased()),
            this, SLOT(slotContentLeaveFocus()));

    connect(this, SIGNAL(verticalSliderReleased()),
            this, SLOT(slotContentLeaveFocus()));
}

PreviewWidget::~PreviewWidget()
{
    delete d->tileTmpPix;
    delete d;
}

void PreviewWidget::clearCache()
{
    d->tileCache.clear();
}

void PreviewWidget::slotReset()
{
    clearCache();
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
    if (d->autoZoom)
    {
        d->autoZoom = false;
        emit signalToggleOffFitToWindow();
    }

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
    d->zoom       = round(zoom * 10000.0) / 10000.0;
    d->zoomWidth  = (int)(previewWidth()  * d->zoom);
    d->zoomHeight = (int)(previewHeight() * d->zoom);

    updateContentsSize();

    // adapt step size to zoom factor. Overall, using a finer step size than scrollbar default.
    int step = qMax(2, (int)(2*lround(d->zoom)));
    horizontalScrollBar()->setSingleStep( step );
    horizontalScrollBar()->setPageStep( step * 10 );
    verticalScrollBar()->setSingleStep( step );
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
        d->zoom       = 1.0;
        d->zoomWidth  = (int)(previewWidth());
        d->zoomHeight = (int)(previewHeight());
    }

    updateContentsSize();
    zoomFactorChanged(d->zoom);
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

void PreviewWidget::updateZoomAndSize(bool alwaysFitToWindow)
{
    // Set zoom for fit-in-window as minimum, but don't scale up images
    // that are smaller than the available space, only scale down.
    double zoom = calcAutoZoomFactor(ZoomInOnly);
    setZoomMin(zoom);
    setZoomMax(zoom*12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() < zoom || alwaysFitToWindow || zoomFactor() == d->currentFitWindowZoom)
    {
        setZoomFactor(zoom);
    }

    // store which zoom factor means it is fit to window
    d->currentFitWindowZoom = zoom;

    updateContentsSize();
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
    if (previewIsNull())
    {
        return d->zoom;
    }

    double srcWidth  = previewWidth();
    double srcHeight = previewHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();

    double zoom = qMin(dstWidth/srcWidth, dstHeight/srcHeight);
    // limit precision as above
    zoom = round(zoom * 10000.0) / 10000.0;

    if (mode == ZoomInOrOut)
        // fit to available space, scale up or down
    {
        return zoom;
    }
    else
        // ZoomInOnly: accept that an image is smaller than available space, don't scale up
    {
        return qMin(1.0, zoom);
    }
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
        xoffset     = qMax(xoffset, 0);
        yoffset     = qMax(yoffset, 0);

        d->pixmapRect = QRect(xoffset, yoffset, d->zoomWidth, d->zoomHeight);
    }
    else
    {
        d->pixmapRect = QRect(0, 0, d->zoomWidth, d->zoomHeight);
    }

    clearCache();
    setContentsSize();
    viewport()->setUpdatesEnabled(true);
}

void PreviewWidget::setContentsSize()
{
    resizeContents(d->zoomWidth, d->zoomHeight);
}

void PreviewWidget::resizeEvent(QResizeEvent* e)
{
    if (!e)
    {
        return;
    }

    Q3ScrollView::resizeEvent(e);

    if (d->autoZoom)
    {
        updateAutoZoom();
    }

    updateContentsSize();

    // No need to repaint. its called
    // automatically after resize

    // To be sure than corner widget used to pan image will be hide/show
    // accordingly with resize event.
    zoomFactorChanged(d->zoom);

    emit signalResized();
}

void PreviewWidget::viewportPaintEvent(QPaintEvent* e)
{
    QPainter p(viewport());
    QRect er(e->rect());
    er = QRect(qMax(er.x()      - 1, 0),
               qMax(er.y()      - 1, 0),
               qMin(er.width()  + 2, contentsRect().width()),
               qMin(er.height() + 2, contentsRect().height()));

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

                p.drawPixmap(pt.x(), pt.y(),
                             *pix,
                             ir.x()-r.x(), ir.y()-r.y(),
                             ir.width(), ir.height());
            }
        }
    }

    p.setClipRegion(clipRegion);
    p.fillRect(er, d->bgColor);
    p.end();

    viewportPaintExtraData();
}

void PreviewWidget::drawText(QPainter* p, const QPoint& corner, const QString& text)
{
    p->save();

    QFontMetrics fontMt = p->fontMetrics();
    QRect fontRect      = fontMt.boundingRect(text);
    QRect textRect;
    textRect.setTopLeft(corner);
    textRect.setSize( QSize(fontRect.width()+5, fontRect.height()+2) );

    // Draw background
    p->setPen(Qt::black);
    QColor semiTransBg = palette().color(QPalette::Window);
    semiTransBg.setAlpha(190);
    p->setBrush(semiTransBg);
    //p->translate(0.5, 0.5);
    p->drawRoundRect(textRect, 10, 10);

    // Draw shadow and text
    p->setPen(palette().color(QPalette::Window).dark(115));
    p->drawText(textRect.translated(3, 1), text);
    p->setPen(palette().color(QPalette::WindowText));
    p->drawText(textRect.translated(2, 0), text);

    p->restore();
}

void PreviewWidget::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
    if (!e || e->button() == Qt::RightButton)
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        emit signalLeftButtonDoubleClicked();

        if (!KGlobalSettings::singleClick())
        {
            emit signalActivated();
        }
    }
}

void PreviewWidget::startPanning(const QPoint& pos)
{
    if (visibleWidth()  < d->zoomWidth ||
        visibleHeight() < d->zoomHeight)
    {
        m_movingInProgress = true;
        d->mousePressPos   = pos;
        viewport()->setCursor(Qt::SizeAllCursor);
    }
}

void PreviewWidget::continuePanning(const QPoint& pos)
{
    scrollBy(d->mousePressPos.x() - pos.x(), d->mousePressPos.y() - pos.y());
    emit signalContentsMovedEvent(false);
    viewport()->update();
}

void PreviewWidget::finishPanning()
{
    emit signalContentsMovedEvent(true);
    viewport()->unsetCursor();
}

void PreviewWidget::contentsMousePressEvent(QMouseEvent* e)
{
    if (!e || e->button() == Qt::RightButton)
    {
        return;
    }

    m_movingInProgress = false;

    if (e->button() == Qt::LeftButton || e->button() == Qt::MidButton)
    {
        d->mousePressPos = e->pos();

        if (!KGlobalSettings::singleClick() || e->button() == Qt::MidButton)
        {
            startPanning(e->pos());
        }

        return;
    }

    viewport()->setMouseTracking(false);
}

void PreviewWidget::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (((e->buttons() & Qt::LeftButton) || (e->buttons() & Qt::MidButton)) && !d->mousePressPos.isNull())
    {
        if (!m_movingInProgress && (e->buttons() & Qt::LeftButton))
        {
            if ((d->mousePressPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
            {
                startPanning(d->mousePressPos);
            }
        }

        if (m_movingInProgress)
        {
            continuePanning(e->pos());
        }
    }
}

void PreviewWidget::contentsMouseReleaseEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if ((e->button() == Qt::LeftButton || e->button() == Qt::MidButton) && !d->mousePressPos.isNull())
    {
        if (!m_movingInProgress && e->button() == Qt::LeftButton)
        {
            emit signalLeftButtonClicked();

            if (KGlobalSettings::singleClick())
            {
                emit signalActivated();
            }
        }
        else
        {
            finishPanning();
        }
    }

    if (e->button() == Qt::RightButton)
    {
        emit signalRightButtonClicked();
    }

    m_movingInProgress = false;
    d->mousePressPos   = QPoint();
}

void PreviewWidget::contentsWheelEvent(QWheelEvent* e)
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
        // When zooming with the mouse-wheel, the image center is kept fixed.
        d->centerZoomPoint = e->pos();

        if (e->delta() < 0 && !minZoom())
        {
            slotDecreaseZoom();
        }
        else if (e->delta() > 0 && !maxZoom())
        {
            slotIncreaseZoom();
        }

        d->centerZoomPoint = QPoint();
        return;
    }

    Q3ScrollView::contentsWheelEvent(e);
}

void PreviewWidget::zoomFactorChanged(double zoom)
{
    emit signalZoomFactorChanged(zoom);
}

void PreviewWidget::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget* pan = new PanIconWidget(d->panIconPopup);

    connect(pan, SIGNAL(signalSelectionTakeFocus()),
            this, SIGNAL(signalContentTakeFocus()));

    connect(pan, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotPanIconSelectionMoved(const QRect&, bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHiden()));

    QRect r((int)(contentsX()    / zoomFactor()), (int)(contentsY()     / zoomFactor()),
            (int)(visibleWidth() / zoomFactor()), (int)(visibleHeight() / zoomFactor()));
    pan->setImage(180, 120, previewToQImage());
    pan->setRegionSelection(r);
    pan->setMouseFocus();
    d->panIconPopup->setMainWidget(pan);
    slotContentTakeFocus();

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

void PreviewWidget::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHiden();
        slotContentLeaveFocus();
    }
}

void PreviewWidget::slotContentTakeFocus()
{
    m_movingInProgress = true;
    viewport()->repaint();
}

void PreviewWidget::slotContentLeaveFocus()
{
    m_movingInProgress = false;
    viewport()->repaint();
}

void PreviewWidget::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    int mult = 1;

    if ( (e->modifiers() & Qt::ControlModifier))
    {
        mult = 10;
    }

    switch ( e->key() )
    {
        case Qt::Key_Right:
        {
            slotContentTakeFocus();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Left:
        {
            slotContentTakeFocus();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Up:
        {
            slotContentTakeFocus();
            verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Down:
        {
            slotContentTakeFocus();
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

void PreviewWidget::keyReleaseEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    switch ( e->key() )
    {
        case Qt::Key_Right:
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            slotContentLeaveFocus();
            break;
        }

        default:
        {
            e->ignore();
            break;
        }
    }
}

void PreviewWidget::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
        return;

    d->bgColor = color;
    viewport()->update();
}

}  // namespace Digikam
