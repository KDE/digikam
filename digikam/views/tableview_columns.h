/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
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

#ifndef TABLEVIEW_COLUMNS_H
#define TABLEVIEW_COLUMNS_H

// Qt includes

#include <QObject>
#include <QStringList>

// KDE includes

// local includes

#include "tableview_columnfactory.h"
#include <libkgeomap/geocoordinates.h>

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFilename : public TableViewColumn
{
public:

    explicit ColumnFilename(TableViewColumnDataSource* const pDataSource)
      : TableViewColumn(pDataSource)
    {
    }
    virtual ~ColumnFilename() { }
    static QString getIdStatic() { return "filename"; }
    virtual QString getId() { return "filename"; }
    virtual QString getTitle() { return i18n("Filename"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        return sourceIndex.data(role);
    }

};

class ColumnCoordinates : public TableViewColumn
{
public:

    explicit ColumnCoordinates(TableViewColumnDataSource* const pDataSource)
      : TableViewColumn(pDataSource)
    {
    }
    virtual ~ColumnCoordinates() { }
    static QString getIdStatic() { return "coordinates"; }
    virtual QString getId() { return "coordinates"; }
    virtual QString getTitle() { return i18n("Coordinates"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        if (role!=Qt::DisplayRole)
        {
            return QVariant();
        }

        const ImageInfo info = dataSource->sourceModel->imageInfo(sourceIndex);

        if (info.isNull() || !info.hasCoordinates())
        {
            return QVariant();
        }

        const KGeoMap::GeoCoordinates coordinates(info.latitudeNumber(), info.longitudeNumber());

        return QString("%1,%2").arg(coordinates.latString()).arg(coordinates.lonString());
    }

};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMNS_H

