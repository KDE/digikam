/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : widget for displaying an item in view with used filters on current image
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "imagefiltershistorytreeitem.h"

namespace Digikam
{

class ImageFiltersHistoryTreeItem::Private
{
public:

    Private()
    {
        parentItem = 0;
        disabled   = false;
    }

    QList<ImageFiltersHistoryTreeItem*> childItems;
    QList<QVariant>                     itemData;
    ImageFiltersHistoryTreeItem*        parentItem;
    bool                                disabled;
};

ImageFiltersHistoryTreeItem::ImageFiltersHistoryTreeItem(const QList<QVariant>& data, ImageFiltersHistoryTreeItem* const parent)
    : d(new Private)
{
    d->parentItem = parent;
    d->itemData   = data;
}

ImageFiltersHistoryTreeItem::ImageFiltersHistoryTreeItem(const QString& data, ImageFiltersHistoryTreeItem* const parent)
    : d(new Private)
{
    d->parentItem = parent;
    d->itemData.append(data);
}

ImageFiltersHistoryTreeItem::ImageFiltersHistoryTreeItem(const ImageFiltersHistoryTreeItem& other)
    : d(new Private(*other.d))
{
}

ImageFiltersHistoryTreeItem::~ImageFiltersHistoryTreeItem()
{
    qDeleteAll(d->childItems);
    delete d;
}

void ImageFiltersHistoryTreeItem::appendChild(ImageFiltersHistoryTreeItem* const item)
{
    d->childItems.append(item);
}

ImageFiltersHistoryTreeItem* ImageFiltersHistoryTreeItem::child(int row) const
{
    return d->childItems.value(row);
}

int ImageFiltersHistoryTreeItem::childCount() const
{
    return d->childItems.count();
}

int ImageFiltersHistoryTreeItem::columnCount() const
{
    return d->itemData.count();
}

QVariant ImageFiltersHistoryTreeItem::data(int column) const
{
    return d->itemData.value(column);
}

ImageFiltersHistoryTreeItem* ImageFiltersHistoryTreeItem::parent() const
{
    return d->parentItem;
}

int ImageFiltersHistoryTreeItem::row() const
{
    if (d->parentItem)
    {
        return d->parentItem->d->childItems.indexOf(const_cast<ImageFiltersHistoryTreeItem*>(this));
    }

    return 0;
}

void ImageFiltersHistoryTreeItem::removeChild(int row)
{
    delete child(row);
    d->childItems.removeAt(row);
}

bool ImageFiltersHistoryTreeItem::isDisabled() const
{
    return d->disabled;
}

void ImageFiltersHistoryTreeItem::setDisabled(bool disabled) const
{
    d->disabled = disabled;
}

} // namespace Digikam
