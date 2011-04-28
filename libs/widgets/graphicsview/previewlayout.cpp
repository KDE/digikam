/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Layout for an item on image preview
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "previewlayout.moc"

// Qt includes

#include <QGraphicsView>
#include <QGraphicsScene>

// KDE includes

#include <kdebug.h>

// Local includes

#include "graphicsdimgitem.h"
#include "graphicsdimgview.h"
#include "imagezoomsettings.h"

namespace Digikam
{

class SinglePhotoPreviewLayout::SinglePhotoPreviewLayoutPriv
{
public:

    SinglePhotoPreviewLayoutPriv()
    {
        view           = 0;
        item           = 0;
        isFitToWindow  = false;
        previousZoom   = 1;
        zoomMultiplier = 1.2;
        minZoom        = 0.1;
        maxZoom        = 12.0;
    }

    GraphicsDImgView* view;
    GraphicsDImgItem* item;

    bool              isFitToWindow;
    double            previousZoom;

    double            zoomMultiplier;
    double            maxZoom;
    double            minZoom;

public:

    ImageZoomSettings* zoomSettings() const
    {
        return item->zoomSettings();
    }

    QSizeF frameSize() const
    {
        return view->maximumViewportSize();
    }
};

SinglePhotoPreviewLayout::SinglePhotoPreviewLayout(QObject* parent)
    : QObject(parent), d(new SinglePhotoPreviewLayoutPriv)
{
}

SinglePhotoPreviewLayout::~SinglePhotoPreviewLayout()
{
    delete d;
}

void SinglePhotoPreviewLayout::setGraphicsView(GraphicsDImgView* view)
{
    d->view = view;
}

void SinglePhotoPreviewLayout::addItem(GraphicsDImgItem* item)
{
    if (d->item)
    {
        disconnect(d->item, SIGNAL(imageChanged()),
                   this, SLOT(updateZoomAndSize()));
    }

    d->item = item;

    if (d->item)
    {
        connect(d->item, SIGNAL(imageChanged()),
                this, SLOT(updateZoomAndSize()));
    }
}

bool SinglePhotoPreviewLayout::isFitToWindow() const
{
    return d->isFitToWindow;
}

double SinglePhotoPreviewLayout::zoomFactor() const
{
    if (!d->item || !d->view)
    {
        return 1;
    }

    return d->zoomSettings()->zoomFactor();
}

double SinglePhotoPreviewLayout::maxZoomFactor() const
{
    return d->maxZoom;
}

double SinglePhotoPreviewLayout::minZoomFactor() const
{
    return d->minZoom;
}

bool SinglePhotoPreviewLayout::atMaxZoom() const
{
    return zoomFactor() >= d->maxZoom;
}

bool SinglePhotoPreviewLayout::atMinZoom() const
{
    return zoomFactor() <= d->minZoom;
}

void SinglePhotoPreviewLayout::setMaxZoomFactor(double z)
{
    d->maxZoom = z;
}

void SinglePhotoPreviewLayout::setMinZoomFactor(double z)
{
    d->minZoom = z;
}

void SinglePhotoPreviewLayout::increaseZoom(const QPoint& viewportAnchor)
{
    if (!d->item || !d->view)
    {
        return;
    }

    double zoom = d->zoomSettings()->zoomFactor() * d->zoomMultiplier;
    zoom        = qMin(zoom, d->maxZoom);
    zoom        = d->zoomSettings()->snappedZoomStep(zoom, d->frameSize());
    setZoomFactor(zoom, viewportAnchor);
}

void SinglePhotoPreviewLayout::decreaseZoom(const QPoint& viewportAnchor)
{
    if (!d->item || !d->view)
    {
        return;
    }

    double zoom    = d->zoomSettings()->zoomFactor() / d->zoomMultiplier;
    zoom           = qMax(zoom, d->minZoom);
    zoom           = d->zoomSettings()->snappedZoomStep(zoom, d->frameSize());
    setZoomFactor(zoom, viewportAnchor);
}

void SinglePhotoPreviewLayout::setZoomFactorSnapped(double z)
{
    setZoomFactor(z, SnapZoomFactor);
}

void SinglePhotoPreviewLayout::setZoomFactor(double z, const QPoint& givenAnchor, SetZoomFlags flags)
{
    if (!d->item || !d->view)
    {
        return;
    }

    QPoint  viewportAnchor = givenAnchor.isNull() ? d->view->viewport()->rect().center() : givenAnchor;
    QPointF sceneAnchor    = d->view->mapToScene(viewportAnchor);
    QPointF imageAnchor    = d->zoomSettings()->mapZoomToImage(sceneAnchor);

    setZoomFactor(z, flags);

    d->view->scrollPointOnPoint(d->zoomSettings()->mapImageToZoom(imageAnchor), viewportAnchor);
}

void SinglePhotoPreviewLayout::setZoomFactor(double z, SetZoomFlags flags)
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (flags & SnapZoomFactor)
    {
        z = d->zoomSettings()->snappedZoomFactor(z, d->frameSize());
    }

    d->isFitToWindow = false;
    d->previousZoom  = d->zoomSettings()->zoomFactor();

    d->zoomSettings()->setZoomFactor(z);
    d->item->sizeHasChanged();
    updateLayout();
    d->item->update();

    emit fitToWindowToggled(d->isFitToWindow);
    emit zoomFactorChanged(d->zoomSettings()->zoomFactor());

    if (flags & CenterView)
    {
        d->view->centerOn(d->view->scene()->sceneRect().width() / 2.0,
                          d->view->scene()->sceneRect().height() / 2.0);
    }
}

void SinglePhotoPreviewLayout::fitToWindow()
{
    if (!d->item || !d->view)
    {
        return;
    }

    d->isFitToWindow = true;
    d->previousZoom  = d->zoomSettings()->zoomFactor();

    d->zoomSettings()->fitToSize(d->frameSize(), ImageZoomSettings::OnlyScaleDown);
    d->item->sizeHasChanged();
    updateLayout();
    d->item->update();

    emit fitToWindowToggled(d->isFitToWindow);
    emit zoomFactorChanged(d->zoomSettings()->zoomFactor());
}

void SinglePhotoPreviewLayout::toggleFitToWindow()
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (d->isFitToWindow)
    {
        setZoomFactor(d->previousZoom);
    }
    else
    {
        fitToWindow();
    }
}

void SinglePhotoPreviewLayout::toggleFitToWindowOr100()
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (d->isFitToWindow)
    {
        setZoomFactor(1.0);
    }
    else
    {
        fitToWindow();
    }
}

void SinglePhotoPreviewLayout::updateLayout()
{
    if (!d->item || !d->view)
    {
        return;
    }

    d->view->scene()->setSceneRect(d->item->boundingRect());
    d->item->setPos(0, 0);
}

void SinglePhotoPreviewLayout::updateZoomAndSize()
{
    // Set zoom for fit-in-window as minimum, but don't scale up images
    // that are smaller than the available space, only scale down.
    double fitZoom = d->zoomSettings()->fitToSizeZoomFactor(d->frameSize(), ImageZoomSettings::OnlyScaleDown);
    setMinZoomFactor(fitZoom);
    setMaxZoomFactor(12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() <= fitZoom || d->isFitToWindow)
    {
        fitToWindow();
    }

    updateLayout();
}

} // namespace Digikam
