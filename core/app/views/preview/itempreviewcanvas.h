/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the canvas preview widget.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier  <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_PREVIEW_CANVAS_H
#define DIGIKAM_ITEM_PREVIEW_CANVAS_H

// Local includes

#include "digikam_config.h"
#include "dimgpreviewitem.h"
#include "iteminfo.h"

class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneHoverEvent;

namespace Digikam
{

class FaceGroup;

class ItemPreviewCanvas : public DImgPreviewItem
{
public:

    explicit ItemPreviewCanvas();
    virtual ~ItemPreviewCanvas();

    void setItemInfo(const ItemInfo& info);
    ItemInfo imageInfo() const;

    void setFaceGroup(FaceGroup* const group);

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PREVIEW_CANVAS_H
