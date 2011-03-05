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

#ifndef IMAGEFILTESRHISTORYTREEITEM_H
#define IMAGEFILTESRHISTORYTREEITEM_H

// Qt includes

#include <QList>
#include <QVariant>

// local includes

#include "digikam_export.h"

namespace Digikam
{

class ImageFiltersHistoryTreeItem
{
public:

    ImageFiltersHistoryTreeItem(const QList<QVariant>& data, ImageFiltersHistoryTreeItem* parent = 0);
    ImageFiltersHistoryTreeItem(const QString& data, ImageFiltersHistoryTreeItem* parent = 0);
    ~ImageFiltersHistoryTreeItem();

    void appendChild(ImageFiltersHistoryTreeItem* child);
    void removeChild(int row);
    ImageFiltersHistoryTreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    ImageFiltersHistoryTreeItem* parent() const;
    void setDisabled(bool disabled) const;
    bool isDisabled() const;

private:

    class ImageFiltersHistoryTreeItemPriv;
    ImageFiltersHistoryTreeItemPriv* const d;
};

} // namespace Digikam

#endif // IMAGEFILTERHISTORYTREEITEM_H
