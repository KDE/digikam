/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-25-07
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importiconview_p.h"

namespace Digikam
{

ImportIconView::Private::Private(ImportIconView* const qq)
    : overlaysActive(false), q_ptr(qq)
{
    utilities          = 0;
    rotateLeftOverlay  = 0;
    rotateRightOverlay = 0;
    normalDelegate     = 0;
}

ImportIconView::Private::~Private()
{
}

void ImportIconView::Private::updateOverlays()
{
    Q_Q(ImportIconView);

    ImportSettings* const settings = ImportSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

            q->removeOverlay(rotateRightOverlay);
            q->removeOverlay(rotateLeftOverlay);

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
}

} // namespace Digikam
