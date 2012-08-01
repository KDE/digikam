/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-25-07
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importiconview_p.moc"

namespace Digikam
{

ImportIconView::ImportIconViewPriv::ImportIconViewPriv(ImportIconView* const qq)
    : overlaysActive(false), q_ptr(qq)
{
    //TODO: utilities          = 0;
    //TODO: rotateLeftOverlay  = 0;
    //TODO: rotateRightOverlay = 0;
    normalDelegate     = 0;
}

ImportIconView::ImportIconViewPriv::~ImportIconViewPriv()
{
}

void ImportIconView::ImportIconViewPriv::updateOverlays()
{
    Q_Q(ImportIconView);
    ImportSettings* settings = ImportSettings::instance();

//TODO: Implement overlays.
//    if (overlaysActive)
//    {
//        if (!settings->getIconShowOverlays())
//        {
//            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

//            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

//            q->removeOverlay(rotateLeftOverlay);
//            q->removeOverlay(rotateRightOverlay);

//            overlaysActive = false;
//        }
//    }
//    else
//    {
//        if (settings->getIconShowOverlays())
//        {
//            q->addOverlay(rotateLeftOverlay, normalDelegate);
//            q->addOverlay(rotateRightOverlay, normalDelegate);

//            connect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                    q, SLOT(slotRotateLeft(QList<QModelIndex>)));

//            connect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                    q, SLOT(slotRotateRight(QList<QModelIndex>)));

//            overlaysActive = true;
//        }
//    }
}

} // namespace Digikam
