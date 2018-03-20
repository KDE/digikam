/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol do de>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamimageview_p.h"

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

DigikamImageView::Private::Private(DigikamImageView* const qq)
    : overlaysActive(false),
      fullscreenActive(false),
      q_ptr(qq)
{
    utilities          = 0;
    rotateLeftOverlay  = 0;
    rotateRightOverlay = 0;
    fullscreenOverlay  = 0;
    normalDelegate     = 0;
    faceDelegate       = 0;
    faceMode           = false;
}

DigikamImageView::Private::~Private()
{
}

void DigikamImageView::Private::updateOverlays()
{
    Q_Q(DigikamImageView);

    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

            q->removeOverlay(rotateLeftOverlay);
            q->removeOverlay(rotateRightOverlay);

            overlaysActive = false;
        }
    }
    else
    {
        if (settings->getIconShowOverlays())
        {
            q->addOverlay(rotateLeftOverlay, normalDelegate);
            q->addOverlay(rotateRightOverlay, normalDelegate);

            connect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                    q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            connect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                    q, SLOT(slotRotateRight(QList<QModelIndex>)));

            overlaysActive = true;
        }
    }

    if (fullscreenActive)
    {
        if (!settings->getIconShowFullscreen())
        {
            disconnect(fullscreenOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotFullscreen(QList<QModelIndex>)));

            q->removeOverlay(fullscreenOverlay);

            fullscreenActive = false;
        }
    }
    else
    {
        if (settings->getIconShowFullscreen())
        {
            fullscreenActive = true;

            q->addOverlay(fullscreenOverlay, normalDelegate);

            connect(fullscreenOverlay, SIGNAL(signalFullscreen(QList<QModelIndex>)),
                    q, SLOT(slotFullscreen(QList<QModelIndex>)));
        }
    }
}

} // namespace Digikam
