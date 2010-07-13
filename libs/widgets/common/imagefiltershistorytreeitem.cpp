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

namespace Digikam {
  
ImageFiltersHistoryTreeItem::ImageFiltersHistoryTreeItem(const QList< QVariant >& data, ImageFiltersHistoryTreeItem* parent)
{
    parentItem = parent;
    itemData = data;
}

ImageFiltersHistoryTreeItem::ImageFiltersHistoryTreeItem(const QString& data, ImageFiltersHistoryTreeItem* parent)
{
    parentItem = parent;
    itemData.append(data);
}


ImageFiltersHistoryTreeItem::~ImageFiltersHistoryTreeItem()
{
    qDeleteAll(childItems);
}

void ImageFiltersHistoryTreeItem::appendChild(ImageFiltersHistoryTreeItem *item)
{
    childItems.append(item);
}

ImageFiltersHistoryTreeItem *ImageFiltersHistoryTreeItem::child(int row)
{
    return childItems.value(row);
}

int ImageFiltersHistoryTreeItem::childCount() const
{
    return childItems.count();
}

int ImageFiltersHistoryTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant ImageFiltersHistoryTreeItem::data(int column) const
{
    return itemData.value(column);
}

ImageFiltersHistoryTreeItem *ImageFiltersHistoryTreeItem::parent()
{
    return parentItem;
}

int ImageFiltersHistoryTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ImageFiltersHistoryTreeItem*>(this));

    return 0;
}

void ImageFiltersHistoryTreeItem::removeChild(int row)
{
    delete child(row);
    childItems.removeAt(row);
}

} //namespace Digikam