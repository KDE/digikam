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

#ifndef RUBBER_ITEM_H
#define RUBBER_ITEM_H

// Qt includes

#include <QFlags>

// Local includes

#include "canvas.h"
#include "imagepreviewitem.h"
#include "regionframeitem.h"
#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT RubberItem : public RegionFrameItem
{
    Q_OBJECT

public:

    RubberItem(ImagePreviewItem* const item);
    virtual ~RubberItem();

    void setCanvas(Canvas* const canvas);

protected:

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* RUBBER_ITEM_H */
