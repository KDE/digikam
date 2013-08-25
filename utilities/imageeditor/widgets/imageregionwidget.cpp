/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-15
 * Description : a widget to draw an image clip region.
 *
 * Copyright (C) 2013 by Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kiconloader.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "previewtoolbar.h"
#include "previewlayout.h"
#include "imageregionitem.h"

namespace Digikam
{

class ImageRegionWidget::Private
{

public:

    Private() :
        onMouseMovePreviewToggled(true),
        capturePtMode(false),
        renderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        oldRenderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont)
    {
    }

    bool        onMouseMovePreviewToggled; // For PreviewToggleOnMouseOver mode.
    bool        capturePtMode;

    int         renderingPreviewMode;
    int         oldRenderingPreviewMode;

    QPolygon    hightlightPoints;
    
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

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged()));

    connect(this, SIGNAL(signalContentsMovedEvent(bool)),
            this, SLOT(slotOriginalImageRegionChanged(bool)));

    layout()->fitToWindow();

}

ImageRegionWidget::~ImageRegionWidget()
{
    delete d_ptr->item;
    delete d_ptr;
}

void ImageRegionWidget::setHighLightPoints(const QPolygon& pointsList)
{
    d_ptr->item->setHighLightPoints(pointsList);
    update();
}

void ImageRegionWidget::setCapturePointMode(bool b)
{
    d_ptr->capturePtMode = b;
    viewport()->setMouseTracking(b);

    if (b)
    {
        d_ptr->oldRenderingPreviewMode = d_ptr->renderingPreviewMode;
        slotPreviewModeChanged(PreviewToolBar::PreviewOriginalImage);
        viewport()->setCursor(QCursor(SmallIcon("color-picker", 32), 1, 28));
    }
    else
    {
        slotPreviewModeChanged(d_ptr->oldRenderingPreviewMode);
        viewport()->unsetCursor();
    }
}

bool ImageRegionWidget::capturePointMode() const
{
    return d_ptr->capturePtMode;
}

void ImageRegionWidget::slotZoomFactorChanged()
{
    emit signalResized();
    emit signalContentsMovedEvent(true);
}

void ImageRegionWidget::slotPreviewModeChanged(int mode)
{
    d_ptr->renderingPreviewMode = mode;
    d_ptr->item->setRenderingPreviewMode(mode);
    slotZoomFactorChanged();
    update();
}

double ImageRegionWidget::zoomFactor() const
{
    return d_ptr->item->zoomSettings()->zoomFactor();
}

QRect ImageRegionWidget::getOriginalImageRegionToRender() const
{
    QRect r = d_ptr->item->getImageRegion();

    int x = (int)((double)r.x() / zoomFactor());
    int y = (int)((double)r.y() / zoomFactor());
    int w = (int)((double)r.width() / zoomFactor());
    int h = (int)((double)r.height() / zoomFactor());

    QRect rect(x, y, w, h);
    return (rect);
}

void ImageRegionWidget::setPreviewImage(const DImg& img)
{
    DImg image = img;
    QRect r    = d_ptr->item->getImageRegion();
    image.resize(r.width(), r.height());

    // Because image plugins are tool which only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to
    // restore the embedded ICC color profile.
    // However, some plugins may set a profile on the preview image, which we accept of course.
    if (image.getIccProfile().isNull())
    {
        image.setIccProfile(d_ptr->item->image().getIccProfile());
    }
    
    d_ptr->item->setTargetImage(image);
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

void ImageRegionWidget::slotPanIconSelectionMoved(const QRect& rect, bool targetDone)
{
    GraphicsDImgView::slotPanIconSelectionMoved(rect, targetDone);
    //setContentsPosition((int)(rect.x()*zoomFactor()), (int)(rect.y()*zoomFactor()), targetDone);
    scrollContentsBy((int)(rect.x()*zoomFactor()), (int)(rect.y()*zoomFactor()));
}

void ImageRegionWidget::slotOriginalImageRegionChanged(bool targetDone)
{
    if (targetDone)
    {
        emit signalOriginalClipFocusChanged();//For Image Edit Tools
    }
}

void ImageRegionWidget::exposureSettingsChanged()
{
    d_ptr->item->clearCache();
    update();
}

void ImageRegionWidget::ICCSettingsChanged()
{
    d_ptr->item->clearCache();
    update();
}

void ImageRegionWidget::toggleFitToWindow()
{
    layout()->toggleFitToWindow();
    update();
}

void ImageRegionWidget::setZoomFactor(double zoom)
{
    d_ptr->item->zoomSettings()->setZoomFactor(zoom);
}

}  // namespace Digikam
