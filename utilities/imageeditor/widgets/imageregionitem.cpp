/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-25
 * Description : image region widget item for image editor.
 *
 * Copyright (C) 2013-2014 Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageregionitem.h"

// Qt includes

#include <QPixmap>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimgitemspriv.h"
#include "editorcore.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "iccsettingscontainer.h"
#include "imageiface.h"
#include "previewtoolbar.h"

namespace Digikam
{

class ImageRegionItem::Private
{
public:

    Private():
        onMouseMovePreviewToggled(true),
        renderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont),
        view(0),
        iface(0)
    {
    }

    bool               onMouseMovePreviewToggled;
    int                renderingPreviewMode;

    QPixmap            targetPix;    // Pixmap of target region to render for paint method.
    QRect              drawRect;
    QPolygon           hightlightPoints;

    ImageRegionWidget* view;
    ImageIface*        iface;
};

ImageRegionItem::ImageRegionItem(ImageRegionWidget* const widget):
    d_ptr(new Private)
{
    d_ptr->view  = widget;
    d_ptr->iface = new ImageIface;
    setAcceptHoverEvents(true);
    setImage(d_ptr->iface->original() ? d_ptr->iface->original()->copy() : DImg());
}

ImageRegionItem::~ImageRegionItem()
{
    delete d_ptr->iface;
    delete d_ptr;
}

QRect ImageRegionItem::getImageRegion() const
{
    return d_ptr->drawRect;
}

void ImageRegionItem::setTargetImage(DImg& img)
{
    d_ptr->targetPix = d_ptr->iface->convertToPixmap(img);
    update();
}

void ImageRegionItem::setRenderingPreviewMode(int mode)
{
    d_ptr->renderingPreviewMode = mode;
    update();
}

void ImageRegionItem::setHighLightPoints(const QPolygon& pointsList)
{
    d_ptr->hightlightPoints = pointsList;
}

void ImageRegionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    d_ptr->drawRect      = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect().size().toSize();

    // scale "as if" scaling to whole image, but clip output to our exposed region
    DImg scaledImage     = d->image.smoothScaleClipped(completeSize.width(),    completeSize.height(),
                                                       d_ptr->drawRect.x(),     d_ptr->drawRect.y(),
                                                       d_ptr->drawRect.width(), d_ptr->drawRect.height());

    if (d->cachedPixmaps.find(d_ptr->drawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(d_ptr->drawRect.topLeft(), pix);
        }
        else
        {
            painter->drawPixmap(d_ptr->drawRect.topLeft(), pix, pixSourceRect);
        }
    }
    else
    {
        // TODO: factoring ICC settings code using ImageIface/EditorCore methods.

        // Apply CM settings.

        bool doSoftProofing              = EditorCore::defaultInstance()->softProofingEnabled();
        ICCSettingsContainer iccSettings = EditorCore::defaultInstance()->getICCSettings();

        if (iccSettings.enableCM && (iccSettings.useManagedView || doSoftProofing))
        {
            IccManager   manager(scaledImage);
            IccTransform monitorICCtrans;

            if (doSoftProofing)
            {
                monitorICCtrans = manager.displaySoftProofingTransform(iccSettings.defaultProofProfile, widget);
            }
            else
            {
                monitorICCtrans = manager.displayTransform(widget);
            }

            pix = scaledImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pix = scaledImage.convertToPixmap();
        }

        d->cachedPixmaps.insert(d_ptr->drawRect, pix);

        painter->drawPixmap(d_ptr->drawRect.topLeft(), pix);
    }

    paintExtraData(painter);

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            painter->drawPixmap(d_ptr->drawRect.topLeft(), pixMask);
        }
    }
}

