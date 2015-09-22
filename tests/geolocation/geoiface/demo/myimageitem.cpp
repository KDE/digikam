/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-20
 * @brief  sub class of QStandardItem to represent the images
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "myimageitem.h"

MyImageItem::MyImageItem(const QUrl& url, const GeoIface::GeoCoordinates& itemCoordinates)
    : QTreeWidgetItem(), coordinates(itemCoordinates), imageUrl(url)
{
}

MyImageItem::~MyImageItem()
{
}

QVariant MyImageItem::data(int column, int role) const
{
    if (role == RoleCoordinates)
    {
        return QVariant::fromValue(coordinates);
    }
    else if (role == Qt::DisplayRole)
    {
        switch (column)
        {
            case 0:
                return imageUrl.fileName();

            case 1:
                return coordinates.geoUrl();

            default:
                return QVariant();
        }
    }

    return QTreeWidgetItem::data(column, role);
}

void MyImageItem::setData(int column, int role, const QVariant& value)
{
    if (role == RoleCoordinates)
    {
        if (value.canConvert<GeoIface::GeoCoordinates>())
        {
            coordinates = value.value<GeoIface::GeoCoordinates>();
            emitDataChanged();
        }

        return;
    }

    QTreeWidgetItem::setData(column, role, value);
}
