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

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

#include <kglobal.h>
#include <klocale.h>

// local includes

#include "tableview_columnfactory.h"
#include "thumbnailloadthread.h"

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
        SubColumnSize = 1
    };

private:
    SubColumn subColumn;

public:

    explicit ColumnFileProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~ColumnFileProperties() { }

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);

    virtual QString getTitle();

    virtual ColumnFlags getColumnFlags() const;

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const;

};

class ColumnFileConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:
    explicit ColumnFileConfigurationWidget(
            TableViewShared* const sharedObject,
            const TableViewColumnConfiguration& columnConfiguration,
            QWidget* const parentWidget
        );
    virtual ~ColumnFileConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private:

    ColumnFileProperties::SubColumn subColumn;
    QComboBox* selectorSizeType;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_FILE_H

