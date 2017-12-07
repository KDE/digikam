/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ITEMVIEWIMAGEDELEGATEPRIV_H
#define ITEMVIEWIMAGEDELEGATEPRIV_H

// Qt includes

#include <QCache>
#include <QFont>
#include <QPainter>
#include <QPolygon>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemViewImageDelegate;

class DIGIKAM_EXPORT ItemViewImageDelegatePrivate
{
public:

    ItemViewImageDelegatePrivate();
    virtual ~ItemViewImageDelegatePrivate()
    {
    }

    void init(ItemViewImageDelegate* const _q);

    void makeStarPolygon();

    /// Resets cached rects. Remember to reimplement in subclass for added rects.
    virtual void clearRects();

public:

    int                       spacing;
    QSize                     gridSize;

    QRect                     rect;
    QRect                     ratingRect;

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps;

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    QPolygon                  starPolygon;
    QSize                     starPolygonSize;

    ThumbnailSize             thumbSize;

    QPersistentModelIndex     editingRating;

    ItemViewImageDelegate*    q;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    // constant values for drawing
    int                       radius;
    int                       margin;
};

} // namespace Digikam

#endif // ITEMVIEWIMAGEDELEGATEPRIV_H
