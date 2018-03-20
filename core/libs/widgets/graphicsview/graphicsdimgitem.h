/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GRAPHICSDIMGITEM_H
#define GRAPHICSDIMGITEM_H

// Qt includes

#include <QtGlobal>
#include <QGraphicsObject>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;
class ImageZoomSettings;

class DIGIKAM_EXPORT GraphicsDImgItem : public QGraphicsObject
{
    Q_OBJECT

public:

    explicit GraphicsDImgItem(QGraphicsItem* const parent = 0);
    virtual ~GraphicsDImgItem();

    /**
     * Sets the DImg to be drawn by this item.
     * Note: DImg is explicitly shared, and no copy is automatically taken here.
     */
    void setImage(const DImg& img);
    DImg image() const;

    const ImageZoomSettings* zoomSettings() const;
    ImageZoomSettings*       zoomSettings();

    void            sizeHasChanged();
    void            clearCache();

    virtual QRectF  boundingRect() const;
    virtual void    paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    virtual QString userLoadingHint() const { return QString(); }

Q_SIGNALS:

    void showContextMenu(QGraphicsSceneContextMenuEvent* e);
    void imageChanged();
    void imageSizeChanged(const QSizeF& size);

protected:

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e);

public:

    // Declared public because of DImgPreviewItemPrivate.
    class GraphicsDImgItemPrivate;

protected:

    GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem* const parent);
    GraphicsDImgItemPrivate* const d_ptr;

protected:

    Q_DECLARE_PRIVATE(GraphicsDImgItem)
};

} // namespace Digikam

#endif // GRAPHICSDIMGITEM_H