void ImageRegionItem::paintExtraData(QPainter* const p)
{
    QRect viewportRect   = boundingRect().toAlignedRect();
    QRect fontRectBefore = p->fontMetrics().boundingRect(viewportRect, 0, i18n("Before"));
    QRect fontRectAfter  = p->fontMetrics().boundingRect(viewportRect, 0, i18n("After"));

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBackgroundMode(Qt::TransparentMode);

    if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
        (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d_ptr->onMouseMovePreviewToggled))
    {
        d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + 20, d_ptr->drawRect.topLeft().y() + 20), fontRectBefore.size()), i18n("Before"));
    }

    else if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
             d_ptr->renderingPreviewMode == PreviewToolBar::NoPreviewMode      ||
             (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && d_ptr->onMouseMovePreviewToggled))
    {
        p->drawPixmap(d_ptr->drawRect.x(), d_ptr->drawRect.y(), d_ptr->targetPix, 0, 0, d_ptr->drawRect.width(), d_ptr->drawRect.height());

        if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
            d_ptr->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
        {
            d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + 20, d_ptr->drawRect.topLeft().y() + 20), fontRectAfter.size()), i18n("After"));
        }
    }

    else if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert ||
             d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
    {
        if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
        {
            p->drawPixmap(d_ptr->drawRect.x() + d_ptr->drawRect.width() / 2, d_ptr->drawRect.y(), d_ptr->targetPix, 0, 0, d_ptr->drawRect.width()/2, d_ptr->drawRect.height());
        }

        if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
        {
            p->drawPixmap(d_ptr->drawRect.x() + d_ptr->drawRect.width() / 2, d_ptr->drawRect.y(), d_ptr->targetPix, d_ptr->drawRect.width()/2, 0, d_ptr->drawRect.width(), d_ptr->drawRect.height());
        }

        p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p->drawLine(d_ptr->drawRect.topLeft().x()+d_ptr->drawRect.width()/2, d_ptr->drawRect.topLeft().y(), d_ptr->drawRect.topLeft().x()+d_ptr->drawRect.width()/2, d_ptr->drawRect.bottomLeft().y());
        p->setPen(QPen(Qt::red, 2, Qt::DotLine));
        p->drawLine(d_ptr->drawRect.topLeft().x() + d_ptr->drawRect.width()/2, d_ptr->drawRect.topLeft().y(), d_ptr->drawRect.topLeft().x()+d_ptr->drawRect.width()/2, d_ptr->drawRect.bottomLeft().y());
        d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + 20,                             d_ptr->drawRect.topLeft().y() + 20), fontRectBefore.size()), i18n("Before"));
        d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + d_ptr->drawRect.width()/2 + 20, d_ptr->drawRect.topLeft().y() + 20), fontRectAfter.size()),  i18n("After"));
    }

    else if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz ||
             d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
    {
        if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
        {
            p->drawPixmap(d_ptr->drawRect.x(), d_ptr->drawRect.y() + d_ptr->drawRect.height() / 2, d_ptr->targetPix, 0, 0, d_ptr->drawRect.width(), d_ptr->drawRect.height()/2);
        }

        if (d_ptr->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
        {
            p->drawPixmap(d_ptr->drawRect.x(), d_ptr->drawRect.y() + d_ptr->drawRect.height() / 2, d_ptr->targetPix, 0, d_ptr->drawRect.height()/2, d_ptr->drawRect.width(), d_ptr->drawRect.height());
        }

        p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p->drawLine(d_ptr->drawRect.topLeft().x() + 1, d_ptr->drawRect.topLeft().y() + d_ptr->drawRect.height()/2, d_ptr->drawRect.topRight().x() - 1, d_ptr->drawRect.topLeft().y() + d_ptr->drawRect.height()/2);
        p->setPen(QPen(Qt::red, 2, Qt::DotLine));
        p->drawLine(d_ptr->drawRect.topLeft().x() + 1, d_ptr->drawRect.topLeft().y() + d_ptr->drawRect.height()/2, d_ptr->drawRect.topRight().x() - 1, d_ptr->drawRect.topLeft().y() + d_ptr->drawRect.height()/2);

        d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + 20, d_ptr->drawRect.topLeft().y() + 20),                              fontRectBefore.size()), i18n("Before"));
        d_ptr->view->drawText(p, QRectF(QPointF(d_ptr->drawRect.topLeft().x() + 20, d_ptr->drawRect.topLeft().y() + d_ptr->drawRect.height()/2 + 20), fontRectAfter.size()),  i18n("After"));
    }

    // Drawing highlighted points.

    if (!d_ptr->hightlightPoints.isEmpty())
    {
        QPoint pt;
        QRectF hpArea;


        for (int i = 0 ; i < d_ptr->hightlightPoints.count() ; ++i)
        {
            pt             = d_ptr->hightlightPoints.point(i);
            double zoomFactor = zoomSettings()->zoomFactor();
            int x = (int)((double)pt.x() * zoomFactor);
            int y = (int)((double)pt.y() * zoomFactor);

            // Check if zoomed point is inside, not actual point
            if (d_ptr->drawRect.contains(QPoint(x,y)))
            {

                //QPoint hp(contentsToViewport(QPointF(x, y)));
                QPointF hp(mapToScene(QPointF(x, y)));

                hpArea.setSize(QSize((int)(16 * zoomFactor), (int)(16 * zoomFactor)));
                hpArea.moveCenter(hp);

                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(hp.x(),                         hpArea.y(),                     hp.x(),                         hp.y() - (int)(3 * zoomFactor));
                p->drawLine(hp.x(),                         hp.y() + (int)(3 * zoomFactor), hp.x(),                         hpArea.bottom());
                p->drawLine(hpArea.x(),                     hp.y(),                         hp.x() - (int)(3 * zoomFactor), hp.y());
                p->drawLine(hp.x() + (int)(3 * zoomFactor), hp.y(),                         hpArea.right(),                 hp.y());

                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(hp.x(),                         hpArea.y(),                     hp.x(),                         hp.y() - (int)(3 * zoomFactor));
                p->drawLine(hp.x(),                         hp.y() + (int)(3 * zoomFactor), hp.x(),                         hpArea.bottom());
                p->drawLine(hpArea.x(),                     hp.y(),                         hp.x() - (int)(3 * zoomFactor), hp.y());
                p->drawLine(hp.x() + (int)(3 * zoomFactor), hp.y(),                         hpArea.right(),                 hp.y());
            }
        }
    }
}

void ImageRegionItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    d_ptr->onMouseMovePreviewToggled = false;
    update();
}

void ImageRegionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    d_ptr->onMouseMovePreviewToggled = true;
    update();
}

} // namespace Digikam
