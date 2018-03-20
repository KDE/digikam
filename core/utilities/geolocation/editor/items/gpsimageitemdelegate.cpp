/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A model for the view to display a list of images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsimageitemdelegate.h"

// Qt includes


#include <QPainter>
#include <QIcon>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class GPSImageItemDelegate::Private
{
public:

    Private()
    : imageList(0),
      thumbnailSize(60)
    {
    }

    GPSImageList* imageList;
    int            thumbnailSize;
};

GPSImageItemDelegate::GPSImageItemDelegate(GPSImageList* const imageList, QObject* const parent)
    : QItemDelegate(parent), d(new Private())
{
    d->imageList = imageList;
}

GPSImageItemDelegate::~GPSImageItemDelegate()
{
    delete d;
}

void GPSImageItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const
{
    if (sortMappedindex.column() != GPSImageItem::ColumnThumbnail)
    {
        QItemDelegate::paint(painter, option, sortMappedindex);
        return;
    }

    const QModelIndex& sourceModelIndex = d->imageList->getSortProxyModel()->mapToSource(sortMappedindex);

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    // TODO: clipping, selected state, disabled state, etc.
    QPixmap itemPixmap = d->imageList->getModel()->getPixmapForIndex(sourceModelIndex, d->thumbnailSize);

    if (itemPixmap.isNull())
    {
        // TODO: paint some default logo
        // TODO: cache this logo
        itemPixmap = QIcon::fromTheme(QString::fromLatin1("view-preview"))
                                      .pixmap(d->thumbnailSize, QIcon::Disabled);
    }

    const QSize availableSize = option.rect.size();
    const QSize pixmapSize    = itemPixmap.size().boundedTo(availableSize);
    QPoint startPoint((availableSize.width()  - pixmapSize.width())  / 2,
                      (availableSize.height() - pixmapSize.height()) / 2);
    startPoint               += option.rect.topLeft();
    painter->drawPixmap(QRect(startPoint, pixmapSize), itemPixmap, QRect(QPoint(0, 0), pixmapSize));
}

QSize GPSImageItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const
{
    if (sortMappedindex.column() == GPSImageItem::ColumnThumbnail)
    {
        return QSize(d->thumbnailSize, d->thumbnailSize);
    }

    const QSize realSizeHint = QItemDelegate::sizeHint(option, sortMappedindex);

    return QSize(realSizeHint.width(), d->thumbnailSize);
}

void GPSImageItemDelegate::setThumbnailSize(const int size)
{
    d->thumbnailSize                 = size;
    GPSImageModel* const imageModel = d->imageList->getModel();

    if (!imageModel)
        return;

    if (imageModel->rowCount() > 0)
    {
        // TODO: is it enough to emit this signal for only 1 item?
        // seems to work in Qt4.5 with QTreeView::setUniformRowHeights(true)
        emit(sizeHintChanged(imageModel->index(0, 0)));
    }
}

int GPSImageItemDelegate::getThumbnailSize() const
{
    return d->thumbnailSize;
}

} // namespace Digikam
