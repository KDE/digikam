/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-15
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageregionwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "imageregionitem.h"
#include "previewtoolbar.h"
#include "previewlayout.h"
#include "dimgitemspriv.h"

namespace Digikam
{

class ImageRegionWidget::Private
{

public:

    Private() :
        capturePtMode(false),
        renderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        oldRenderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        delay(0),
        item(0)
    {
    }

    bool             capturePtMode;

    int              renderingPreviewMode;
    int              oldRenderingPreviewMode;

    QPolygon         hightlightPoints;

    QTimer*          delay;

    ImageRegionItem* item;
};

ImageRegionWidget::ImageRegionWidget(QWidget* const parent)
    : GraphicsDImgView(parent), d_ptr(new Private)
{
    d_ptr->item = new ImageRegionItem(this);
    setItem(d_ptr->item);

    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setMinimumSize(480, 320);
    setWhatsThis(i18n("<p>Here you can see the original clip image "
                      "which will be used for the preview computation.</p>"
                      "<p>Click and drag the mouse cursor in the "
                      "image to change the clip focus.</p>"));

    d_ptr->delay = new QTimer(this);
    d_ptr->delay->setInterval(500);
    d_ptr->delay->setSingleShot(true);

    connect(d_ptr->delay, SIGNAL(timeout()),
            this, SLOT(slotOriginalImageRegionChanged()));

    connect(this, SIGNAL(viewportRectChanged(const QRectF&)),
            this, SLOT(slotOriginalImageRegionChangedDelayed()));

    layout()->fitToWindow();
    installPanIcon();
}

ImageRegionWidget::~ImageRegionWidget()
{
    delete d_ptr->item;
    delete d_ptr;
}

void ImageRegionWidget::setHighLightPoints(const QPolygon& pointsList)
{
    d_ptr->item->setHighLightPoints(pointsList);
    viewport()->update();
}

void ImageRegionWidget::setCapturePointMode(bool b)
{
    if (d_ptr->capturePtMode && b)
    {
        return;
    }

    d_ptr->capturePtMode = b;
    viewport()->setMouseTracking(!b);

    if (b)
    {
        d_ptr->oldRenderingPreviewMode = d_ptr->renderingPreviewMode;
        slotPreviewModeChanged(PreviewToolBar::PreviewOriginalImage);
        viewport()->setCursor(QCursor(QIcon::fromTheme(QLatin1String("color-picker")).pixmap(32), 1, 28));
    }
    else
    {
        slotPreviewModeChanged(d_ptr->oldRenderingPreviewMode);
        viewport()->unsetCursor();
    }
}

void ImageRegionWidget::slotPreviewModeChanged(int mode)
{
    d_ptr->renderingPreviewMode = mode;
    d_ptr->item->setRenderingPreviewMode(mode);
    slotOriginalImageRegionChanged();
    viewport()->update();
}

QRect ImageRegionWidget::getOriginalImageRegionToRender() const
{
    QRect  r = d_ptr->item->getImageRegion();
    double z = layout()->zoomFactor();

    int x    = qRound((double)r.x()      / z);
    int y    = qRound((double)r.y()      / z);
    int w    = qRound((double)r.width()  / z);
    int h    = qRound((double)r.height() / z);

    QRect rect(x, y, w, h);
    return (rect);
}

void ImageRegionWidget::setPreviewImage(const DImg& img)
{
    DImg image = img;
    QRect r    = d_ptr->item->getImageRegion();
    image.resize(r.width(), r.height());

    // Because tool which only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to
    // restore the embedded ICC color profile.
    // However, some tools may set a profile on the preview image, which we accept of course.
    if (image.getIccProfile().isNull())
    {
        image.setIccProfile(d_ptr->item->image().getIccProfile());
    }

    d_ptr->item->setTargetImage(image);
}

DImg ImageRegionWidget::getOriginalImage() const
{
    return (d_ptr->item->image().copy());
}

DImg ImageRegionWidget::getOriginalRegionImage(bool useDownscaledImage) const
{
    DImg image = d_ptr->item->image().copy(getOriginalImageRegionToRender());

    if (useDownscaledImage)
    {
        QRect r = d_ptr->item->getImageRegion();
        image.resize(r.width(), r.height());
    }

    return (image);
}

void ImageRegionWidget::slotOriginalImageRegionChangedDelayed()
{
    viewport()->update();
    d_ptr->delay->start();
}

void ImageRegionWidget::slotOriginalImageRegionChanged(bool targetDone)
{
    if (targetDone)
    {
        emit signalOriginalClipFocusChanged(); // For Image Edit Tools
    }
}

void ImageRegionWidget::exposureSettingsChanged()
{
    d_ptr->item->clearCache();
    viewport()->update();
}

void ImageRegionWidget::ICCSettingsChanged()
{
    d_ptr->item->clearCache();
    viewport()->update();
}

void ImageRegionWidget::mousePressEvent(QMouseEvent* e)
{
    if (d_ptr->capturePtMode)
    {
        emitCapturedPointFromOriginal(mapToScene(e->pos()));
        QGraphicsView::mousePressEvent(e);
        return;
    }

    GraphicsDImgView::mousePressEvent(e);
}

void ImageRegionWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d_ptr->capturePtMode)
    {
        setCapturePointMode(false);
        QGraphicsView::mouseReleaseEvent(e);
        return;
    }

    GraphicsDImgView::mouseReleaseEvent(e);
}

void ImageRegionWidget::emitCapturedPointFromOriginal(const QPointF& pt)
{
    int x        = (int)(pt.x() / layout()->zoomFactor());
    int y        = (int)(pt.y() / layout()->zoomFactor());
    QPoint imgPt(x, y);
    DColor color = d_ptr->item->image().getPixelColor(x, y);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Captured point from image : " << imgPt;
    emit signalCapturedPointFromOriginal(color, imgPt);
}

}  // namespace Digikam
