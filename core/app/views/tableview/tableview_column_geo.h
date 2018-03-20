/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Geographic column
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

#ifndef TABLEVIEW_COLUMN_GEO_H
#define TABLEVIEW_COLUMN_GEO_H

// Qt includes

#include <QStringList>

// Local includes

#include "tableview_columnfactory.h"

class QComboBox;

namespace Digikam
{

namespace TableViewColumns
{

class ColumnGeoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnHasCoordinates = 0,
        SubColumnCoordinates = 1,
        SubColumnAltitude = 2
    } subColumn;

public:

    explicit ColumnGeoProperties(TableViewShared* const tableViewShared,
                                 const TableViewColumnConfiguration& pConfiguration,
                                 const SubColumn pSubColumn,
                                 QObject* const parent = 0);
    virtual ~ColumnGeoProperties();

    virtual QString getTitle() const;
    virtual ColumnFlags getColumnFlags() const;
    virtual QVariant data(TableViewModel::Item* const item, const int role) const;
    virtual ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const;
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);

    static QStringList getSubColumns();
    static TableViewColumnDescription getDescription();
};

// -----------------------------------------------------------------------------------------------

class ColumnGeoConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:
    explicit ColumnGeoConfigurationWidget(TableViewShared* const sharedObject,
                                          const TableViewColumnConfiguration& columnConfiguration,
                                          QWidget* const parentWidget);
    virtual ~ColumnGeoConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private:

    ColumnGeoProperties::SubColumn subColumn;
    QComboBox*                     selectorAltitudeUnit;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_GEO_H

