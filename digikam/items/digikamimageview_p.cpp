/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol do de>
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

#include "digikamimageview_p.moc"

// KDE includes

#include <KActionMenu>
#include <KMenu>

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

DigikamImageViewPriv::DigikamImageViewPriv(DigikamImageView* qq)
    : overlaysActive(false), q_ptr(qq)
{
    utilities          = 0;
    rotateLeftOverlay  = 0;
    rotateRightOverlay = 0;
    normalDelegate     = 0;
    faceDelegate       = 0;
}

DigikamImageViewPriv::~DigikamImageViewPriv()
{
}

void DigikamImageViewPriv::updateOverlays()
{
    Q_Q(DigikamImageView);
    AlbumSettings* settings = AlbumSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(const QList<QModelIndex>&)),
                       q, SLOT(slotRotateLeft(const QList<QModelIndex>&)));

            disconnect(rotateRightOverlay, SIGNAL(signalRotate(const QList<QModelIndex>&)),
                       q, SLOT(slotRotateRight(const QList<QModelIndex>&)));

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

            connect(rotateLeftOverlay, SIGNAL(signalRotate(const QList<QModelIndex>&)),
                    q, SLOT(slotRotateLeft(const QList<QModelIndex>&)));

            connect(rotateRightOverlay, SIGNAL(signalRotate(const QList<QModelIndex>&)),
                    q, SLOT(slotRotateRight(const QList<QModelIndex>&)));

            overlaysActive = true;
        }
    }
}

void DigikamImageViewPriv::triggerRotateAction(const char* actionName)
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());

    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == actionName)
            {
                ac->trigger();
            }
        }
    }
}

} // namespace Digikam
