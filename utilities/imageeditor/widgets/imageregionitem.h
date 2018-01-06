/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-25
 * Description : image region widget item for image editor.
 *
 * Copyright (C) 2013-2014 Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
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

#ifndef IMAGEREGIONITEM_H
#define IMAGEREGIONITEM_H

// Qt includes

#include <QStyleOptionGraphicsItem>

// Local includes

#include "graphicsdimgitem.h"
#include "digikam_export.h"
#include "dimg.h"
#include "imageregionwidget.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageRegionItem : public GraphicsDImgItem
{

public:

    ImageRegionItem(ImageRegionWidget* const view);
    virtual ~ImageRegionItem();

    void setTargetImage(DImg& img);
    void setHighLightPoints(const QPolygon& pointsList);
    void setRenderingPreviewMode(int mode);

    void  paintExtraData(QPainter* const painter);
    QRect getImageRegion() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

private:

    class Private;
    Private* const d_ptr;
};

}  // namespace Digikam

#endif // IMAGEREGIONITEM_H
