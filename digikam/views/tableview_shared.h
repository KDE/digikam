/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-14
 * Description : Table view shared object
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_SHARED_H
#define TABLEVIEW_SHARED_H

namespace Digikam
{

class TableViewModel;
class TableViewColumnFactory;
class TableViewItemDelegate;
class ImageFilterModel;
class ThumbnailLoadThread;
class TableViewSortFilterProxyModel;

class TableViewShared
{
public:
    TableViewModel* tableViewModel;
    TableViewColumnFactory* columnFactory;
    TableViewItemDelegate* itemDelegate;
    ImageFilterModel* imageFilterModel;
    ThumbnailLoadThread* thumbnailLoadThread;
    TableViewSortFilterProxyModel* sortModel;
};

} /* namespace Digikam */

#endif // TABLEVIEW_SHARED_H
