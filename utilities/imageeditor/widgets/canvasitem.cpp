/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-04
 * Description : image editor canvas item for image editor.
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

#include "canvasitem.h"

// Qt includes
#include <QPixmap>
#include <QPainter>

// KDE includes

#include <klocale.h>

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

class CanvasItem::Private
{
public:
  
    Private()
        
    {
        view       = 0;
        rotated    = false;
    }

    Canvas*          view;
    QRect            drawRect;
    QPixmap          pix;
    bool             rotated;
};

CanvasItem::CanvasItem(Canvas *widget):
    d_ptr(new Private)
{
    d_ptr->view = widget;
}

CanvasItem::~CanvasItem()
{
    delete d_ptr;
}

void CanvasItem::toggleRotated()
{
    d_ptr->rotated = !d_ptr->rotated;
}

QRectF CanvasItem::boundingRect() const
{
    // always return full integer sizes, we can only scale to integer
    if (!d_ptr->rotated)
    {
        return QRectF(QPointF(0,0), zoomSettings()->zoomedSize()).toAlignedRect();
    }
    else
    {
        double newHeight = pow(zoomSettings()->zoomedSize().height(), 2) / zoomSettings()->zoomedSize().width();
        return QRectF(0, 0, zoomSettings()->zoomedSize().height(), newHeight).toAlignedRect();
    }
}

void CanvasItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    GraphicsDImgItem::paint(painter, option, widget);

    /*if (d_ptr->view->im->getImg())
    {
        QRect   pixSourceRect;
        QSize   completeSize = boundingRect().size().toSize();
        DImg    scaledImage;
        d_ptr->drawRect = boundingRect().toAlignedRect();
        scaledImage = d_ptr->view->im->getImg()->smoothScale(completeSize.width(), completeSize.height(), Qt::IgnoreAspectRatio);

        if (d->cachedPixmaps.find(d_ptr->drawRect, &d_ptr->pix, &pixSourceRect))
        {
            if (pixSourceRect.isNull())
            {
                painter->drawPixmap(d_ptr->drawRect.topLeft(), d_ptr->pix);
            }
            else
            {
                painter->drawPixmap(d_ptr->drawRect.topLeft(), d_ptr->pix, pixSourceRect);
            }
        }
        else
        {
            ICCSettingsContainer iccSettings = d_ptr->view->im->getICCSettings();

            if (iccSettings.enableCM && iccSettings.useManagedView)
            {
                IccManager   manager(scaledImage);
                IccTransform monitorICCtrans = manager.displayTransform(widget);
                d_ptr->pix = scaledImage.convertToPixmap(monitorICCtrans);
            }
            else
            {
                d_ptr->pix = scaledImage.convertToPixmap();
            }

            d->cachedPixmaps.insert(d_ptr->drawRect, d_ptr->pix);
            painter->drawPixmap(d_ptr->drawRect.topLeft(), d_ptr->pix);
        }

        // Show the Over/Under exposure pixels indicators

        ExposureSettingsContainer* const expoSettings = d_ptr->im->getExposureSettings();

        if (expoSettings)
        {
            if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
            {
                QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
                QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
                painter->drawPixmap(d_ptr->drawRect.topLeft(), pixMask);
            }
        }
    }*/
}

}
// namespace Digikam
