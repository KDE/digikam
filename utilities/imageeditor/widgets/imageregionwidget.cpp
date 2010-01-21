/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-17
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QPainter>
#include <QPen>
#include <QImage>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// KDE includes

#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imageiface.h"
#include "previewtoolbar.h"

namespace Digikam
{

class ImageRegionWidgetPriv
{

public:

    ImageRegionWidgetPriv()
    {
        onMouseMovePreviewToggled = true;
        iface                     = 0;
        renderingPreviewMode      = PreviewToolBar::PreviewBothImagesVertCont;
    }

    bool        onMouseMovePreviewToggled;

    int         renderingPreviewMode;
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

    connect(this, SIGNAL(signalSelectionTakeFocus()),
            this, SLOT(slotSelectionTakeFocus()));

    connect(this, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    connect(this, SIGNAL(horizontalSliderPressed()),
            this, SLOT(slotSelectionTakeFocus()));

    connect(this, SIGNAL(verticalSliderPressed()),
            this, SLOT(slotSelectionTakeFocus()));

    connect(this, SIGNAL(horizontalSliderReleased()),
            this, SLOT(slotSelectionLeaveFocus()));

    connect(this, SIGNAL(verticalSliderReleased()),
            this, SLOT(slotSelectionLeaveFocus()));
}

ImageRegionWidget::~ImageRegionWidget()
{
    if (d->iface) delete d->iface;
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

void ImageRegionWidget::drawText(QPainter* p, const QRect& rect, const QString& text)
{
    p->save();

    // Draw background
    p->setPen(Qt::black);
    QColor semiTransBg = palette().color(QPalette::Window);
    semiTransBg.setAlpha(190);
    p->setBrush(semiTransBg);
    p->translate(0.5, 0.5);
    p->drawRoundRect(rect, 10.0, 10.0);

    // Draw shadow and text
    p->setPen(palette().color(QPalette::Window).dark(115));
    p->drawText(rect.translated(1, 1), text);
    p->setPen(palette().color(QPalette::WindowText));
    p->drawText(rect, text);

    p->restore();
}

void ImageRegionWidget::viewportPaintExtraData()
{
    if (!m_movingInProgress && !d->pixmapRegion.isNull())
    {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBackgroundMode(Qt::TransparentMode);
        QFontMetrics fontMt = p.fontMetrics();

        QString text;
        QRect textRect, fontRect;

        // Target region.
        QRect region = getLocalTargetImageRegion();
        QRect rt(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));

        // Original region.
        region = getLocalImageRegionToRender();
        QRect ro(contentsToViewport(region.topLeft()), contentsToViewport(region.bottomRight()));

        p.translate(previewRect().topLeft());

        // Drawing separate view.

        if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
            (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d->onMouseMovePreviewToggled))
        {
            text     = i18n("Original");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
            textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
            drawText(&p, textRect, text);
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                 d->renderingPreviewMode == PreviewToolBar::NoPreviewMode      ||
                 (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled))
        {
            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
            {
                text     = i18n("Target");
                fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
                textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
                textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
                drawText(&p, textRect, text);
            }
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
        {
            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
            p.drawLine(rt.topLeft().x(), rt.topLeft().y(), rt.bottomLeft().x(), rt.bottomLeft().y());
            p.setPen(QPen(Qt::red, 2, Qt::DotLine));
            p.drawLine(rt.topLeft().x(), rt.topLeft().y()+1, rt.bottomLeft().x(), rt.bottomLeft().y()-1);

            text     = i18n("Target");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
            textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
            drawText(&p, textRect, text);

            text     = i18n("Original");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
                ro.translate(-ro.width(), 0);

            textRect.setTopLeft(QPoint(ro.topLeft().x()+20, ro.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
            drawText(&p, textRect, text);
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz ||
                 d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
        {
            p.drawPixmap(rt.x(), rt.y(), d->pixmapRegion, 0, 0, rt.width(), rt.height());

            p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
            p.drawLine(rt.topLeft().x()+1, rt.topLeft().y(), rt.topRight().x()-1, rt.topRight().y());
            p.setPen(QPen(Qt::red, 2, Qt::DotLine));
            p.drawLine(rt.topLeft().x(), rt.topLeft().y(), rt.topRight().x(), rt.topRight().y());

            text     = i18n("Target");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
            textRect.setTopLeft(QPoint(rt.topLeft().x()+20, rt.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
            drawText(&p, textRect, text);

            text     = i18n("Original");
            fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);

            if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
                ro.translate(0, -ro.height());

            textRect.setTopLeft(QPoint(ro.topLeft().x()+20, ro.topLeft().y()+20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
            drawText(&p, textRect, text);
        }

        // Drawing highlighted points.

        if (!d->hightlightPoints.isEmpty())
        {
            QPoint pt;
            QRect  hpArea;

            for (int i = 0 ; i < d->hightlightPoints.count() ; ++i)
            {
                pt = d->hightlightPoints.point(i);

                if ( getOriginalImageRegionToRender().contains(pt) )
                {
                    int x = (int)(((double)pt.x() * tileSize()) / floor(tileSize() / zoomFactor()));
                    int y = (int)(((double)pt.y() * tileSize()) / floor(tileSize() / zoomFactor()));

                    QPoint hp(contentsToViewport(QPoint(x, y)));
                    hpArea.setSize(QSize((int)(16*zoomFactor()), (int)(16*zoomFactor())));
                    hpArea.moveCenter(hp);

                    p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
                    p.drawLine(hp.x(), hpArea.y(), hp.x(), hp.y()-(int)(3*zoomFactor()));
                    p.drawLine(hp.x(), hp.y()+(int)(3*zoomFactor()), hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(), hp.y(), hp.x()-(int)(3*zoomFactor()), hp.y());
                    p.drawLine(hp.x()+(int)(3*zoomFactor()), hp.y(), hpArea.right(), hp.y());

                    p.setPen(QPen(Qt::red, 2, Qt::DotLine));
                    p.drawLine(hp.x(), hpArea.y(), hp.x(), hp.y()-(int)(3*zoomFactor()));
                    p.drawLine(hp.x(), hp.y()+(int)(3*zoomFactor()), hp.x(), hpArea.bottom());
                    p.drawLine(hpArea.x(), hp.y(), hp.x()-(int)(3*zoomFactor()), hp.y());
                    p.drawLine(hp.x()+(int)(3*zoomFactor()), hp.y(), hpArea.right(), hp.y());
                }
            }
        }

        p.end();
    }
}

QRect ImageRegionWidget::getLocalImageRegionToRender()
{
    QRect region;
    QRect pr = previewRect();
    int pX   = pr.x();
    int pY   = pr.y();
    int pW   = visibleWidth() > pr.width() ? pr.width() : visibleWidth();
    int pH   = visibleHeight() > pr.height() ? pr.height() : visibleHeight();

    if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
    {
        region = QRect((int)ceilf(contentsX() -pX + visibleWidth()/2.0),
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
                       (int)ceilf(contentsY() - pY + visibleHeight()/2.0),
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

QRect ImageRegionWidget::getOriginalImageRegion()
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
            region = QRect(contentsX(), contentsY(), visibleWidth()/2, visibleHeight());
            break;
        case PreviewToolBar::PreviewBothImagesHorzCont:
            region = QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()/2);
            break;
    }

    return region;
}

QRect ImageRegionWidget::getOriginalImageRegionToRender()
{
    QRect r = getLocalImageRegionToRender();

    int x = (int)(((double)r.x()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int y = (int)(((double)r.y()      / tileSize()) * floor(tileSize() / zoomFactor()));
    int w = (int)(((double)r.width()  / tileSize()) * floor(tileSize() / zoomFactor()));
    int h = (int)(((double)r.height() / tileSize()) * floor(tileSize() / zoomFactor()));

    QRect rect(x, y, w, h);
    return (rect);
}

QRect ImageRegionWidget::getLocalTargetImageRegion()
{
    QRect region = getLocalImageRegionToRender();

    if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
        region.translate(region.width(), 0);
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
        region.translate(0, region.height());

    return region;
}

void ImageRegionWidget::setCenterImageRegionPosition()
{
    center(contentsWidth()/2, contentsHeight()/2);
    slotZoomFactorChanged();
}

void ImageRegionWidget::setContentsPosition(int x, int y, bool targetDone)
{
    if( targetDone )
        m_movingInProgress = false;

    setContentsPos(x, y);

    if( targetDone )
       slotZoomFactorChanged();
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
        image.setIccProfile(d->image.getIccProfile());

    d->pixmapRegion = d->iface->convertToPixmap(image);
    repaintContents(false);
}

DImg ImageRegionWidget::getOriginalRegionImage()
{
    return (d->image.copy(getOriginalImageRegionToRender()));
}

void ImageRegionWidget::setContentsSize()
{
PreviewWidget::setContentsSize();
return;

    switch (d->renderingPreviewMode)
    {
        case PreviewToolBar::PreviewOriginalImage:
        case PreviewToolBar::PreviewBothImagesVertCont:
        case PreviewToolBar::PreviewBothImagesHorzCont:
        case PreviewToolBar::PreviewTargetImage:
        case PreviewToolBar::PreviewToggleOnMouseOver:
        {
            PreviewWidget::setContentsSize();
            break;
        }
        case PreviewToolBar::PreviewBothImagesVert:
        {
            resizeContents(zoomWidth()+visibleWidth()/2, zoomHeight());
            break;
        }
        case PreviewToolBar::PreviewBothImagesHorz:
        {
            resizeContents(zoomWidth(), zoomHeight()+visibleHeight()/2);
            break;
        }
        default:
        {
            kWarning() << "Unknown separation view specified";
        }
    }
}

void ImageRegionWidget::contentsWheelEvent(QWheelEvent* e)
{
    e->accept();

    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->delta() < 0 && !minZoom())
            slotDecreaseZoom();
        else if (e->delta() > 0 && !maxZoom())
            slotIncreaseZoom();
        return;
    }
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

void ImageRegionWidget::slotSelectionTakeFocus()
{
    restorePixmapRegion();
}

void ImageRegionWidget::slotSelectionLeaveFocus()
{
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

}  // namespace Digikam
