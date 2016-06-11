/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: File properties
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

#ifndef TABLEVIEW_COLUMN_FILE_H
#define TABLEVIEW_COLUMN_FILE_H

// Qt includes

#include <QStringList>

// Local includes

#include "tableview_columnfactory.h"

class QComboBox;

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFileProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnName = 0,
        SubColumnFilePath = 1,
        SubColumnSize = 2,
        SubColumnLastModified = 3
    };

public:

    explicit ColumnFileProperties(TableViewShared* const tableViewShared,
                                  const TableViewColumnConfiguration& pConfiguration,
                                  const SubColumn pSubColumn,
                                  QObject* const parent = 0);
    virtual ~ColumnFileProperties() {};

    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);
    virtual QString getTitle() const;
    virtual ColumnFlags getColumnFlags() const;
    virtual QVariant data(TableViewModel::Item* const item, const int role) const;
    virtual ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const;

public:

    static TableViewColumnDescription getDescription();
    static QStringList                getSubColumns();

private:

    SubColumn subColumn;
};

// ---------------------------------------------------------------------------------------

class ColumnFileConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:

    explicit ColumnFileConfigurationWidget(TableViewShared* const sharedObject,
                                           const TableViewColumnConfiguration& columnConfiguration,
                                           QWidget* const parentWidget);
    virtual ~ColumnFileConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private:

    ColumnFileProperties::SubColumn subColumn;
    QComboBox*                      selectorSizeType;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_FILE_H
