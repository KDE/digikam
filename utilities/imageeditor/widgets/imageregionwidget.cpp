/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-17
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageregionwidget.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QCursor>
#include <QPainter>
#include <QPen>
#include <QImage>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// KDE includes

#include <kiconloader.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imageiface.h"
#include "previewtoolbar.h"

namespace Digikam
{

class ImageRegionWidget::ImageRegionWidgetPriv
{

public:

    ImageRegionWidgetPriv() :
        onMouseMovePreviewToggled(true),
        capturePtMode(false),
        renderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        oldRenderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        xpos(0),
        ypos(0),
        iface(0)
    {
    }

    bool        onMouseMovePreviewToggled;
    bool        capturePtMode;

    int         renderingPreviewMode;
    int         oldRenderingPreviewMode;
    int         xpos;
    int         ypos;

    QPixmap     pixmapRegion;          // Pixmap of current region to render.

    QPolygon    hightlightPoints;

    DImg        image;                 // Entire content image to render pixmap.

    ImageIface* iface;
};

ImageRegionWidget::ImageRegionWidget(QWidget* parent)
    : PreviewWidget(parent), d(new ImageRegionWidgetPriv)
{
    d->iface = new ImageIface(0, 0);
    d->image = d->iface->getOriginalImg()->copy();

    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setMinimumSize(480, 320);
    setWhatsThis(i18n("<p>Here you can see the original clip image "
                      "which will be used for the preview computation.</p>"
                      "<p>Click and drag the mouse cursor in the "
                      "image to change the clip focus.</p>"));

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged()));

    connect(this, SIGNAL(signalContentTakeFocus()),
            this, SLOT(slotContentTakeFocus()));

    connect(this, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));
}

ImageRegionWidget::~ImageRegionWidget()
{
    delete d->iface;
    delete d;
}

int ImageRegionWidget::previewWidth()
{
    return d->image.width();
}

int ImageRegionWidget::previewHeight()
{
    return d->image.height();
}

bool ImageRegionWidget::previewIsNull()
{
    return d->image.isNull();
}

void ImageRegionWidget::resetPreview()
{
    d->image.reset();
}

void ImageRegionWidget::paintPreview(QPixmap* pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->image.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = d->iface->convertToPixmap(img);
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2, 0, 0, pix2.width(), pix2.height());
    p.end();
}

void ImageRegionWidget::setHighLightPoints(const QPolygon& pointsList)
{
    d->hightlightPoints = pointsList;
    repaintContents(false);
}

void ImageRegionWidget::setCapturePointMode(bool b)
{
    d->capturePtMode = b;
    viewport()->setMouseTracking(b);

    if (b)
    {
        d->oldRenderingPreviewMode = d->renderingPreviewMode;
        slotPreviewModeChanged(PreviewToolBar::PreviewOriginalImage);
        viewport()->setCursor(QCursor(SmallIcon("color-picker", 32), 1, 28));
    }
    else
    {
        slotPreviewModeChanged(d->oldRenderingPreviewMode);
        viewport()->unsetCursor();
    }
}

bool ImageRegionWidget::capturePointMode() const
{
    return d->capturePtMode;
}

void ImageRegionWidget::slotZoomFactorChanged()
{
    emit signalContentsMovedEvent(true);
}

void ImageRegionWidget::slotPreviewModeChanged(int mode)
{
    d->renderingPreviewMode = mode;
    updateContentsSize();
    slotZoomFactorChanged();
}

