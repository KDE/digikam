/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-27
 * Description : image preview item for image editor.
 *
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

#include "imagepreviewitem.h"

// Qt includes

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

// Local includes

#include "digikam_config.h"
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
    Q_D(GraphicsDImgItem);
    d->init(this);
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

    /* For high resolution ("retina") displays, Mac OS X / Qt
       report only half of the physical resolution in terms of
       pixels, i.e. every logical pixels corresponds to 2x2
       physical pixels. However, UI elements and fonts are
       nevertheless rendered at full resolution, and pixmaps
       as well, provided their resolution is high enough (that
       is, higher than the reported, logical resolution).

       To work around this, we render the photos not a logical
       resolution, but with the photo's full resolution, but
       at the screen's aspect ratio. When we later draw this
       high resolution bitmap, it is up to Qt to scale the
       photo to the true physical resolution.  The ratio
       computed below is the ratio between the photo and
       screen resolutions, or equivalently the factor by which
       we need to increase the pixel size of the rendered
       pixmap.
    */
#ifdef USE_QT_SCALING
    double xratio = double(d->image.width()) / completeSize.width();
    double yratio = double(d->image.height()) / completeSize.height();
    double ratio = qMax(qMin(xratio, yratio), 1.0);
#else
    double ratio = 1.0;
#endif

    QRect  scaledDrawRect = QRectF(ratio*drawRect.x(), ratio*drawRect.y(),
                                   ratio*drawRect.width(), ratio*drawRect.height()).toRect();

    // scale "as if" scaling to whole image, but clip output to our exposed region
    QSize scaledCompleteSize = QSizeF(ratio*completeSize.width(), ratio*completeSize.height()).toSize();
    DImg scaledImage   = d->image.smoothScaleClipped(scaledCompleteSize.width(), scaledCompleteSize.height(),
                                                     scaledDrawRect.x(), scaledDrawRect.y(),
                                                     scaledDrawRect.width(), scaledDrawRect.height());

    if (d->cachedPixmaps.find(scaledDrawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect, pix);
        }
        else
        {
            painter->drawPixmap(drawRect, pix, pixSourceRect);
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

        d->cachedPixmaps.insert(scaledDrawRect, pix);

        painter->drawPixmap(drawRect, pix);
    }

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            painter->drawPixmap(drawRect, pixMask);
        }
    }
}

}  // namespace Digikam
