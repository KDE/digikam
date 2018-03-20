/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-20
 * Description : sub class of QStandardItem to represent the images
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef MY_IMAGE_ITEM_H
#define MY_IMAGE_ITEM_H

// Qt includes

#include <QTreeWidgetItem>
#include <QPersistentModelIndex>
#include <QUrl>

// Local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"

using namespace Digikam;

const int RoleMyData      = Qt::UserRole+0;
const int RoleCoordinates = Qt::UserRole+1;

class MyImageItem : public QTreeWidgetItem
{
public:

    MyImageItem(const QUrl& url, const GeoCoordinates& itemCoordinates);
    virtual ~MyImageItem();

    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant& value);

private:

    GeoCoordinates coordinates;
    QUrl           imageUrl;
};

#endif // MY_IMAGE_ITEM_H