void ImageRegionWidget::viewportPaintExtraData()
{
    if (!m_movingInProgress && !d->pixmapRegion.isNull())
    {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBackgroundMode(Qt::TransparentMode);

        QRect region;

        // Original region.
        region = getLocalImageRegionToRender();
        QRect ro(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));

        // Target region.
        QRect rt(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));

        p.translate(previewRect().topLeft());

        // Drawing separate view.

        if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
            (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d->onMouseMovePreviewToggled))
        {
            drawText(&p, QPoint(rt.topLeft().x() + 20, rt.topLeft().y() + 20), i18n("Before"));
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                 d->renderingPreviewMode == PreviewToolBar::NoPreviewMode      ||
                 (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled))
        {
            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
            {
                drawText(&p, QPoint(rt.topLeft().x() + 20, rt.topLeft().y() + 20), i18n("After"));
            }
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
        {
            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
            {
                rt.translate(rt.width(), 0);
            }

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
            {
                ro.translate(-ro.width(), 0);
            }

            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
            p.drawLine(rt.topLeft().x(), rt.topLeft().y(), rt.bottomLeft().x(), rt.bottomLeft().y());
            p.setPen(QPen(Qt::red, 2, Qt::DotLine));
            p.drawLine(rt.topLeft().x(), rt.topLeft().y() + 1, rt.bottomLeft().x(), rt.bottomLeft().y() - 1);

            drawText(&p, QPoint(ro.topLeft().x() + 20, ro.topLeft().y() + 20), i18n("Before"));
            drawText(&p, QPoint(rt.topLeft().x() + 20, rt.topLeft().y() + 20), i18n("After"));
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
        {
            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
            {
                rt.translate(0, rt.height());
            }

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
            {
                ro.translate(0, -ro.height());
            }

            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
            p.drawLine(rt.topLeft().x() + 1, rt.topLeft().y(), rt.topRight().x() - 1, rt.topRight().y());
            p.setPen(QPen(Qt::red, 2, Qt::DotLine));
            p.drawLine(rt.topLeft().x()  , rt.topLeft().y(), rt.topRight().x()  , rt.topRight().y());

            drawText(&p, QPoint(ro.topLeft().x() + 20, ro.topLeft().y() + 20), i18n("Before"));
            drawText(&p, QPoint(rt.topLeft().x() + 20, rt.topLeft().y() + 20), i18n("After"));
        }

        // Drawing highlighted points.

        if (!d->hightlightPoints.isEmpty())
        {
            QPoint pt;
            QRect  hpArea;

            for (int i = 0 ; i < d->hightlightPoints.count() ; ++i)
            {
                pt = d->hightlightPoints.point(i);

                if (getOriginalImageRegionToRender().contains(pt))
                {
                    int x = (int)(((double)pt.x() * tileSize()) / floor(tileSize() / zoomFactor()));
                    int y = (int)(((double)pt.y() * tileSize()) / floor(tileSize() / zoomFactor()));

                    QPoint hp(contentsToViewport(QPoint(x, y)));
                    hpArea.setSize(QSize((int)(16 * zoomFactor()), (int)(16 * zoomFactor())));
                    hpArea.moveCenter(hp);

                    p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
                    p.drawLine(hp.x(), hpArea.y(), hp.x(), hp.y() - (int)(3 * zoomFactor()));
                    p.drawLine(hp.x(), hp.y() + (int)(3 * zoomFactor()), hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(), hp.y(), hp.x() - (int)(3 * zoomFactor()), hp.y());
                    p.drawLine(hp.x() + (int)(3 * zoomFactor()), hp.y(), hpArea.right(), hp.y());

                    p.setPen(QPen(Qt::red, 2, Qt::DotLine));
                    p.drawLine(hp.x(), hpArea.y(), hp.x(), hp.y() - (int)(3 * zoomFactor()));
                    p.drawLine(hp.x(), hp.y() + (int)(3 * zoomFactor()), hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(), hp.y(), hp.x() - (int)(3 * zoomFactor()), hp.y());
                    p.drawLine(hp.x() + (int)(3 * zoomFactor()), hp.y(), hpArea.right(), hp.y());
                }
            }
        }

        p.end();
    }
}

