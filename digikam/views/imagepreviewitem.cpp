/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the image preview widget.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "imagepreviewitem.h"

// Qt includes

#include <QGraphicsSceneContextMenuEvent>
#include <QMouseEvent>

// Local includes

#include "facegroup.h"
#include "imagepreviewview.h"

namespace Digikam
{

class ImagePreviewItem::Private
{
public:

    Private()
    {
        view  = 0;
        group = 0;
    }

    ImagePreviewView* view;
    FaceGroup*        group;
    ImageInfo         info;
};

ImagePreviewItem::ImagePreviewItem(ImagePreviewView* const view)
    : d(new Private)
{
    d->view = view;
    setAcceptHoverEvents(true);
}

ImagePreviewItem::~ImagePreviewItem()
{
    delete d;
}

void ImagePreviewItem::setFaceGroup(FaceGroup* const group)
{
    d->group = group;
}

void ImagePreviewItem::setImageInfo(const ImageInfo& info)
{
    d->info = info;
    setPath(info.filePath());
}

ImageInfo ImagePreviewItem::imageInfo() const
{
    return d->info;
}

void ImagePreviewItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    d->view->showContextMenu(d->info, e);
}

void ImagePreviewItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverEnterEvent(e);
}

void ImagePreviewItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverLeaveEvent(e);
}

void ImagePreviewItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverMoveEvent(e);
}

}  // namespace Digikam
