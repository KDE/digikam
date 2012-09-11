/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-27
 * Description : image preview item for image editor.
 *
 * Copyright (C) 2011 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepreviewitem.h"

// Qt includes

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

// Local includes

#include "dimg.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "editorcore.h"
#include "dimgitemspriv.h"

namespace Digikam
{

ImagePreviewItem::ImagePreviewItem()
{
}

ImagePreviewItem::~ImagePreviewItem()
{
}

void ImagePreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    QRect   drawRect     = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect().size().toSize();

    // scale "as if" scaling to whole image, but clip output to our exposed region
    DImg scaledImage     = d->image.smoothScaleClipped(completeSize.width(), completeSize.height(),
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
        // TODO: factoring ICC settings code using ImageIface/EditorCore methods.

        // Apply CM settings.

        ICCSettingsContainer iccSettings = EditorCore::defaultInstance()->getICCSettings();

        if (iccSettings.enableCM && iccSettings.useManagedView)
        {
            IccManager manager(scaledImage);
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

    ExposureSettingsContainer* expoSettings = EditorCore::defaultInstance()->getExposureSettings();

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

}  // namespace Digikam