QRect ImageRegionWidget::getLocalImageRegionToRender() const
{
    QRect region;
    QRect pr = previewRect();
    int pX   = pr.x();
    int pY   = pr.y();
    int pW   = visibleWidth() > pr.width() ? pr.width() : visibleWidth();
    int pH   = visibleHeight() > pr.height() ? pr.height() : visibleHeight();

    if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
    {
        region = QRect((int)ceilf(contentsX() - pX + visibleWidth() / 2.0),
                       contentsY(),
                       (int)ceilf(pW / 2.0),
                       pH);
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
    {
        region = QRect(contentsX(),
                       contentsY(),
                       (int)ceilf(pW / 2.0),
                       pH);
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
    {
        region = QRect(contentsX(),
                       (int)ceilf(contentsY() - pY + visibleHeight() / 2.0),
                       pW,
                       (int)ceilf(pH / 2.0));
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
    {
        region = QRect(contentsX(),
                       contentsY(),
                       pW,
                       (int)ceilf(pH / 2.0));
    }
    else
    {
        region = QRect(contentsX(),
                       contentsY(),
                       pW,
                       pH);
    }

    return (region);
}

QRect ImageRegionWidget::getOriginalImageRegion() const
{
    QRect region;

    switch (d->renderingPreviewMode)
    {
        case PreviewToolBar::PreviewOriginalImage:
        case PreviewToolBar::PreviewTargetImage:
        case PreviewToolBar::PreviewBothImagesVert:
        case PreviewToolBar::PreviewBothImagesHorz:
        case PreviewToolBar::PreviewToggleOnMouseOver:
            region = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());
            break;

        case PreviewToolBar::PreviewBothImagesVertCont:
            region = QRect(contentsX(), contentsY(), visibleWidth() / 2, visibleHeight());
            break;

        case PreviewToolBar::PreviewBothImagesHorzCont:
            region = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight() / 2);
            break;
    }

    return region;
}

QRect ImageRegionWidget::getOriginalImageRegionToRender() const
{
    QRect r = getLocalImageRegionToRender();

    int x = (int)(((double)r.x()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int y = (int)(((double)r.y()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int w = (int)(((double)r.width()  / tileSize()) * floor(tileSize() / zoomFactor()));
    int h = (int)(((double)r.height() / tileSize()) * floor(tileSize() / zoomFactor()));

    QRect rect(x, y, w, h);
    return (rect);
}

void ImageRegionWidget::setCenterImageRegionPosition()
{
    center(contentsWidth() / 2, contentsHeight() / 2);
    slotZoomFactorChanged();
}

void ImageRegionWidget::setContentsPosition(int x, int y, bool targetDone)
{
    if (targetDone)
    {
        m_movingInProgress = false;
    }

    setContentsPos(x, y);

    if (targetDone)
    {
        slotZoomFactorChanged();
    }
}

void ImageRegionWidget::backupPixmapRegion()
{
    d->pixmapRegion = QPixmap();
}

void ImageRegionWidget::restorePixmapRegion()
{
    m_movingInProgress = true;
    viewport()->repaint();
}

void ImageRegionWidget::setPreviewImage(const DImg& img)
{
    DImg image = img;
    QRect r    = getLocalImageRegionToRender();
    image.resize(r.width(), r.height());

    // Because image plugins are tool witch only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to
    // restore the embedded ICC color profile.
    // However, some plugins may set a profile on the preview image, which we accept of course.
    if (image.getIccProfile().isNull())
    {
        image.setIccProfile(d->image.getIccProfile());
    }

    d->pixmapRegion = d->iface->convertToPixmap(image);
    repaintContents(false);
}

DImg ImageRegionWidget::getOriginalRegionImage(bool useDownscaledImage) const
{
    DImg image = d->image.copy(getOriginalImageRegionToRender());

    if (useDownscaledImage)
    {
        QRect r = getLocalImageRegionToRender();
        image.resize(r.width(), r.height());
    }

    return (image);
}

QImage ImageRegionWidget::previewToQImage() const
{
    return d->image.copyQImage();
}

void ImageRegionWidget::slotPanIconSelectionMoved(const QRect& rect, bool targetDone)
{
    PreviewWidget::slotPanIconSelectionMoved(rect, targetDone);
    setContentsPosition((int)(rect.x()*zoomFactor()), (int)(rect.y()*zoomFactor()), targetDone);
}

void ImageRegionWidget::slotContentTakeFocus()
{
    PreviewWidget::slotContentTakeFocus();
    restorePixmapRegion();
}

void ImageRegionWidget::slotContentLeaveFocus()
{
    PreviewWidget::slotContentLeaveFocus();
    setContentsPosition(contentsX(), contentsY(), true);
}

void ImageRegionWidget::slotOriginalImageRegionChanged(bool targetDone)
{
    if (targetDone)
    {
        backupPixmapRegion();
        emit signalOriginalClipFocusChanged();
    }
}

void ImageRegionWidget::exposureSettingsChanged()
{
    clearCache();
    viewport()->update();
}

void ImageRegionWidget::ICCSettingsChanged()
{
    clearCache();
    viewport()->update();
}

void ImageRegionWidget::enterEvent(QEvent*)
{
    if (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
    {
        d->onMouseMovePreviewToggled = false;
        viewport()->repaint();
    }
}

void ImageRegionWidget::leaveEvent(QEvent*)
{
    if (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
    {
        d->onMouseMovePreviewToggled = true;
        viewport()->repaint();
    }
}

void ImageRegionWidget::contentsMousePressEvent(QMouseEvent* e)
{
    if (d->capturePtMode)
    {
        QRect  region = getLocalImageRegionToRender();
        // Original region.
        QRect  ro(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));
        // Target region.
        QRect rt(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));
        QPoint tl = previewRect().topLeft();
        ro.translate(tl);
        rt.translate(tl);

        QPoint pt(contentsToViewport(e->pos()));

        // Drawing separate view.

        if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
            (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d->onMouseMovePreviewToggled))
        {
            if (ro.contains(pt))
            {
                emitCapturedPointFromOriginal(pt - ro.topLeft());
            }
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
        {
            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
            {
                rt.translate(rt.width(), 0);
            }

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
            {
                ro.translate(-ro.width(), 0);
            }

            if (!rt.contains(pt) && ro.contains(pt))
            {
                emitCapturedPointFromOriginal(pt - ro.topLeft());
            }
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
        {
            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
            {
                rt.translate(0, rt.height());
            }

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
            {
                ro.translate(0, -ro.height());
            }

            if (!rt.contains(pt) && ro.contains(pt))
            {
                emitCapturedPointFromOriginal(pt - ro.topLeft());
            }
        }

        return;
    }

    PreviewWidget::contentsMousePressEvent(e);
}

void ImageRegionWidget::emitCapturedPointFromOriginal(const QPoint& pt)
{
    int x = (int)(((double)pt.x() / tileSize()) * floor(tileSize() / zoomFactor()));
    int y = (int)(((double)pt.y() / tileSize()) * floor(tileSize() / zoomFactor()));
    QPoint imgPt(x, y);
    DColor color = d->image.getPixelColor(x, y);
    kDebug() << "Captured point from image : " << imgPt;
    emit signalCapturedPointFromOriginal(color, imgPt);
}

void ImageRegionWidget::contentsMouseReleaseEvent(QMouseEvent* e)
{
    if (d->capturePtMode)
    {
        setCapturePointMode(false);
        return;
    }

    PreviewWidget::contentsMouseReleaseEvent(e);
}

}  // namespace Digikam
