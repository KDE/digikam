/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the image preview widget.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier  <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPREVIEWVIEWITEM_H
#define IMAGEPREVIEWVIEWITEM_H

// Local includes

#include "config-digikam.h"
#include "dimgpreviewitem.h"
#include "imageinfo.h"

class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneHoverEvent;

namespace Digikam
{

#ifdef HAVE_KFACE
class FaceGroup;
#endif /* HAVE_KFACE */

class ImagePreviewViewItem : public DImgPreviewItem
{
public:

    explicit ImagePreviewViewItem();
    virtual ~ImagePreviewViewItem();

    void setImageInfo(const ImageInfo& info);
    ImageInfo imageInfo() const;

#ifdef HAVE_KFACE
    void setFaceGroup(FaceGroup* const group);
#endif /* HAVE_KFACE */

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWVIEWITEM_H */
