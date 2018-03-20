/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt item view for images - the delegate Private
 *
 * Copyright (C) 2013      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTODELEGATEPRIV_H
#define SHOWFOTODELEGATEPRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "showfotothumbnailbar.h"
#include "itemviewshowfotodelegatepriv.h"
#include "showfotodelegate.h"

namespace ShowFoto
{

class ShowfotoDelegate::ShowfotoDelegatePrivate : public ItemViewShowfotoDelegatePrivate
{
public:

    ShowfotoDelegatePrivate()
    {
        contentWidth        = 0;
        drawImageFormat     = true;
        drawMouseOverFrame  = true;
        drawFocusFrame      = true;
        currentModel        = 0;
        currentView         = 0;

        actualPixmapRectCache.setMaxCost(250);
    }

    int                    contentWidth;

    QRect                  dateRect;
    QRect                  pixmapRect;
    QRect                  nameRect;
    QRect                  resolutionRect;
    QRect                  sizeRect;
    QRect                  imageInformationRect;
    QRect                  groupRect;
    QRect                  coordinatesRect;

    bool                   drawImageFormat;
    bool                   drawFocusFrame;
    bool                   drawMouseOverFrame;

    QCache<int, QRect>     actualPixmapRectCache;

    ShowfotoThumbnailBar*  currentView;
    QAbstractItemModel*    currentModel;

public:

    virtual void clearRects();
};

// --- ShowfotoThumbnailDelegate ----------------------------------------------------

class ShowfotoThumbnailDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoThumbnailDelegatePrivate()
    {
        flow                = QListView::LeftToRight;

        // switch off drawing of frames
        drawMouseOverFrame  = false;
        drawFocusFrame      = false;
    }

    void init(ShowfotoThumbnailDelegate* const q);

public:

    QListView::Flow flow;
    QRect           viewSize;
};

// --- ShowfotoNormalDelegate ----------------------------------------------------

class ShowfotoNormalDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoNormalDelegatePrivate()
    {
    }

    void init(ShowfotoNormalDelegate* const q, ShowfotoThumbnailBar* const parent);
};

} // namespace Digikam


#endif // SHOWFOTODELEGATEPRIV_H
