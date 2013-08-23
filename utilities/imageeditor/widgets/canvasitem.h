/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-04
 * Description : image editor canvas item for image editor.
 *
 * Copyright (C) 2013 Yiou Wang <geow812 at gmail dot com>
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

#ifndef CANVASITEM_H
#define CANVASITEM_H

// Qt includes

#include <QStyleOptionGraphicsItem>

// Local includes

#include "graphicsdimgitem.h"
#include "digikam_export.h"
#include "dimg.h"
#include "canvas.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT CanvasItem : public GraphicsDImgItem
{

public:

    CanvasItem(Canvas* view);
    virtual ~CanvasItem();
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    EditorCore* im();

private:

    class Private;
    Private* const d_ptr;

};

}  // namespace Digikam

#endif
