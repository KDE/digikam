/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef ITEMVIEWIMPORTDELEGATEPRIV_H
#define ITEMVIEWIMPORTDELEGATEPRIV_H

// Qt includes

#include <QCache>
#include <QFont>
#include <QPainter>
#include <QPolygon>
#include <QModelIndex>

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ImageDelegateOverlay;
class ItemViewImportDelegate;

class ItemViewImportDelegatePrivate
{
public:

    ItemViewImportDelegatePrivate();
    virtual ~ItemViewImportDelegatePrivate() {}

    void init(ItemViewImportDelegate* const _q);

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

    ItemViewImportDelegate*   q;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    // constant values for drawing
    int                       radius;
    int                       margin;
};

} // namespace Digikam

#endif // ITEMVIEWIMPORTDELEGATEPRIV_H
