/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-25
 * Description : image region widget item for image editor.
 *
 * Copyright (C) 2013 Yiou Wang <geow812 at gmail dot com>
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

// Local includes
#include "previewtoolbar.h"
#include "dimgitemspriv.h"
#include "iccsettingscontainer.h"
#include "editorcore.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "editortool.h"

namespace Digikam
{

class ImageRegionItem::Private
{
public:
  
    Private():
        renderingPreviewMode(PreviewToolBar::PreviewBothImagesVertCont)
    {
      
    }

    QPixmap   pixmapRegion;          // Pixmap of current region to render including original and target for paint method.
    DImg      targetImage;
    int       renderingPreviewMode;
    QRect     imageRegion;
    QPolygon  hightlightPoints;
};

ImageRegionItem::ImageRegionItem():
    d_ptr(new Private)
{
}

ImageRegionItem::~ImageRegionItem()
{
    delete d_ptr;
}

QRect ImageRegionItem::getImageRegion()
{
    return d_ptr->imageRegion;
}

void ImageRegionItem::setTargetImage(const DImg& img)
{
    d_ptr->targetImage = img;

    //TODO: remove this after different PreviewModes are implemented
    Q_D(GraphicsDImgItem);
    d->image = img;
}

void ImageRegionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);
    
    QRect     drawRect = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    d_ptr->imageRegion = drawRect;
    QRect     pixSourceRect;
    QPixmap   pix;
    
    // scale "as if" scaling to whole image, but clip output to our exposed region
    DImg scaledImage     = d->image.smoothScaleClipped(d->image.width(), d->image.height(),
                                                       drawRect.x(), drawRect.y(), drawRect.width(), drawRect.height());

    if (d->cachedPixmaps.find(drawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect.topLeft(), pix);
        }
        else
        {
            painter->drawPixmap(drawRect.topLeft(), pix, pixSourceRect);
        }
    }
    else
    {

        ICCSettingsContainer iccSettings = EditorCore::defaultInstance()->getICCSettings();

        if (iccSettings.enableCM && iccSettings.useManagedView)
        {
            IccManager   manager(scaledImage);
            IccTransform monitorICCtrans = manager.displayTransform(widget);
            pix                          = scaledImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pix = scaledImage.convertToPixmap();
        }

        d->cachedPixmaps.insert(drawRect, pix);

        painter->drawPixmap(drawRect.topLeft(), pix);
    }

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            painter->drawPixmap(drawRect.topLeft(), pixMask);
        }
    }
}

void ImageRegionItem::setHighLightPoints(const QPolygon& pointsList)
{
    d_ptr->hightlightPoints = pointsList;
}

}  // namespace Digikam
