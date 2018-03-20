/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2013-09-13
* Description : rubber item for Canvas
*
* Copyright (C) 2013-2014 by Yiou Wang <geow812 at gmail dot com>
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

#include "rubberitem.h"

namespace Digikam
{

class RubberItem::Private
{

public:

    Private()
    {
        canvas = 0;
    }

    Canvas* canvas;
};

RubberItem::RubberItem(ImagePreviewItem* const parent)
    : RegionFrameItem(parent), d(new Private)
{
}

RubberItem::~RubberItem()
{
    delete d;
}

void RubberItem::setCanvas(Canvas* const canvas)
{
    d->canvas = canvas;
}

void RubberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    RegionFrameItem::mouseReleaseEvent(event);
    d->canvas->slotSelected();
}

void RubberItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    RegionFrameItem::mouseMoveEvent(event);
    d->canvas->slotSelectionMoved();
}

} // namespace Digikam
