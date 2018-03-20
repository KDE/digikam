/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef ITEMVIEWSHOWFOTODELEGATEPRIV_H
#define ITEMVIEWSHOWFOTODELEGATEPRIV_H

// Qt includes

#include <QFont>
#include <QPainter>
#include <QModelIndex>

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "imagedelegateoverlay.h"

using namespace Digikam;

namespace ShowFoto
{

class ItemViewShowfotoDelegate;

class ItemViewShowfotoDelegatePrivate
{
public:

    ItemViewShowfotoDelegatePrivate();
    virtual ~ItemViewShowfotoDelegatePrivate() {}

    void init(ItemViewShowfotoDelegate* const _q);

    /// Resets cached rects. Remember to reimplement in subclass for added rects.
    virtual void clearRects();

public:

    int                       spacing;
    QSize                     gridSize;

    QRect                     rect;

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps;

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    ThumbnailSize             thumbSize;

    ItemViewShowfotoDelegate* q;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    // constant values for drawing
    int                       radius;
    int                       margin;
};

} // namespace ShowFoto

#endif // ITEMVIEWSHOWFOTODELEGATEPRIVATE_H
