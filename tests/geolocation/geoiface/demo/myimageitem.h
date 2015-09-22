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

#ifndef MYIMAGEITEM_H
#define MYIMAGEITEM_H

// Qt includes

#include <QTreeWidgetItem>
#include <QPersistentModelIndex>
#include <QUrl>

// libkgeomap includes

#include "src/types.h"
#include "src/geocoordinates.h"

const int RoleMyData      = Qt::UserRole+0;
const int RoleCoordinates = Qt::UserRole+1;

class MyImageItem : public QTreeWidgetItem
{
public:

    MyImageItem(const QUrl& url, const KGeoMap::GeoCoordinates& itemCoordinates);
    virtual ~MyImageItem();

    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant& value);

private:

    KGeoMap::GeoCoordinates coordinates;
    QUrl                    imageUrl;
};

#endif /* MYIMAGEITEM_H */
